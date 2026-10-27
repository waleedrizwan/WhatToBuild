#include "data/ddragon.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <future>
#include <regex>
#include <stdexcept>
#include <unordered_set>

#include "net/http.h"

using nlohmann::json;

namespace {

const std::string BASE = "https://ddragon.leagueoflegends.com";

json getJson(const std::string& url) {
  HttpResponse res = httpGet(url);
  if (!res.error.empty()) throw std::runtime_error("Request failed (" + res.error + "): " + url);
  if (!res.ok()) throw std::runtime_error("Request failed (" + std::to_string(res.status) + "): " + url);
  return json::parse(res.body);
}

std::vector<std::string> stringList(const json& obj, const char* key) {
  if (!obj.contains(key) || !obj[key].is_array()) return {};
  return obj[key].get<std::vector<std::string>>();
}

std::string optString(const json& obj, const char* key) {
  auto it = obj.find(key);
  return it != obj.end() && it->is_string() ? it->get<std::string>() : "";
}

std::unique_ptr<GameData> parseGameData(const std::string& version, const json& rawChampions, const json& rawItems) {
  auto data = std::make_unique<GameData>();
  data->version = version;

  for (const auto& [key, c] : rawChampions.items()) {
    const json& info = c.at("info");
    data->champions.push_back({
        .id = c.at("id").get<std::string>(),
        .name = c.at("name").get<std::string>(),
        .title = c.at("title").get<std::string>(),
        .tags = stringList(c, "tags"),
        .info = {info.at("attack").get<int>(), info.at("defense").get<int>(), info.at("magic").get<int>(),
                 info.at("difficulty").get<int>()},
        .attackRange = c.at("stats").at("attackrange").get<double>(),
        .image = BASE + "/cdn/" + version + "/img/champion/" + c.at("image").at("full").get<std::string>(),
    });
  }
  std::stable_sort(data->champions.begin(), data->champions.end(),
                   [](const Champion& a, const Champion& b) { return nameLess(a.name, b.name); });

  // Item ids are numeric; walk them in numeric order so the first copy of a name wins.
  std::vector<std::pair<long, const json*>> ordered;
  for (auto it = rawItems.begin(); it != rawItems.end(); ++it) {
    char* end = nullptr;
    long id = std::strtol(it.key().c_str(), &end, 10);
    if (end && *end == '\0') ordered.emplace_back(id, &it.value());
  }
  std::sort(ordered.begin(), ordered.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

  std::unordered_set<std::string> seen;
  for (const auto& [id, rawPtr] : ordered) {
    const json& raw = *rawPtr;
    // Summoner's Rift only; ids >= 10000 are mode-specific copies (Arena, ARAM, ...).
    bool onRift = raw.contains("maps") && raw["maps"].value("11", false);
    if (id >= 10000 || !onRift || !raw.at("gold").value("purchasable", false)) continue;
    if ((raw.contains("inStore") && raw["inStore"] == false) || !optString(raw, "requiredChampion").empty() ||
        !optString(raw, "requiredAlly").empty())
      continue;
    std::string name = raw.at("name").get<std::string>();
    if (!seen.insert(name).second) continue;

    Item item{
        .id = std::to_string(id),
        .name = name,
        .description = optString(raw, "description"),
        .plaintext = optString(raw, "plaintext"),
        .gold = raw.at("gold").value("total", 0),
        .tags = stringList(raw, "tags"),
        .image = BASE + "/cdn/" + version + "/img/item/" + raw.at("image").at("full").get<std::string>(),
    };
    if (raw.contains("stats"))
      for (const auto& [k, v] : raw["stats"].items()) item.stats[k] = v.get<double>();
    item.isBoots = item.hasTag("Boots");
    item.isCompleted = !raw.contains("into") || raw["into"].empty();
    data->items.push_back(std::move(item));
  }

  data->index();
  return data;
}

}  // namespace

bool Champion::hasTag(std::string_view tag) const {
  return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

bool Item::hasTag(std::string_view tag) const {
  return std::find(tags.begin(), tags.end(), tag) != tags.end();
}

double Item::stat(const std::string& key) const {
  auto it = stats.find(key);
  return it == stats.end() ? 0 : it->second;
}

void GameData::index() {
  championsById_.clear();
  itemsByName_.clear();
  itemsById_.clear();
  for (const auto& c : champions) championsById_[c.id] = &c;
  for (const auto& i : items) {
    itemsByName_[i.name] = &i;
    itemsById_[i.id] = &i;
  }
}

const Champion* GameData::championById(const std::string& id) const {
  auto it = championsById_.find(id);
  return it == championsById_.end() ? nullptr : it->second;
}

const Item* GameData::itemByName(const std::string& name) const {
  auto it = itemsByName_.find(name);
  return it == itemsByName_.end() ? nullptr : it->second;
}

const Item* GameData::itemById(const std::string& id) const {
  auto it = itemsById_.find(id);
  return it == itemsById_.end() ? nullptr : it->second;
}

std::unique_ptr<GameData> loadGameData() {
  json versions = getJson(BASE + "/api/versions.json");
  std::string version = versions.at(0).get<std::string>();
  auto champions = std::async(std::launch::async, getJson, BASE + "/cdn/" + version + "/data/en_US/champion.json");
  json items = getJson(BASE + "/cdn/" + version + "/data/en_US/item.json");
  return parseGameData(version, champions.get().at("data"), items.at("data"));
}

bool isTrackableItem(const Item& item) {
  if (item.hasTag("Consumable") || item.hasTag("Trinket")) return false;
  return (item.isCompleted || item.isBoots) && item.gold >= 700;
}

std::string describeItem(const Item& item) {
  static const std::regex lineBreak(R"(<br\s*/?>)", std::regex::icase);
  static const std::regex blockEnd(R"(</(stats|passive|active|li)>)", std::regex::icase);
  static const std::regex anyTag(R"(<[^>]+>)");
  static const std::regex spaces(R"([ \t]+)");
  static const std::regex paddedNewline(R"( *\n *)");
  static const std::regex blankLines(R"(\n{2,})");

  std::string s = std::regex_replace(item.description, lineBreak, "\n");
  s = std::regex_replace(s, blockEnd, "\n");
  s = std::regex_replace(s, anyTag, " ");
  s = std::regex_replace(s, spaces, " ");
  s = std::regex_replace(s, paddedNewline, "\n");
  s = std::regex_replace(s, blankLines, "\n");
  auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

bool nameLess(std::string_view a, std::string_view b) {
  auto lower = [](unsigned char ch) { return std::tolower(ch); };
  bool less = std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [&](char x, char y) {
    return lower(x) < lower(y);
  });
  bool greater = std::lexicographical_compare(b.begin(), b.end(), a.begin(), a.end(), [&](char x, char y) {
    return lower(x) < lower(y);
  });
  return less || (!greater && a > b);  // on a case-only tie, lowercase first
}

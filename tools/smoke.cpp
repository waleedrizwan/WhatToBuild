// Prints recommended builds for a few sample matchups against live Data Dragon data.

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "data/champions.h"
#include "data/ddragon.h"
#include "data/items.h"
#include "engine/analyze.h"
#include "engine/recommend.h"

namespace {

struct Scenario {
  std::string you;
  std::vector<std::string> enemies;
  std::vector<std::vector<std::string>> enemyItems;
};

const std::vector<Scenario> SCENARIOS = {
    {"Jinx", {"Draven", "Leona", "Zed", "Malphite", "Soraka"}, {}},
    {"Ahri", {"Fizz", "Kassadin", "Elise", "Mordekaiser", "Lulu"}, {}},
    {"Darius", {"Teemo", "Aatrox", "Vladimir", "Zed", "Yuumi"}, {}},
    {"Ornn", {"Yasuo", "Tryndamere", "MasterYi", "Jinx", "Thresh"}, {}},
    {"Zed", {"Ahri", "Malphite", "Sejuani", "Lulu", "Caitlyn"}, {}},
    {"Kayle", {"Malzahar", "Warwick", "Garen", "Ashe", "Janna"}, {}},
    // Same comp as the first, but with items: Malphite stacking armor, Draven with Mortal Reminder.
    {"Jinx",
     {"Draven", "Leona", "Zed", "Malphite", "Soraka"},
     {{"Mortal Reminder", "Infinity Edge"}, {}, {"Edge of Night"}, {"Thornmail", "Frozen Heart", "Plated Steelcaps"},
      {"Moonstone Renewer"}}},
    // Kai'Sa building AP should flip the damage read.
    {"Ornn", {"Kaisa", "Elise", "Ahri", "Sejuani", "Lulu"}, {{"Nashor's Tooth", "Riftmaker", "Zhonya's Hourglass"}}},
};

/** Like JS toFixed: ties round up (printf would round 0.625 to "0.62"). */
std::string fixed(double v, int digits) {
  double scale = std::pow(10, digits);
  char buf[32];
  std::snprintf(buf, sizeof buf, "%.*f", digits, std::floor(v * scale + 0.5) / scale);
  return buf;
}

std::string join(const std::vector<std::string>& xs, const char* sep) {
  std::string out;
  for (std::size_t i = 0; i < xs.size(); i++) out += (i ? sep : "") + xs[i];
  return out;
}

}  // namespace

int main() {
  std::unique_ptr<GameData> data;
  try {
    data = loadGameData();
  } catch (const std::exception& e) {
    std::fprintf(stderr, "Couldn't load Data Dragon: %s\n", e.what());
    return 1;
  }
  std::printf("Patch %s\n", data->version.c_str());

  std::vector<std::string> missing;
  for (const auto& k : itemKnowledge())
    if (!data->itemByName(k.name)) missing.push_back("'" + k.name + "'");
  for (const auto& k : bootsKnowledge())
    if (!data->itemByName(k.name)) missing.push_back("'" + k.name + "'");
  if (!missing.empty()) std::printf("Unknown item names: [ %s ]\n", join(missing, ", ").c_str());

  for (const Scenario& s : SCENARIOS) {
    const Champion* champ = data->championById(s.you);
    if (!champ) {
      std::printf("\nUnknown champion %s\n", s.you.c_str());
      continue;
    }
    std::vector<Enemy> enemies;
    for (std::size_t i = 0; i < s.enemies.size(); i++) {
      const Champion* c = data->championById(s.enemies[i]);
      if (!c) continue;
      Enemy e{c, {}};
      if (i < s.enemyItems.size())
        for (const std::string& n : s.enemyItems[i])
          if (const Item* item = data->itemByName(n)) e.items.push_back(item);
      enemies.push_back(std::move(e));
    }
    TeamAnalysis analysis = analyzeEnemies(enemies, s.enemies[0]);
    Build build = recommendBuild(*data, *champ, archetypeFor(*champ), defaultRoleFor(*champ), analysis);

    std::vector<std::string> names;
    for (const Enemy& e : enemies) names.push_back(e.champion->name);
    std::printf("\n=== %s (%s) vs %s\n", champ->name.c_str(), std::string(archetypeKey(build.archetype)).c_str(),
                join(names, ", ").c_str());
    for (const std::string& n : analysis.intel) std::printf(" i %s\n", n.c_str());

    std::vector<std::string> needs;
    for (Need n : kNeeds) {
      if (analysis.need(n) <= 0.25) continue;
      needs.push_back(std::string(needKey(n)) + "=" + fixed(analysis.need(n), 2));
    }
    std::printf("AD %s%% / AP %s%% %s\n", fixed(analysis.adShare * 100, 0).c_str(), fixed(analysis.apShare * 100, 0).c_str(),
                join(needs, " ").c_str());

    std::vector<std::string> starter;
    for (const Item* i : build.starter) starter.push_back(i->name);
    std::printf("Start: %s\n", join(starter, " + ").c_str());
    if (build.boots)
      std::printf("Boots: %s — %s\n", build.boots->item->name.c_str(), join(build.boots->reasons, "; ").c_str());
    for (std::size_t i = 0; i < build.core.size(); i++) {
      const Pick& p = build.core[i];
      std::printf(" %zu. %s [%s] — %s\n", i + 1, p.item->name.c_str(), fixed(p.score, 1).c_str(),
                  join(p.reasons, "; ").c_str());
    }
    std::vector<std::string> situational;
    for (const Pick& p : build.situational) situational.push_back(p.item->name);
    std::printf(" Situational: %s\n", join(situational, ", ").c_str());
    for (const std::string& n : build.notes) std::printf(" * %s\n", n.c_str());
  }
  return 0;
}

#include "live/live_client.h"

#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdlib>

#include "net/http.h"

using nlohmann::json;

namespace {

constexpr auto POLL_INTERVAL = std::chrono::seconds(5);

std::string str(const json& obj, const char* key) {
  auto it = obj.find(key);
  return it != obj.end() && it->is_string() ? it->get<std::string>() : "";
}

std::optional<Role> roleForPosition(const std::string& position) {
  if (position == "TOP") return Role::Top;
  if (position == "JUNGLE") return Role::Jungle;
  if (position == "MIDDLE") return Role::Mid;
  if (position == "BOTTOM") return Role::Bot;
  if (position == "UTILITY") return Role::Support;
  return std::nullopt;
}

const Champion* championFor(const json& p, const GameData& data) {
  // rawChampionName is "game_character_displayname_MonkeyKing" — the Data Dragon id, and not localized.
  static const std::string prefix = "game_character_displayname_";
  std::string rawId = str(p, "rawChampionName");
  if (rawId.starts_with(prefix)) rawId.erase(0, prefix.size());
  if (!rawId.empty())
    if (const Champion* c = data.championById(rawId)) return c;
  std::string name = str(p, "championName");
  for (const Champion& c : data.champions)
    if (c.name == name) return &c;
  return nullptr;
}

std::vector<std::string> itemNames(const json& p, const GameData& data) {
  std::vector<std::string> out;
  if (!p.contains("items") || !p["items"].is_array()) return out;
  for (const json& i : p["items"]) {
    if (!i.contains("itemID") || !i["itemID"].is_number()) continue;
    const Item* item = data.itemById(std::to_string(i["itemID"].get<long>()));
    if (item && isTrackableItem(*item)) out.push_back(item->name);
  }
  return out;
}

bool isActivePlayer(const json& p, const json& active) {
  if (!active.is_object()) return false;
  std::string riotId = str(active, "riotId"), gameName = str(active, "riotIdGameName"), summoner = str(active, "summonerName");
  if (!riotId.empty() && !str(p, "riotId").empty()) return riotId == str(p, "riotId");
  if (!gameName.empty() && !str(p, "riotIdGameName").empty()) return gameName == str(p, "riotIdGameName");
  return !summoner.empty() && summoner == str(p, "summonerName");
}

LiveStatus waiting(std::string message) {
  return {.state = LiveStatus::State::Waiting, .message = std::move(message)};
}

}  // namespace

std::string liveClientUrl() {
  const char* env = std::getenv("LIVE_CLIENT_URL");
  return env && *env ? env : "https://127.0.0.1:2999";
}

std::optional<LiveGame> parseLiveGame(const std::string& body, const GameData& data) {
  try {
    json raw = json::parse(body);
    const json& players = raw.at("allPlayers");
    const json active = raw.value("activePlayer", json::object());
    const json* me = nullptr;
    for (const json& p : players)
      if (isActivePlayer(p, active)) {
        me = &p;
        break;
      }
    if (!me) return std::nullopt;  // spectating, or the game is still loading
    const Champion* myChamp = championFor(*me, data);
    if (!myChamp) return std::nullopt;

    LiveGame game;
    game.you = myChamp->id;
    game.yourItems = itemNames(*me, data);
    std::string myPosition = str(*me, "position");
    game.role = roleForPosition(myPosition);

    int seen = 0;
    for (const json& p : players) {
      if (str(p, "team") == str(*me, "team")) continue;
      if (seen++ >= 5) break;
      const Champion* c = championFor(p, data);
      if (!c) continue;
      if (!myPosition.empty() && game.laneIndex < 0 && str(p, "position") == myPosition)
        game.laneIndex = static_cast<int>(game.enemies.size());
      game.enemies.push_back({c->id, itemNames(p, data)});
    }

    const json gameData = raw.value("gameData", json::object());
    game.gameTime = gameData.value("gameTime", 0.0);
    game.gameMode = str(gameData, "gameMode");
    return game;
  } catch (const json::exception&) {
    return std::nullopt;
  }
}

LiveStatus fetchLiveGame(const GameData& data, const std::string& baseUrl) {
  HttpResponse res = httpGet(baseUrl + "/liveclientdata/allgamedata", {.timeoutMs = 3000, .insecure = true});
  // Nothing listening on the port (or a 5xx) means no game is running.
  if (!res.error.empty() || res.status >= 500) return waiting("No game running. Waiting for a match to start…");
  if (!res.ok()) return waiting("Game is loading…");
  std::optional<LiveGame> game = parseLiveGame(res.body, data);
  if (!game) return waiting("Game is loading…");
  return {.state = LiveStatus::State::Connected, .game = std::move(*game)};
}

LivePoller::LivePoller(std::function<void()> onUpdate) : onUpdate_(std::move(onUpdate)) {}

LivePoller::~LivePoller() { stop(); }

void LivePoller::start(const GameData& data) {
  stop();
  {
    std::lock_guard lock(mu_);
    stopping_ = false;
    status_ = waiting("Looking for a game…");
  }
  thread_ = std::thread(&LivePoller::run, this, &data, liveClientUrl());
}

void LivePoller::stop() {
  {
    std::lock_guard lock(mu_);
    stopping_ = true;
  }
  cv_.notify_all();
  if (thread_.joinable()) thread_.join();
  std::lock_guard lock(mu_);
  status_ = {};
}

LiveStatus LivePoller::status() const {
  std::lock_guard lock(mu_);
  return status_;
}

void LivePoller::run(const GameData* data, std::string url) {
  while (true) {
    LiveStatus next = fetchLiveGame(*data, url);
    {
      std::unique_lock lock(mu_);
      if (stopping_) return;
      status_ = std::move(next);
    }
    if (onUpdate_) onUpdate_();
    std::unique_lock lock(mu_);
    if (cv_.wait_for(lock, POLL_INTERVAL, [this] { return stopping_; })) return;
  }
}

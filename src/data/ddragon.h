#pragma once
// Live champion/item data from Riot's public Data Dragon CDN (no API key needed).

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

struct ChampionInfo {
  int attack = 0;
  int defense = 0;
  int magic = 0;
  int difficulty = 0;
};

struct Champion {
  std::string id;    // e.g. "MonkeyKing"
  std::string name;  // e.g. "Wukong"
  std::string title;
  std::vector<std::string> tags;
  ChampionInfo info;
  double attackRange = 0;
  std::string image;

  bool hasTag(std::string_view tag) const;
};

struct Item {
  std::string id;
  std::string name;
  std::string description;  // HTML from Riot
  std::string plaintext;
  int gold = 0;
  std::vector<std::string> tags;
  std::string image;
  std::map<std::string, double> stats;
  bool isBoots = false;
  bool isCompleted = false;

  bool hasTag(std::string_view tag) const;
  double stat(const std::string& key) const;
};

/** Holds pointers into itself, so it lives behind a unique_ptr and is never copied. */
struct GameData {
  std::string version;
  std::vector<Champion> champions;  // sorted by name
  std::vector<Item> items;          // Summoner's Rift shop, by id

  GameData() = default;
  GameData(const GameData&) = delete;
  GameData& operator=(const GameData&) = delete;

  const Champion* championById(const std::string& id) const;
  const Item* itemByName(const std::string& name) const;
  const Item* itemById(const std::string& id) const;

  /** Builds the lookup tables; call once after filling champions and items. */
  void index();

 private:
  std::unordered_map<std::string, const Champion*> championsById_;
  std::unordered_map<std::string, const Item*> itemsByName_;
  std::unordered_map<std::string, const Item*> itemsById_;
};

/** Fetches the latest patch's data. Throws std::runtime_error on failure. */
std::unique_ptr<GameData> loadGameData();

/** Items worth tracking on an enemy: finished items and boots, no starters/consumables. */
bool isTrackableItem(const Item& item);

/** Riot descriptions are HTML; turn them into readable plain text. */
std::string describeItem(const Item& item);

/** Case-insensitive ordering for display names (close to JS localeCompare). */
bool nameLess(std::string_view a, std::string_view b);

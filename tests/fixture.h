#pragma once
// A tiny, offline stand-in for Data Dragon: a handful of champions and items with the tags
// and stats the engine actually reads, plus a few entries the loader must filter out.

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <vector>

#include "data/ddragon.h"

namespace fixture {

using nlohmann::json;

inline json champion(const std::string& id, const std::string& name, std::vector<std::string> tags, int attack,
                     int defense, int magic, double range) {
  return {
      {"id", id},
      {"name", name},
      {"title", "the Test"},
      {"tags", tags},
      {"info", {{"attack", attack}, {"defense", defense}, {"magic", magic}, {"difficulty", 5}}},
      {"stats", {{"attackrange", range}}},
      {"image", {{"full", id + ".png"}}},
  };
}

inline json item(const std::string& name, int gold, std::vector<std::string> tags, json stats = json::object()) {
  return {
      {"name", name},
      {"description", "<mainText><stats>" + name + "</stats></mainText>"},
      {"plaintext", ""},
      {"gold", {{"total", gold}, {"purchasable", true}}},
      {"tags", tags},
      {"maps", {{"11", true}, {"12", true}}},
      {"stats", stats},
      {"image", {{"full", name + ".png"}}},
  };
}

inline json championJson() {
  json data = json::object();
  auto add = [&](json c) { data[c["id"].get<std::string>()] = c; };
  add(champion("Jinx", "Jinx", {"Marksman"}, 9, 2, 4, 525));
  add(champion("Malphite", "Malphite", {"Tank", "Fighter"}, 5, 9, 7, 125));
  add(champion("Soraka", "Soraka", {"Support", "Mage"}, 2, 5, 7, 550));
  add(champion("Zed", "Zed", {"Assassin"}, 9, 2, 1, 125));
  add(champion("Ahri", "Ahri", {"Mage", "Assassin"}, 3, 4, 8, 550));
  add(champion("Darius", "Darius", {"Fighter", "Tank"}, 9, 5, 1, 175));
  add(champion("MonkeyKing", "Wukong", {"Fighter", "Tank"}, 8, 5, 2, 175));
  return {{"data", data}};
}

inline json itemJson() {
  json data = json::object();
  // Finished items the engine knows about.
  data["3031"] = item("Infinity Edge", 3500, {"CriticalStrike", "Damage"}, {{"FlatPhysicalDamageMod", 65}});
  data["3032"] = item("Yun Tal Wildarrows", 3000, {"CriticalStrike", "AttackSpeed", "Damage"},
                      {{"FlatPhysicalDamageMod", 50}});
  data["3036"] = item("Lord Dominik's Regards", 3000, {"ArmorPenetration", "CriticalStrike", "Damage"},
                      {{"FlatPhysicalDamageMod", 35}});
  data["3033"] = item("Mortal Reminder", 3000, {"ArmorPenetration", "CriticalStrike", "Damage"},
                      {{"FlatPhysicalDamageMod", 35}});
  data["6672"] = item("Kraken Slayer", 3000, {"AttackSpeed", "Damage"}, {{"FlatPhysicalDamageMod", 45}});
  data["3153"] = item("Blade of The Ruined King", 3200, {"AttackSpeed", "Damage", "LifeSteal"},
                      {{"FlatPhysicalDamageMod", 40}});
  data["3026"] = item("Guardian Angel", 3200, {"Armor", "Damage"},
                      {{"FlatPhysicalDamageMod", 55}, {"FlatArmorMod", 45}});
  data["3075"] = item("Thornmail", 2450, {"Armor", "Health"}, {{"FlatArmorMod", 75}, {"FlatHPPoolMod", 150}});
  data["3110"] = item("Frozen Heart", 2500, {"Armor", "Mana"}, {{"FlatArmorMod", 65}});
  data["3115"] = item("Nashor's Tooth", 2900, {"SpellDamage", "AttackSpeed"}, {{"FlatMagicDamageMod", 80}});
  data["4633"] = item("Riftmaker", 3100, {"SpellDamage", "Health", "SpellVamp"},
                      {{"FlatMagicDamageMod", 70}, {"FlatHPPoolMod", 350}});
  data["3157"] = item("Zhonya's Hourglass", 3250, {"SpellDamage", "Armor"},
                      {{"FlatMagicDamageMod", 105}, {"FlatArmorMod", 50}});
  data["6617"] = item("Moonstone Renewer", 2200, {"SpellDamage", "ManaRegen"}, {{"FlatMagicDamageMod", 25}});
  // Boots.
  data["3006"] = item("Berserker's Greaves", 1100, {"Boots", "AttackSpeed"});
  data["3047"] = item("Plated Steelcaps", 1200, {"Boots", "Armor"}, {{"FlatArmorMod", 25}});
  data["3111"] = item("Mercury's Treads", 1250, {"Boots", "SpellBlock"}, {{"FlatSpellBlockMod", 25}});
  // Starters and components.
  data["1055"] = item("Doran's Blade", 450, {"Damage", "Health", "LifeSteal"});
  data["2003"] = item("Health Potion", 50, {"Consumable"});
  json longSword = item("Long Sword", 350, {"Damage"}, {{"FlatPhysicalDamageMod", 10}});
  longSword["into"] = {"3031"};
  data["1036"] = longSword;

  // Entries the loader must drop.
  data["223031"] = item("Infinity Edge (Arena)", 3500, {"CriticalStrike"});  // mode copy, id >= 10000
  data["4031"] = item("Infinity Edge", 9999, {"CriticalStrike"});            // duplicate name, later id
  json notOnRift = item("Howling Abyss Only", 1000, {"Damage"});
  notOnRift["maps"] = {{"11", false}, {"12", true}};
  data["3901"] = notOnRift;
  json championOnly = item("Kalista's Black Spear", 0, {});
  championOnly["requiredChampion"] = "Kalista";
  data["3599"] = championOnly;
  json hidden = item("Your Cut", 0, {});
  hidden["inStore"] = false;
  data["3400"] = hidden;
  json unbuyable = item("Not For Sale", 1000, {"Damage"});
  unbuyable["gold"]["purchasable"] = false;
  data["3902"] = unbuyable;
  return {{"data", data}};
}

/** Parsed fixture data; built once and shared, since GameData is immutable after loading. */
inline const GameData& data() {
  static const std::unique_ptr<GameData> d =
      parseGameData("1.0.1", championJson().dump(), itemJson().dump());
  return *d;
}

inline const Champion& champ(const std::string& id) { return *data().championById(id); }
inline const Item* itemNamed(const std::string& name) { return data().itemByName(name); }

}  // namespace fixture

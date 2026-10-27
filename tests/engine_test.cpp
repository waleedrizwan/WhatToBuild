#include <gtest/gtest.h>

#include <algorithm>
#include <unordered_set>

#include "data/items.h"
#include "engine/analyze.h"
#include "engine/enemy_items.h"
#include "engine/recommend.h"
#include "fixture.h"

using fixture::champ;
using fixture::itemNamed;

namespace {

std::vector<const Item*> items(std::initializer_list<const char*> names) {
  std::vector<const Item*> out;
  for (const char* n : names) out.push_back(itemNamed(n));
  return out;
}

bool contains(const std::vector<std::string>& lines, const std::string& text) {
  return std::any_of(lines.begin(), lines.end(), [&](const std::string& l) { return l.find(text) != std::string::npos; });
}

std::vector<std::string> names(const std::vector<Pick>& picks) {
  std::vector<std::string> out;
  for (const Pick& p : picks) out.push_back(p.item->name);
  return out;
}

Build buildFor(const char* you, const TeamAnalysis& analysis, const std::vector<const Item*>& owned = {},
               std::optional<Role> role = std::nullopt) {
  const Champion& c = champ(you);
  return recommendBuild(fixture::data(), c, archetypeFor(c), role.value_or(defaultRoleFor(c)), analysis, owned);
}

}  // namespace

// --- Enemy items ------------------------------------------------------------

TEST(EnemyBuild, SpotsArmorStacking) {
  EnemyBuild b = summarizeEnemyBuild("Malphite", items({"Thornmail", "Frozen Heart"}), 0.7);
  EXPECT_DOUBLE_EQ(b.armor, 140);
  EXPECT_DOUBLE_EQ(b.tankItems, 2);
  EXPECT_TRUE(contains(b.intel, "Malphite has 140 bonus armor (Thornmail, Frozen Heart): buy armor penetration"));
  EXPECT_TRUE(contains(b.intel, "Malphite has Grievous Wounds (Thornmail)"));
}

TEST(EnemyBuild, ItemsOverrideTheKitsDamageType) {
  ThreatProfile kit = threatProfile(champ("Zed"));
  auto [profile, build] = applyEnemyBuild(kit, items({"Nashor's Tooth", "Riftmaker", "Zhonya's Hourglass"}));
  EXPECT_GT(profile.apShare, 0.6);
  EXPECT_LT(kit.apShare, 0.2);
  EXPECT_TRUE(contains(build.intel, "Zed is building AP, so their damage is mostly magic"));
  EXPECT_TRUE(contains(build.intel, "Zed has Zhonya's Hourglass, so time your burst around it"));
}

TEST(EnemyBuild, NoItemsLeavesProfileAlone) {
  ThreatProfile kit = threatProfile(champ("Jinx"));
  auto [profile, build] = applyEnemyBuild(kit, {});
  EXPECT_DOUBLE_EQ(profile.apShare, kit.apShare);
  EXPECT_EQ(profile.traits, kit.traits);
  EXPECT_TRUE(build.intel.empty());
}

TEST(EnemyBuild, TraitsAreCapped) {
  ThreatProfile kit = threatProfile(champ("Soraka"));  // heal 1.2 already
  auto [profile, build] = applyEnemyBuild(kit, items({"Blade of The Ruined King", "Riftmaker", "Moonstone Renewer"}));
  EXPECT_DOUBLE_EQ(profile.trait(Trait::Heal), 1.5);
}

// --- Team analysis ----------------------------------------------------------

TEST(AnalyzeEnemies, EmptyTeamIsNeutral) {
  TeamAnalysis a = analyzeEnemies({}, "");
  EXPECT_DOUBLE_EQ(a.adShare, 0.5);
  EXPECT_DOUBLE_EQ(a.apShare, 0.5);
  for (double n : a.needs) EXPECT_DOUBLE_EQ(n, 0);
  EXPECT_EQ(a.laneOpponent(), nullptr);
}

TEST(AnalyzeEnemies, PhysicalTeamNeedsArmor) {
  TeamAnalysis a = analyzeEnemies({{&champ("Zed"), {}}, {&champ("Darius"), {}}, {&champ("Jinx"), {}}}, "");
  EXPECT_GT(a.adShare, 0.8);
  EXPECT_GT(a.need(Need::Armor), 0.9);
  EXPECT_DOUBLE_EQ(a.need(Need::Mr), 0);
  EXPECT_FALSE(a.sourcesOf(Need::Armor).empty());
}

TEST(AnalyzeEnemies, LaneOpponentCountsMore) {
  std::vector<Enemy> team = {{&champ("Zed"), {}}, {&champ("Ahri"), {}}};
  TeamAnalysis zedLane = analyzeEnemies(team, "Zed");
  TeamAnalysis ahriLane = analyzeEnemies(team, "Ahri");
  ASSERT_NE(zedLane.laneOpponent(), nullptr);
  EXPECT_EQ(zedLane.laneOpponent()->champion->id, "Zed");
  EXPECT_GT(zedLane.adShare, ahriLane.adShare);
}

TEST(AnalyzeEnemies, HealersAndTanksDriveTheirNeeds) {
  TeamAnalysis a = analyzeEnemies({{&champ("Soraka"), {}}, {&champ("Malphite"), items({"Thornmail", "Frozen Heart"})}}, "");
  EXPECT_GT(a.need(Need::Antiheal), 0.5);
  ASSERT_FALSE(a.sourcesOf(Need::Antiheal).empty());
  EXPECT_EQ(a.sourcesOf(Need::Antiheal).front(), "Soraka");
  EXPECT_EQ(a.sourcesOf(Need::Armorpen), std::vector<std::string>{"Malphite"});
  EXPECT_TRUE(contains(a.intel, "140 bonus armor"));

  TeamAnalysis noItems = analyzeEnemies({{&champ("Soraka"), {}}, {&champ("Malphite"), {}}}, "");
  EXPECT_GT(a.need(Need::Armorpen), noItems.need(Need::Armorpen));
}

// --- Recommendations --------------------------------------------------------

TEST(RecommendBuild, GenericJinxBuild) {
  Build b = buildFor("Jinx", analyzeEnemies({}, ""));
  EXPECT_EQ(b.archetype, Archetype::AdcCrit);
  ASSERT_FALSE(b.core.empty());
  EXPECT_EQ(b.core.front().item->name, "Infinity Edge");
  ASSERT_TRUE(b.boots.has_value());
  EXPECT_EQ(b.boots->item->name, "Berserker's Greaves");
  ASSERT_EQ(b.starter.size(), 3u);
  EXPECT_EQ(b.starter[0]->name, "Doran's Blade");
  EXPECT_EQ(b.starter[1]->name, "Health Potion");
  EXPECT_LE(b.core.size(), 5u);

  int gold = b.boots->item->gold;
  for (const Pick& p : b.core) gold += p.item->gold;
  EXPECT_EQ(b.totalGold, gold);
}

TEST(RecommendBuild, NeverTwoItemsFromOneUniqueGroup) {
  // Heavy healing makes Mortal Reminder tempting, but it shares Last Whisper with Lord Dominik's.
  TeamAnalysis a = analyzeEnemies({{&champ("Soraka"), {}}, {&champ("Malphite"), items({"Thornmail", "Frozen Heart"})}}, "");
  for (const Build& b : {buildFor("Jinx", a), buildFor("Jinx", a, items({"Lord Dominik's Regards"}))}) {
    std::unordered_set<std::string> groups;
    for (const Pick& p : b.core) {
      const ItemKnowledge* k = findItemKnowledge(p.item->name);
      std::string group = k && !k->group.empty() ? k->group : isAntihealItem(p.item->name) ? "antiheal" : "";
      if (!group.empty()) EXPECT_TRUE(groups.insert(group).second) << "two " << group << " items";
    }
  }
}

TEST(RecommendBuild, OwnedItemsAreLockedInFirst) {
  Build b = buildFor("Jinx", analyzeEnemies({}, ""), items({"Kraken Slayer", "Plated Steelcaps"}));
  ASSERT_FALSE(b.core.empty());
  EXPECT_EQ(b.core.front().item->name, "Kraken Slayer");
  EXPECT_TRUE(b.core.front().owned);
  ASSERT_TRUE(b.boots.has_value());
  EXPECT_EQ(b.boots->item->name, "Plated Steelcaps");  // what you bought, not what it would pick
  EXPECT_TRUE(b.boots->owned);
  auto core = names(b.core);
  EXPECT_EQ(std::count(core.begin(), core.end(), "Kraken Slayer"), 1);
}

TEST(RecommendBuild, SupportItemsOnlyForSupports) {
  Build support = buildFor("Soraka", analyzeEnemies({}, ""), {}, Role::Support);
  Build mid = buildFor("Soraka", analyzeEnemies({}, ""), {}, Role::Mid);
  auto supportCore = names(support.core), midCore = names(mid.core);
  EXPECT_NE(std::find(supportCore.begin(), supportCore.end(), "Moonstone Renewer"), supportCore.end());
  EXPECT_EQ(std::find(midCore.begin(), midCore.end(), "Moonstone Renewer"), midCore.end());
  // Supports start with one potion, laners with two.
  auto potions = [](const Build& b) {
    return std::count_if(b.starter.begin(), b.starter.end(), [](const Item* i) { return i->name == "Health Potion"; });
  };
  EXPECT_EQ(potions(support), 1);
  EXPECT_EQ(potions(mid), 2);
}

TEST(RecommendBuild, NextOptionsAreDistinctChoicesForTheNextSlot) {
  Build b = buildFor("Jinx", analyzeEnemies({}, ""));
  ASSERT_EQ(b.nextOptions.size(), 3u);
  EXPECT_EQ(b.nextOptions[0].item, b.core[0].item);  // the build's pick is the first option
  for (std::size_t i = 1; i < b.nextOptions.size(); i++) EXPECT_GE(b.nextOptions[i - 1].score, b.nextOptions[i].score);

  std::unordered_set<std::string> seen;
  for (const Pick& p : b.nextOptions) {
    EXPECT_TRUE(seen.insert(p.item->name).second);
    EXPECT_FALSE(p.reasons.empty());
  }
}

TEST(RecommendBuild, NextOptionsSkipWhatYouOwn) {
  Build b = buildFor("Jinx", analyzeEnemies({}, ""), items({"Infinity Edge", "Lord Dominik's Regards"}));
  auto options = names(b.nextOptions);
  EXPECT_EQ(std::find(options.begin(), options.end(), "Infinity Edge"), options.end());
  EXPECT_EQ(std::find(options.begin(), options.end(), "Mortal Reminder"), options.end());  // same group as LDR
}

TEST(RecommendBuild, AnswersTheEnemyComp) {
  // With a healer and an armor-stacker on the enemy team, reasons should say so.
  TeamAnalysis a = analyzeEnemies({{&champ("Soraka"), {}}, {&champ("Malphite"), items({"Thornmail", "Frozen Heart"})}}, "");
  Build b = buildFor("Jinx", a);
  std::vector<std::string> reasons;
  for (const Pick& p : b.core) reasons.insert(reasons.end(), p.reasons.begin(), p.reasons.end());
  EXPECT_TRUE(contains(reasons, "Armor penetration"));
  EXPECT_TRUE(contains(b.notes, "Heavy healing on the enemy team (Soraka"));
}

#include <gtest/gtest.h>

#include "data/ddragon.h"
#include "fixture.h"

using fixture::data;

TEST(ParseGameData, SortsChampionsByNameAndBuildsImageUrls) {
  const auto& champs = data().champions;
  ASSERT_EQ(champs.size(), 7u);
  for (std::size_t i = 1; i < champs.size(); i++) EXPECT_TRUE(nameLess(champs[i - 1].name, champs[i].name));
  EXPECT_EQ(data().championById("MonkeyKing")->name, "Wukong");
  EXPECT_EQ(data().championById("Jinx")->image, "https://ddragon.leagueoflegends.com/cdn/1.0.1/img/champion/Jinx.png");
  EXPECT_DOUBLE_EQ(data().championById("Jinx")->attackRange, 525);
}

TEST(ParseGameData, KeepsOnlySummonersRiftShopItems) {
  EXPECT_EQ(data().itemByName("Infinity Edge (Arena)"), nullptr);
  EXPECT_EQ(data().itemByName("Howling Abyss Only"), nullptr);
  EXPECT_EQ(data().itemByName("Kalista's Black Spear"), nullptr);
  EXPECT_EQ(data().itemByName("Your Cut"), nullptr);
  EXPECT_EQ(data().itemByName("Not For Sale"), nullptr);
  EXPECT_NE(data().itemByName("Thornmail"), nullptr);
}

TEST(ParseGameData, FirstIdWinsForDuplicateNames) {
  const Item* ie = data().itemByName("Infinity Edge");
  ASSERT_NE(ie, nullptr);
  EXPECT_EQ(ie->id, "3031");
  EXPECT_EQ(ie->gold, 3500);
  EXPECT_EQ(data().itemById("4031"), nullptr);
}

TEST(ParseGameData, ReadsTagsStatsAndFlags) {
  const Item* thornmail = data().itemByName("Thornmail");
  EXPECT_DOUBLE_EQ(thornmail->stat("FlatArmorMod"), 75);
  EXPECT_DOUBLE_EQ(thornmail->stat("FlatMagicDamageMod"), 0);
  EXPECT_TRUE(thornmail->hasTag("Health"));
  EXPECT_TRUE(thornmail->isCompleted);
  EXPECT_TRUE(data().itemByName("Plated Steelcaps")->isBoots);
  EXPECT_FALSE(data().itemByName("Long Sword")->isCompleted);
}

TEST(ParseGameData, RejectsMalformedJson) {
  EXPECT_ANY_THROW(parseGameData("1.0", "{not json", "{}"));
  EXPECT_ANY_THROW(parseGameData("1.0", R"({"nodata": {}})", R"({"data": {}})"));
}

TEST(IsTrackableItem, FinishedItemsAndBootsOnly) {
  EXPECT_TRUE(isTrackableItem(*data().itemByName("Infinity Edge")));
  EXPECT_TRUE(isTrackableItem(*data().itemByName("Berserker's Greaves")));
  EXPECT_FALSE(isTrackableItem(*data().itemByName("Health Potion")));  // consumable
  EXPECT_FALSE(isTrackableItem(*data().itemByName("Long Sword")));     // component
  EXPECT_FALSE(isTrackableItem(*data().itemByName("Doran's Blade")));  // starter, under 700g
}

TEST(DescribeItem, TurnsRiotHtmlIntoPlainLines) {
  Item item;
  item.description =
      "<mainText><stats><attention>65</attention> Attack Damage<br>25% Critical Strike Chance</stats><br><br>"
      "<passive>Perfection</passive><br>Critical strikes  deal bonus damage.</mainText>";
  EXPECT_EQ(describeItem(item),
            "65 Attack Damage\n25% Critical Strike Chance\nPerfection\nCritical strikes deal bonus damage.");
}

TEST(NameLess, IgnoresCase) {
  EXPECT_TRUE(nameLess("ahri", "Zed"));
  EXPECT_FALSE(nameLess("Zed", "ahri"));
  EXPECT_TRUE(nameLess("Kai'Sa", "Kalista"));  // punctuation sorts before letters
  EXPECT_FALSE(nameLess("Jinx", "Jinx"));
}

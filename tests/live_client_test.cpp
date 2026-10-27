#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include "fixture.h"
#include "live/live_client.h"

using nlohmann::json;

namespace {

json player(const std::string& champ, const std::string& display, const std::string& team, const std::string& position,
            const std::string& riotId, std::vector<int> itemIds) {
  json items = json::array();
  for (int id : itemIds) items.push_back({{"itemID", id}, {"displayName", std::to_string(id)}});
  return {
      {"championName", display},
      {"rawChampionName", "game_character_displayname_" + champ},
      {"riotId", riotId},
      {"riotIdGameName", riotId.substr(0, riotId.find('#'))},
      {"summonerName", riotId.substr(0, riotId.find('#'))},
      {"team", team},
      {"position", position},
      {"items", items},
  };
}

json game() {
  return {
      {"activePlayer", {{"riotId", "Me#NA1"}, {"riotIdGameName", "Me"}, {"summonerName", "Me"}}},
      {"allPlayers",
       {
           player("Jinx", "Jinx", "ORDER", "BOTTOM", "Me#NA1", {3006, 3031, 2003, 1036}),
           player("Darius", "Darius", "ORDER", "TOP", "Ally#NA1", {3047}),
           player("Malphite", "Malphite", "CHAOS", "TOP", "E1#NA1", {3075, 3110}),
           player("MonkeyKing", "Wukong", "CHAOS", "JUNGLE", "E2#NA1", {}),
           player("Zed", "Zed", "CHAOS", "BOTTOM", "E3#NA1", {3033}),
       }},
      {"gameData", {{"gameMode", "CLASSIC"}, {"gameTime", 754.5}}},
  };
}

}  // namespace

TEST(ParseLiveGame, ReadsYouYourItemsAndRole) {
  auto g = parseLiveGame(game().dump(), fixture::data());
  ASSERT_TRUE(g.has_value());
  EXPECT_EQ(g->you, "Jinx");
  EXPECT_EQ(g->role, Role::Bot);
  // Potion and Long Sword aren't tracked; boots and finished items are.
  EXPECT_EQ(g->yourItems, (std::vector<std::string>{"Berserker's Greaves", "Infinity Edge"}));
  EXPECT_DOUBLE_EQ(g->gameTime, 754.5);
  EXPECT_EQ(g->gameMode, "CLASSIC");
}

TEST(ParseLiveGame, ReadsEnemiesAndFindsLaneOpponent) {
  auto g = parseLiveGame(game().dump(), fixture::data());
  ASSERT_TRUE(g.has_value());
  ASSERT_EQ(g->enemies.size(), 3u);
  EXPECT_EQ(g->enemies[0].id, "Malphite");
  EXPECT_EQ(g->enemies[0].items, (std::vector<std::string>{"Thornmail", "Frozen Heart"}));
  EXPECT_EQ(g->enemies[1].id, "MonkeyKing");  // matched by rawChampionName, not the display name
  EXPECT_EQ(g->laneIndex, 2);                 // Zed is also BOTTOM
}

TEST(ParseLiveGame, FallsBackToDisplayNameAndActivePlayerNames) {
  json raw = game();
  raw["allPlayers"][3].erase("rawChampionName");  // Wukong
  raw["activePlayer"].erase("riotId");
  for (json& p : raw["allPlayers"]) p.erase("riotId");
  auto g = parseLiveGame(raw.dump(), fixture::data());
  ASSERT_TRUE(g.has_value());
  EXPECT_EQ(g->you, "Jinx");
  EXPECT_EQ(g->enemies[1].id, "MonkeyKing");
}

TEST(ParseLiveGame, NoPositionsMeansNoLaneOpponent) {
  json raw = game();
  for (json& p : raw["allPlayers"]) p["position"] = "";
  auto g = parseLiveGame(raw.dump(), fixture::data());
  ASSERT_TRUE(g.has_value());
  EXPECT_EQ(g->laneIndex, -1);
  EXPECT_EQ(g->role, std::nullopt);
}

TEST(ParseLiveGame, AtMostFiveEnemies) {
  json raw = game();
  for (int i = 0; i < 4; i++) raw["allPlayers"].push_back(player("Ahri", "Ahri", "CHAOS", "MIDDLE", "X#NA1", {}));
  auto g = parseLiveGame(raw.dump(), fixture::data());
  ASSERT_TRUE(g.has_value());
  EXPECT_EQ(g->enemies.size(), 5u);
}

TEST(ParseLiveGame, NothingWhileSpectatingOrLoading) {
  json spectating = game();
  spectating["activePlayer"] = {{"riotId", "Someone#Else"}};
  EXPECT_FALSE(parseLiveGame(spectating.dump(), fixture::data()).has_value());

  json loading = game();
  loading.erase("activePlayer");
  EXPECT_FALSE(parseLiveGame(loading.dump(), fixture::data()).has_value());

  EXPECT_FALSE(parseLiveGame("", fixture::data()).has_value());
  EXPECT_FALSE(parseLiveGame("{\"allPlayers\": 5}", fixture::data()).has_value());
  EXPECT_FALSE(parseLiveGame("<html>Service Unavailable</html>", fixture::data()).has_value());
}

TEST(ParseLiveGame, UnknownChampionIsSkipped) {
  json raw = game();
  raw["allPlayers"].push_back(player("Brandnewchamp", "Brand New", "CHAOS", "MIDDLE", "N#NA1", {}));
  auto g = parseLiveGame(raw.dump(), fixture::data());
  ASSERT_TRUE(g.has_value());
  EXPECT_EQ(g->enemies.size(), 3u);
}

TEST(LiveClientUrl, DefaultsToTheGameClient) {
  // Only checks the default; CI doesn't set LIVE_CLIENT_URL.
  if (std::getenv("LIVE_CLIENT_URL")) GTEST_SKIP();
  EXPECT_EQ(liveClientUrl(), "https://127.0.0.1:2999");
}

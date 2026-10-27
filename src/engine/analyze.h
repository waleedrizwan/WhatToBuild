#pragma once

#include <array>
#include <string>
#include <vector>

#include "data/champions.h"
#include "data/ddragon.h"
#include "data/items.h"
#include "engine/enemy_items.h"

inline constexpr double LANE_OPPONENT_WEIGHT = 1.6;

struct TeamAnalysis {
  std::vector<ThreatProfile> profiles;
  /** What each enemy has bought, parallel to `profiles`. */
  std::vector<EnemyBuild> builds;
  /** Index into `profiles`, or -1. */
  int laneOpponentIndex = -1;
  double adShare = 0.5;
  double apShare = 0.5;
  /** 0..1 — how much the enemy comp demands each kind of answer. */
  NeedValues needs{};
  /** Enemy champion names driving each need, biggest contributor first. */
  std::array<std::vector<std::string>, kNeedCount> sources;
  std::vector<std::string> stealth;
  std::vector<std::string> suppressors;
  /** Notes read from enemy items ("Malphite has 150 bonus armor…"). */
  std::vector<std::string> intel;

  double need(Need n) const { return needs[idx(n)]; }
  const std::vector<std::string>& sourcesOf(Need n) const { return sources[idx(n)]; }
  const ThreatProfile* laneOpponent() const;
};

struct Enemy {
  const Champion* champion;
  std::vector<const Item*> items;
};

TeamAnalysis analyzeEnemies(const std::vector<Enemy>& enemies, const std::string& laneOpponentId);

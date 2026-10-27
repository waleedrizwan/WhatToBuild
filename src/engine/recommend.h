#pragma once

#include <optional>
#include <string>
#include <vector>

#include "data/champions.h"
#include "data/ddragon.h"
#include "engine/analyze.h"

struct Pick {
  const Item* item = nullptr;
  double score = 0;
  std::vector<std::string> reasons;
  /** Already in your inventory (from the live game). */
  bool owned = false;
};

struct Build {
  Archetype archetype;
  std::vector<const Item*> starter;
  std::optional<Pick> boots;
  std::vector<Pick> core;
  std::vector<Pick> situational;
  std::vector<std::string> notes;
  int totalGold = 0;
};

Build recommendBuild(const GameData& data, const Champion& champion, Archetype archetype, Role role,
                     const TeamAnalysis& analysis, const std::vector<const Item*>& owned = {});

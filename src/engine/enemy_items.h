#pragma once
// Reads what an enemy has actually bought and folds it into their threat profile.
// A champion's kit says what they *usually* do; their items say what they're doing *this game*.

#include <string>
#include <utility>
#include <vector>

#include "data/champions.h"
#include "data/ddragon.h"

struct EnemyBuild {
  double armor = 0;
  double mr = 0;
  double hp = 0;
  double adItems = 0;
  double apItems = 0;
  double tankItems = 0;
  std::vector<std::string> grievous;
  std::vector<std::string> cleanse;
  std::vector<std::string> spellshield;
  std::vector<std::string> stasis;
  /** One-line intel notes about this enemy's build. */
  std::vector<std::string> intel;
};

EnemyBuild summarizeEnemyBuild(const std::string& name, const std::vector<const Item*>& items, double usualApShare);

/** Returns a copy of the profile adjusted for what the champion has actually built. */
std::pair<ThreatProfile, EnemyBuild> applyEnemyBuild(const ThreatProfile& p, const std::vector<const Item*>& items);

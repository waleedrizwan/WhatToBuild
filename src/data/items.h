#pragma once
// What each completed item is good for. `fit` is how naturally an item belongs
// in a build style (0-10); `counters` says which enemy needs it answers (0-1).
// Names must match Data Dragon exactly; unknown names are ignored at runtime.

#include <array>
#include <map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "data/champions.h"

enum class Need {
  Armor,
  Mr,
  Antiheal,
  Antishield,
  Armorpen,
  Magicpen,
  Antiburst,
  AntiCC,
  AntiAuto,
  AntiCrit,
  AntiPoke,
};
inline constexpr std::size_t kNeedCount = 11;
inline constexpr std::array<Need, kNeedCount> kNeeds = {
    Need::Armor,     Need::Mr,     Need::Antiheal, Need::Antishield, Need::Armorpen, Need::Magicpen,
    Need::Antiburst, Need::AntiCC, Need::AntiAuto, Need::AntiCrit,   Need::AntiPoke,
};

/** e.g. "armor", "antiCC". */
std::string_view needKey(Need n);

using NeedValues = std::array<double, kNeedCount>;
using FitTable = std::map<Archetype, double>;
/** Kept in the order written; ties in scoring explain the first-listed need first. */
using Counters = std::vector<std::pair<Need, double>>;

struct ItemKnowledge {
  std::string name;
  FitTable fit;
  Counters counters;
  /** Items sharing a group have a unique passive — only one per build. */
  std::string group;
  /** Team-utility items that only make sense in the support role. */
  bool supportOnly = false;
};

struct BootsKnowledge {
  std::string name;
  FitTable fit;
  Counters counters;
};

const std::vector<ItemKnowledge>& itemKnowledge();
const ItemKnowledge* findItemKnowledge(std::string_view name);
const std::vector<BootsKnowledge>& bootsKnowledge();

/** Grievous Wounds items. At most one is ever worth it. */
bool isAntihealItem(std::string_view name);

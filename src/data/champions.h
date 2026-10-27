#pragma once
// Hand-curated champion knowledge layered on top of Data Dragon's tags/info.
// Data Dragon doesn't say *how* a champion deals damage or what threats it poses,
// so these tables encode that. Champions not listed fall back to heuristics.

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "data/ddragon.h"
#include "util/enum.h"

enum class Archetype {
  AdcCrit,
  AdcOnHit,
  AdCaster,
  AdAssassin,
  Bruiser,
  Tank,
  ApMage,
  ApBattlemage,
  ApAssassin,
  ApOnHit,
  Enchanter,
};
inline constexpr std::size_t kArchetypeCount = 11;
inline constexpr std::array<Archetype, kArchetypeCount> kArchetypes = {
    Archetype::AdcCrit, Archetype::AdcOnHit,     Archetype::AdCaster,   Archetype::AdAssassin,
    Archetype::Bruiser, Archetype::Tank,         Archetype::ApMage,     Archetype::ApBattlemage,
    Archetype::ApAssassin, Archetype::ApOnHit,   Archetype::Enchanter,
};

/** e.g. "Crit marksman". */
std::string_view archetypeLabel(Archetype a);
/** Stable id used in saved settings and logs, e.g. "adc_crit". */
std::string_view archetypeKey(Archetype a);
std::optional<Archetype> archetypeFromKey(std::string_view key);
/** The "ap_*" build styles (not enchanter). */
bool isApArchetype(Archetype a);

enum class Role { Top, Jungle, Mid, Bot, Support };
inline constexpr std::size_t kRoleCount = 5;
inline constexpr std::array<Role, kRoleCount> kRoles = {Role::Top, Role::Jungle, Role::Mid, Role::Bot, Role::Support};

std::string_view roleLabel(Role r);
std::string_view roleKey(Role r);
std::optional<Role> roleFromKey(std::string_view key);

/** Weighted threat traits. 1 = defining trait, 0.5 = notable. */
enum class Trait { Heal, Shield, CC, Suppress, Burst, Auto, Crit, Poke, Stealth };
inline constexpr std::size_t kTraitCount = 9;
using Traits = std::array<double, kTraitCount>;

/** Item names that are signature picks for a champion; they get a fit bonus. */
const std::vector<std::string>& signatureItems(const std::string& championId);

Archetype archetypeFor(const Champion& c);
Role defaultRoleFor(const Champion& c);

struct ThreatProfile {
  const Champion* champion = nullptr;
  /** Share of damage that is magic (0..1). */
  double apShare = 0;
  /** How much of the enemy team's damage this champion represents. */
  double damageWeight = 1;
  bool isTank = false;
  bool isRanged = false;
  Traits traits{};

  double trait(Trait t) const { return traits[idx(t)]; }
};

ThreatProfile threatProfile(const Champion& c);

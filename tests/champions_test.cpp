#include <gtest/gtest.h>

#include "data/champions.h"
#include "fixture.h"

using fixture::champ;

TEST(ArchetypeFor, UsesOverridesFirst) {
  EXPECT_EQ(archetypeFor(champ("Malphite")), Archetype::Tank);
  EXPECT_EQ(archetypeFor(champ("Soraka")), Archetype::Enchanter);
  EXPECT_EQ(archetypeFor(champ("Zed")), Archetype::AdAssassin);
}

TEST(ArchetypeFor, FallsBackToTagsAndInfo) {
  EXPECT_EQ(archetypeFor(champ("Jinx")), Archetype::AdcCrit);     // Marksman
  EXPECT_EQ(archetypeFor(champ("Ahri")), Archetype::ApAssassin);  // Mage + Assassin
  EXPECT_EQ(archetypeFor(champ("Darius")), Archetype::Bruiser);   // Fighter, attack > magic
}

TEST(DefaultRoleFor, PicksLaneFromArchetypeAndRange) {
  EXPECT_EQ(defaultRoleFor(champ("Jinx")), Role::Bot);
  EXPECT_EQ(defaultRoleFor(champ("Soraka")), Role::Support);
  EXPECT_EQ(defaultRoleFor(champ("Ahri")), Role::Mid);
  EXPECT_EQ(defaultRoleFor(champ("Zed")), Role::Mid);
  EXPECT_EQ(defaultRoleFor(champ("Darius")), Role::Top);
}

TEST(ThreatProfile, MarksmenAutoAttackAndCrit) {
  ThreatProfile p = threatProfile(champ("Jinx"));
  EXPECT_DOUBLE_EQ(p.trait(Trait::Auto), 1);
  EXPECT_DOUBLE_EQ(p.trait(Trait::Crit), 1);
  EXPECT_TRUE(p.isRanged);
  EXPECT_DOUBLE_EQ(p.apShare, 0.1);
}

TEST(ThreatProfile, ReadsTablesAndTags) {
  ThreatProfile malphite = threatProfile(champ("Malphite"));
  EXPECT_TRUE(malphite.isTank);
  EXPECT_DOUBLE_EQ(malphite.apShare, 0.7);
  EXPECT_DOUBLE_EQ(malphite.damageWeight, 0.6);
  EXPECT_DOUBLE_EQ(threatProfile(champ("Soraka")).trait(Trait::Heal), 1.2);
  EXPECT_DOUBLE_EQ(threatProfile(champ("Soraka")).damageWeight, 0.45);
  EXPECT_GE(threatProfile(champ("Zed")).trait(Trait::Burst), 0.7);
}

TEST(Keys, RoundTrip) {
  for (Archetype a : kArchetypes) EXPECT_EQ(archetypeFromKey(archetypeKey(a)), a);
  for (Role r : kRoles) EXPECT_EQ(roleFromKey(roleKey(r)), r);
  EXPECT_EQ(archetypeFromKey("nope"), std::nullopt);
  EXPECT_EQ(roleFromKey("auto"), std::nullopt);
}

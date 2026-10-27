#include "engine/enemy_items.h"

#include <algorithm>
#include <format>
#include <functional>
#include <unordered_set>

namespace {

const std::unordered_set<std::string> HEALING_ITEMS = {
    "Bloodthirster", "Ravenous Hydra", "Riftmaker", "Sundered Sky", "Spirit Visage", "Warmog's Armor",
    "Moonstone Renewer", "Echoes of Helia", "Redemption", "Blade of The Ruined King", "Death's Dance",
    "Overlord's Bloodmail", "Unending Despair", "Endless Hunger", "Hextech Gunblade", "Maw of Malmortius",
    "Dusk and Dawn", "Mercurial Scimitar", "Rod of Ages", "Protoplasm Harness", "Heartsteel", "Kaenic Rookern",
};

const std::unordered_set<std::string> SHIELD_ITEMS = {
    "Immortal Shieldbow", "Sterak's Gage", "Maw of Malmortius", "Locket of the Iron Solari", "Eclipse",
    "Archangel's Staff", "Seraph's Embrace", "Moonstone Renewer", "Kaenic Rookern", "Ardent Censer",
    "Echoes of Helia", "Hextech Rocketbelt",
};

const std::unordered_set<std::string> GRIEVOUS_ITEMS = {"Mortal Reminder", "Chempunk Chainsword", "Morellonomicon",
                                                        "Thornmail"};
const std::unordered_set<std::string> CLEANSE_ITEMS = {"Mercurial Scimitar", "Quicksilver Sash"};
const std::unordered_set<std::string> SPELLSHIELD_ITEMS = {"Banshee's Veil", "Edge of Night"};
const std::unordered_set<std::string> STASIS_ITEMS = {"Zhonya's Hourglass", "Guardian Angel"};

bool isHealing(const Item& i) {
  return HEALING_ITEMS.contains(i.name) || i.hasTag("LifeSteal") || i.hasTag("SpellVamp");
}

std::string firstThree(const std::vector<std::string>& xs) {
  std::string out;
  for (std::size_t i = 0; i < xs.size() && i < 3; i++) out += (i ? ", " : "") + xs[i];
  return out;
}

}  // namespace

EnemyBuild summarizeEnemyBuild(const std::string& name, const std::vector<const Item*>& items, double usualApShare) {
  EnemyBuild b;
  std::vector<std::string> armorItems, mrItems, healItems, critItems;

  for (const Item* item : items) {
    double armor = item->stat("FlatArmorMod");
    double mr = item->stat("FlatSpellBlockMod");
    double hp = item->stat("FlatHPPoolMod");
    b.armor += armor;
    b.mr += mr;
    b.hp += hp;
    if (armor >= 30) armorItems.push_back(item->name);
    if (mr >= 30) mrItems.push_back(item->name);

    bool ad = item->stat("FlatPhysicalDamageMod") > 0 || item->hasTag("CriticalStrike") ||
              item->hasTag("ArmorPenetration");
    bool ap = item->stat("FlatMagicDamageMod") > 0 || item->hasTag("SpellDamage");
    if (!item->isBoots) {
      if (ad && ap) {
        b.adItems += 0.5;
        b.apItems += 0.5;
      } else if (ad) b.adItems += 1;
      else if (ap) b.apItems += 1;
      else if (armor + mr >= 40 || hp >= 350) b.tankItems += 1;
    }

    if (isHealing(*item)) healItems.push_back(item->name);
    if (item->hasTag("CriticalStrike")) critItems.push_back(item->name);
    if (GRIEVOUS_ITEMS.contains(item->name)) b.grievous.push_back(item->name);
    if (CLEANSE_ITEMS.contains(item->name)) b.cleanse.push_back(item->name);
    if (SPELLSHIELD_ITEMS.contains(item->name)) b.spellshield.push_back(item->name);
    if (STASIS_ITEMS.contains(item->name)) b.stasis.push_back(item->name);
  }

  auto& intel = b.intel;
  if (b.armor >= 60)
    intel.push_back(std::format("{} has {} bonus armor ({}): buy armor penetration", name, b.armor, firstThree(armorItems)));
  if (b.mr >= 60)
    intel.push_back(std::format("{} has {} bonus MR ({}): buy magic penetration", name, b.mr, firstThree(mrItems)));
  // Only worth saying when it's not their usual damage type.
  if (b.apItems > b.adItems && b.apItems >= 1 && usualApShare < 0.6)
    intel.push_back(name + " is building AP, so their damage is mostly magic");
  else if (b.adItems > b.apItems && b.adItems >= 1 && usualApShare > 0.4)
    intel.push_back(name + " is building AD, so their damage is mostly physical");
  if (healItems.size() >= 2) intel.push_back(std::format("{} has lots of sustain ({})", name, firstThree(healItems)));
  if (critItems.size() >= 2) intel.push_back(std::format("{} has {} crit items", name, critItems.size()));
  if (!b.grievous.empty())
    intel.push_back(std::format("{} has Grievous Wounds ({}): your healing gets cut", name, firstThree(b.grievous)));
  if (!b.cleanse.empty()) intel.push_back(std::format("{} can cleanse CC ({})", name, firstThree(b.cleanse)));
  if (!b.stasis.empty())
    intel.push_back(std::format("{} has {}, so time your burst around it", name, firstThree(b.stasis)));
  return b;
}

std::pair<ThreatProfile, EnemyBuild> applyEnemyBuild(const ThreatProfile& p, const std::vector<const Item*>& items) {
  EnemyBuild build = summarizeEnemyBuild(p.champion->name, items, p.apShare);
  if (items.empty()) return {p, build};

  ThreatProfile out = p;
  double dmgItems = build.adItems + build.apItems;
  if (dmgItems > 0) {
    // Items beat kit stereotypes: an AP Kai'Sa or AD Shyvana shifts the damage split.
    out.apShare = p.apShare * 0.4 + (build.apItems / dmgItems) * 0.6;
  }
  if (build.tankItems >= 2 && build.tankItems > dmgItems) {
    out.isTank = true;
    out.damageWeight *= 0.75;
  } else if (dmgItems >= 2 && p.isTank) {
    // A "tank" building damage hits much harder.
    out.damageWeight = std::max(out.damageWeight, 0.9);
  }

  auto count = [&](const std::function<bool(const Item&)>& pred) {
    return static_cast<double>(std::count_if(items.begin(), items.end(), [&](const Item* i) { return pred(*i); }));
  };
  auto& t = out.traits;
  t[idx(Trait::Heal)] += 0.35 * count(isHealing);
  t[idx(Trait::Shield)] += 0.4 * count([](const Item& i) { return SHIELD_ITEMS.contains(i.name); });
  t[idx(Trait::Crit)] += 0.35 * count([](const Item& i) { return i.hasTag("CriticalStrike"); });
  t[idx(Trait::Auto)] += 0.25 * count([](const Item& i) { return !i.isBoots && i.hasTag("AttackSpeed"); });
  t[idx(Trait::Burst)] +=
      0.2 * count([](const Item& i) { return i.hasTag("ArmorPenetration") || i.hasTag("MagicPenetration"); });
  for (double& v : t) v = std::min(v, 1.5);

  return {out, build};
}

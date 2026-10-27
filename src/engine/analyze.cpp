#include "engine/analyze.h"

#include <algorithm>

namespace {

double clamp01(double n) { return std::max(0.0, std::min(1.0, n)); }

}  // namespace

const ThreatProfile* TeamAnalysis::laneOpponent() const {
  return laneOpponentIndex >= 0 ? &profiles[laneOpponentIndex] : nullptr;
}

TeamAnalysis analyzeEnemies(const std::vector<Enemy>& enemies, const std::string& laneOpponentId) {
  TeamAnalysis a;
  for (const Enemy& e : enemies) {
    auto [profile, build] = applyEnemyBuild(threatProfile(*e.champion), e.items);
    a.profiles.push_back(profile);
    a.builds.push_back(std::move(build));
  }
  auto weightOf = [&](const ThreatProfile& p) { return p.champion->id == laneOpponentId ? LANE_OPPONENT_WEIGHT : 1.0; };

  double ad = 0;
  double ap = 0;
  for (const ThreatProfile& p : a.profiles) {
    double w = weightOf(p) * p.damageWeight;
    ap += w * p.apShare;
    ad += w * (1 - p.apShare);
  }
  double total = ad + ap;
  if (total == 0) total = 1;
  bool any = !a.profiles.empty();
  a.adShare = any ? ad / total : 0.5;
  a.apShare = any ? ap / total : 0.5;

  // Per-champion contribution to each need, so we can both sum and explain.
  std::vector<NeedValues> contrib;
  for (std::size_t i = 0; i < a.profiles.size(); i++) {
    const ThreatProfile& p = a.profiles[i];
    const EnemyBuild& b = a.builds[i];
    double w = weightOf(p);
    double dmg = w * p.damageWeight;
    double tankiness = p.isTank ? 1 + (p.champion->info.defense >= 8 ? 0.3 : 0) : 0;
    // Bought resistances count on top of the kit: 150 bonus armor ≈ one more tank to chew through.
    double hpBulk = b.hp / 2000;
    NeedValues c{};
    c[idx(Need::Armor)] = dmg * (1 - p.apShare);
    c[idx(Need::Mr)] = dmg * p.apShare;
    c[idx(Need::Antiheal)] = w * p.trait(Trait::Heal);
    c[idx(Need::Antishield)] = w * p.trait(Trait::Shield);
    c[idx(Need::Armorpen)] = w * (tankiness + b.armor / 150 + hpBulk);
    c[idx(Need::Magicpen)] = w * (tankiness + b.mr / 150 + hpBulk);
    c[idx(Need::Antiburst)] = w * p.trait(Trait::Burst);
    c[idx(Need::AntiCC)] = w * (p.trait(Trait::CC) + p.trait(Trait::Suppress) * 0.6);
    c[idx(Need::AntiAuto)] = dmg * p.trait(Trait::Auto);
    c[idx(Need::AntiCrit)] = w * p.trait(Trait::Crit);
    c[idx(Need::AntiPoke)] = w * p.trait(Trait::Poke);
    contrib.push_back(c);
  }
  auto sum = [&](Need n) {
    double s = 0;
    for (const NeedValues& c : contrib) s += c[idx(n)];
    return s;
  };

  auto& needs = a.needs;
  needs[idx(Need::Armor)] = any ? clamp01((a.adShare - 0.3) / 0.5) : 0;
  needs[idx(Need::Mr)] = any ? clamp01((a.apShare - 0.3) / 0.5) : 0;
  needs[idx(Need::Antiheal)] = clamp01(sum(Need::Antiheal) / 1.6);
  needs[idx(Need::Antishield)] = clamp01(sum(Need::Antishield) / 2);
  needs[idx(Need::Armorpen)] = clamp01(sum(Need::Armorpen) / 3);
  needs[idx(Need::Magicpen)] = clamp01(sum(Need::Magicpen) / 3);
  needs[idx(Need::Antiburst)] = clamp01(sum(Need::Antiburst) / 1.8);
  needs[idx(Need::AntiCC)] = clamp01(sum(Need::AntiCC) / 3);
  needs[idx(Need::AntiAuto)] = clamp01(sum(Need::AntiAuto) / 2);
  needs[idx(Need::AntiCrit)] = clamp01(sum(Need::AntiCrit) / 1.5);
  needs[idx(Need::AntiPoke)] = clamp01(sum(Need::AntiPoke) / 1.8);

  for (Need n : kNeeds) {
    // Penetration sources must be genuinely tanky, not just someone with one defensive item.
    double threshold = n == Need::Armor || n == Need::Mr            ? 0.5
                       : n == Need::Armorpen || n == Need::Magicpen ? 0.9
                                                                    : 0.3;
    std::vector<std::size_t> order;
    for (std::size_t i = 0; i < contrib.size(); i++)
      if (contrib[i][idx(n)] >= threshold) order.push_back(i);
    std::stable_sort(order.begin(), order.end(),
                     [&](std::size_t x, std::size_t y) { return contrib[x][idx(n)] > contrib[y][idx(n)]; });
    for (std::size_t i : order) a.sources[idx(n)].push_back(a.profiles[i].champion->name);
  }

  for (std::size_t i = 0; i < a.profiles.size(); i++) {
    const ThreatProfile& p = a.profiles[i];
    if (a.laneOpponentIndex < 0 && p.champion->id == laneOpponentId) a.laneOpponentIndex = static_cast<int>(i);
    if (p.trait(Trait::Stealth) >= 0.5) a.stealth.push_back(p.champion->name);
    if (p.trait(Trait::Suppress) >= 1) a.suppressors.push_back(p.champion->name);
  }
  for (const EnemyBuild& b : a.builds) a.intel.insert(a.intel.end(), b.intel.begin(), b.intel.end());
  return a;
}

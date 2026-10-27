#include "engine/recommend.h"

#include <algorithm>
#include <cctype>
#include <unordered_set>

namespace {

using enum Archetype;

constexpr int CORE_SLOTS = 5;
// Early slots lean on what the champion wants; later slots react to the enemy.
constexpr std::array<double, CORE_SLOTS> FIT_WEIGHT = {1, 1, 0.9, 0.8, 0.7};
constexpr std::array<double, CORE_SLOTS> COUNTER_WEIGHT = {0.15, 0.3, 0.6, 0.8, 0.9};

bool isOffensiveNeed(Need n) {
  return n == Need::Antiheal || n == Need::Antishield || n == Need::Armorpen || n == Need::Magicpen;
}

bool isSquishy(Archetype a) {
  return a == AdcCrit || a == AdcOnHit || a == ApMage || a == ApAssassin || a == AdAssassin || a == Enchanter ||
         a == AdCaster;
}

std::string needPhrase(Need n, const std::string& who) {
  switch (n) {
    case Need::Armor: return "Armor vs heavy physical damage" + who;
    case Need::Mr: return "Magic resist vs heavy magic damage" + who;
    case Need::Antiheal: return "Grievous Wounds to cut healing" + who;
    case Need::Antishield: return "Breaks shields" + who;
    case Need::Armorpen: return "Armor penetration vs tanky / armor-stacking enemies" + who;
    case Need::Magicpen: return "Magic penetration vs tanky / MR-stacking enemies" + who;
    case Need::Antiburst: return "Survives burst / assassination" + who;
    case Need::AntiCC: return "Answers crowd control" + who;
    case Need::AntiAuto: return "Slows down auto-attackers" + who;
    case Need::AntiCrit: return "Reduces crit damage" + who;
    case Need::AntiPoke: return "Sustain through poke" + who;
  }
  return who;
}

std::string joinFirst(const std::vector<std::string>& names, std::size_t n) {
  std::string out;
  for (std::size_t i = 0; i < names.size() && i < n; i++) out += (i ? ", " : "") + names[i];
  return out;
}

/** "Battlemage (sustained AP)" → "battlemage (sustained AP)". */
std::string styleName(Archetype archetype) {
  std::string s(archetypeLabel(archetype));
  for (char& ch : s) ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  auto isWord = [](char ch) { return std::isalnum(static_cast<unsigned char>(ch)) || ch == '_'; };
  for (std::size_t i = 0; i < s.size();) {
    if (!isWord(s[i])) {
      i++;
      continue;
    }
    std::size_t j = i;
    while (j < s.size() && isWord(s[j])) j++;
    if (std::string_view word(s.data() + i, j - i); word == "ap" || word == "ad")
      for (std::size_t k = i; k < j; k++) s[k] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[k])));
    i = j;
  }
  return s;
}

std::string whoText(const std::vector<std::string>& names) {
  if (names.empty()) return "";
  return " (" + joinFirst(names, 3) + ")";
}

double fitOf(const FitTable& fit, Archetype a, double fallback) {
  auto it = fit.find(a);
  return it == fit.end() ? fallback : it->second;
}

/** The unique-passive group an item belongs to, or "" for none. */
std::string groupOf(const std::string& name) {
  if (const ItemKnowledge* k = findItemKnowledge(name); k && !k->group.empty()) return k->group;
  return isAntihealItem(name) ? "antiheal" : "";
}

struct Candidate {
  const Item* item;
  double fit;
  const Counters* counters;
  std::string group;
  bool signature;
};

std::vector<Candidate> candidatesFor(const GameData& data, const Champion& champion, Archetype archetype, Role role) {
  const auto& signatures = signatureItems(champion.id);
  std::vector<Candidate> out;
  for (const ItemKnowledge& k : itemKnowledge()) {
    const Item* item = data.itemByName(k.name);
    if (!item) continue;  // removed or renamed this patch
    if (k.supportOnly && role != Role::Support) continue;
    bool signature = std::find(signatures.begin(), signatures.end(), k.name) != signatures.end();
    double fit = fitOf(k.fit, archetype, 0) + (signature ? 3 : 0);
    if (fit <= 0) continue;
    out.push_back({item, fit, &k.counters, groupOf(k.name), signature});
  }
  return out;
}

/** Carries can't afford to over-itemize defensively; tanks don't care about penetration. */
double needScale(Archetype archetype, Need need) {
  if (isOffensiveNeed(need)) return archetype == Tank ? 0.5 : 1;
  if (archetype == Tank) return 1;
  return isSquishy(archetype) ? 0.6 : 0.8;
}

Pick scoreCandidate(const Candidate& c, int slot, const TeamAnalysis& analysis, Archetype archetype) {
  int s = std::min(slot, CORE_SLOTS - 1);
  double score = c.fit * FIT_WEIGHT[s];
  std::vector<std::pair<Need, double>> contributions;
  for (const auto& [need, amount] : *c.counters) {
    double v = amount * analysis.need(need) * 8 * COUNTER_WEIGHT[s] * needScale(archetype, need);
    score += v;
    if (v >= 1) contributions.emplace_back(need, v);
  }
  std::stable_sort(contributions.begin(), contributions.end(),
                   [](const auto& a, const auto& b) { return a.second > b.second; });

  std::vector<std::string> reasons;
  for (const auto& [need, v] : contributions) reasons.push_back(needPhrase(need, whoText(analysis.sourcesOf(need))));
  if (c.signature) reasons.insert(reasons.begin(), "Signature item for this champion");
  else if (c.fit >= 8) reasons.insert(reasons.begin(), "Core item for " + styleName(archetype) + " builds");
  else if (reasons.empty()) reasons.push_back("Solid stats for " + styleName(archetype) + " builds");
  return {c.item, score, std::move(reasons)};
}

std::optional<Pick> pickBoots(const GameData& data, Archetype archetype, const TeamAnalysis& analysis) {
  double squish = isSquishy(archetype) ? 0.7 : 1;
  std::optional<Pick> best;
  for (const BootsKnowledge& k : bootsKnowledge()) {
    const Item* item = data.itemByName(k.name);
    if (!item) continue;
    double score = fitOf(k.fit, archetype, 1);
    std::vector<std::string> reasons;
    for (const auto& [need, amount] : k.counters) {
      double v = amount * analysis.need(need) * 8 * squish;
      score += v;
      if (v >= 2) reasons.push_back(needPhrase(need, whoText(analysis.sourcesOf(need))));
    }
    if (reasons.empty()) reasons.push_back("Standard boots for " + styleName(archetype) + " builds");
    if (!best || score > best->score) best = Pick{item, score, std::move(reasons)};
  }
  return best;
}

std::vector<const Item*> pickStarter(const GameData& data, const Champion& champion, Archetype archetype, Role role,
                                     const TeamAnalysis& analysis) {
  std::string first;
  const ThreatProfile* lane = analysis.laneOpponent();
  if (role == Role::Support) first = "World Atlas";
  else if (role == Role::Jungle) {
    first = archetype == Tank                                               ? "Mosstomper Seedling"
            : archetype == Bruiser || archetype == AdcOnHit || archetype == AdcCrit ? "Gustwalker Hatchling"
                                                                            : "Scorchclaw Pup";
  } else if (isApArchetype(archetype)) first = "Doran's Ring";
  else if (archetype == Tank) first = "Doran's Helm";
  else if (role == Role::Top && champion.attackRange < 300 && lane && lane->isRanged) first = "Doran's Shield";
  else first = "Doran's Blade";

  std::vector<const Item*> out;
  const Item* starter = data.itemByName(first);
  if (!starter) starter = data.itemByName("Doran's Blade");
  if (starter) out.push_back(starter);
  int potions = role == Role::Jungle || role == Role::Support ? 1 : 2;
  if (const Item* potion = data.itemByName("Health Potion"))
    for (int i = 0; i < potions; i++) out.push_back(potion);
  return out;
}

std::vector<std::string> buildNotes(const TeamAnalysis& analysis, Archetype archetype, const std::vector<Pick>& core) {
  std::vector<std::string> notes;
  bool isAP = isApArchetype(archetype) || archetype == Enchanter;

  if (analysis.need(Need::Antiheal) >= 0.5) {
    std::string component = isAP ? "Oblivion Orb" : archetype == Tank ? "Bramble Vest" : "Executioner's Calling";
    bool hasGW = std::any_of(core.begin(), core.end(), [](const Pick& p) { return isAntihealItem(p.item->name); });
    notes.push_back("Heavy healing on the enemy team (" + joinFirst(analysis.sourcesOf(Need::Antiheal), 3) + "). " +
                    "Pick up " + component + " early" +
                    (hasGW ? " and finish it into your Grievous Wounds item" : "") +
                    " — even the component is worth it.");
  }
  if (!analysis.suppressors.empty()) {
    notes.push_back(joinFirst(analysis.suppressors, analysis.suppressors.size()) +
                    " can lock you down with suppression/ultimate CC — Quicksilver Sash "
                    "(or Mercurial Scimitar) removes it, and Cleanse is a strong summoner spell.");
  }
  std::vector<std::string> grievousOwners;
  for (std::size_t i = 0; i < analysis.profiles.size(); i++)
    if (!analysis.builds[i].grievous.empty()) grievousOwners.push_back(analysis.profiles[i].champion->name);
  if (!grievousOwners.empty() &&
      (archetype == Bruiser || archetype == ApBattlemage || archetype == Enchanter || archetype == AdcOnHit)) {
    notes.push_back(joinFirst(grievousOwners, grievousOwners.size()) +
                    " already bought Grievous Wounds, so your healing is cut. "
                    "Resistances and shields are worth more than extra lifesteal or healing.");
  }
  if (!analysis.stealth.empty()) {
    notes.push_back(joinFirst(analysis.stealth, analysis.stealth.size()) +
                    " use stealth — carry Control Wards and consider Oracle Lens.");
  }
  if (analysis.need(Need::AntiPoke) >= 0.6 && isSquishy(archetype)) {
    notes.push_back("Poke-heavy enemy comp (" + joinFirst(analysis.sourcesOf(Need::AntiPoke), 3) +
                    ") — consider Second Wind/Bone Plating runes and extra potions.");
  }
  if (analysis.need(Need::Antiburst) >= 0.7 && isSquishy(archetype)) {
    notes.push_back("Lots of burst — a Stopwatch / early defensive component can save a back-line carry.");
  }
  return notes;
}

}  // namespace

Build recommendBuild(const GameData& data, const Champion& champion, Archetype archetype, Role role,
                     const TeamAnalysis& analysis, const std::vector<const Item*>& owned) {
  std::vector<Candidate> candidates = candidatesFor(data, champion, archetype, role);
  std::vector<Pick> core;
  std::unordered_set<std::string> usedGroups;
  std::unordered_set<std::string> used;

  // What you've already bought stays in the build; the engine plans the rest around it.
  const Item* ownedBoots = nullptr;
  for (const Item* item : owned) {
    if (item->isBoots) {
      if (!ownedBoots) ownedBoots = item;
      continue;
    }
    core.push_back({item, 0, {"Already bought"}, true});
    used.insert(item->name);
    if (std::string g = groupOf(item->name); !g.empty()) usedGroups.insert(g);
  }

  for (int slot = static_cast<int>(core.size()); slot < CORE_SLOTS; slot++) {
    std::optional<Pick> best;
    const Candidate* bestCandidate = nullptr;
    for (const Candidate& c : candidates) {
      if (used.contains(c.item->name) || (!c.group.empty() && usedGroups.contains(c.group))) continue;
      Pick p = scoreCandidate(c, slot, analysis, archetype);
      if (!best || p.score > best->score) {
        best = std::move(p);
        bestCandidate = &c;
      }
    }
    if (!best) break;
    used.insert(best->item->name);
    if (!bestCandidate->group.empty()) usedGroups.insert(bestCandidate->group);
    core.push_back(std::move(*best));
  }

  // Situational: items that answer the comp better than they fit generically.
  std::vector<Pick> situational;
  for (const Candidate& c : candidates) {
    if (used.contains(c.item->name) || c.counters->empty()) continue;
    Pick p = scoreCandidate(c, CORE_SLOTS - 1, analysis, archetype);
    double counterValue = p.score - c.fit * FIT_WEIGHT.back();
    if (counterValue >= 1.5) situational.push_back(std::move(p));
  }
  std::stable_sort(situational.begin(), situational.end(),
                   [](const Pick& a, const Pick& b) { return a.score > b.score; });
  if (situational.size() > 5) situational.resize(5);
  for (Pick& p : situational) {
    std::erase_if(p.reasons, [](const std::string& r) { return r.starts_with("Core item") || r.starts_with("Solid stats"); });
  }

  std::optional<Pick> boots =
      ownedBoots ? std::optional<Pick>(Pick{ownedBoots, 0, {"Already bought"}, true}) : pickBoots(data, archetype, analysis);
  int totalGold = boots ? boots->item->gold : 0;
  for (const Pick& p : core) totalGold += p.item->gold;
  std::vector<std::string> notes = buildNotes(analysis, archetype, core);

  return {
      .archetype = archetype,
      .starter = pickStarter(data, champion, archetype, role, analysis),
      .boots = std::move(boots),
      .core = std::move(core),
      .situational = std::move(situational),
      .notes = std::move(notes),
      .totalGold = totalGold,
  };
}

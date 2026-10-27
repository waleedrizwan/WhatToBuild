#include "data/champions.h"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

namespace {

using enum Archetype;

constexpr std::array<std::string_view, kArchetypeCount> ARCHETYPE_LABELS = {
    "Crit marksman",        "On-hit attacker",           "AD caster",   "Lethality assassin",
    "Bruiser / fighter",    "Tank",                      "Burst / control mage",
    "Battlemage (sustained AP)", "AP assassin",          "AP on-hit",   "Enchanter support",
};

constexpr std::array<std::string_view, kArchetypeCount> ARCHETYPE_KEYS = {
    "adc_crit", "adc_onhit",     "ad_caster",   "ad_assassin", "bruiser",  "tank",
    "ap_mage",  "ap_battlemage", "ap_assassin", "ap_onhit",    "enchanter",
};

constexpr std::array<std::string_view, kRoleCount> ROLE_LABELS = {"Top", "Jungle", "Mid", "Bot (ADC)", "Support"};
constexpr std::array<std::string_view, kRoleCount> ROLE_KEYS = {"top", "jungle", "mid", "bot", "support"};

const std::unordered_map<std::string, Archetype> ARCHETYPE_OVERRIDES = {
    // On-hit / attack-speed scalers
    {"Vayne", AdcOnHit}, {"KogMaw", AdcOnHit}, {"Kalista", AdcOnHit}, {"Varus", AdcOnHit},
    {"Kaisa", AdcOnHit}, {"MasterYi", AdcOnHit}, {"Belveth", AdcOnHit}, {"Kindred", AdcCrit},
    // Crit melee
    {"Yasuo", AdcCrit}, {"Yone", AdcCrit}, {"Tryndamere", AdcCrit}, {"Nilah", AdcCrit},
    {"Gangplank", AdcCrit},
    // AD casters
    {"Ezreal", AdCaster}, {"Jayce", AdCaster}, {"Corki", AdCaster}, {"Senna", AdCaster},
    {"Pantheon", AdCaster}, {"Aphelios", AdcCrit},
    // Lethality
    {"Zed", AdAssassin}, {"Talon", AdAssassin}, {"Khazix", AdAssassin}, {"Rengar", AdAssassin},
    {"Qiyana", AdAssassin}, {"Naafiri", AdAssassin}, {"Pyke", AdAssassin}, {"Kayn", AdAssassin},
    {"Shaco", AdAssassin}, {"Quinn", AdAssassin}, {"Akshan", AdcCrit},
    // AP assassins
    {"Fizz", ApAssassin}, {"Ekko", ApAssassin}, {"Kassadin", ApAssassin}, {"Akali", ApAssassin},
    {"Evelynn", ApAssassin}, {"Diana", ApAssassin}, {"Katarina", ApAssassin}, {"Leblanc", ApAssassin},
    {"Elise", ApAssassin}, {"Nidalee", ApAssassin},
    // Battlemages / sustained AP
    {"Mordekaiser", ApBattlemage}, {"Rumble", ApBattlemage}, {"Lillia", ApBattlemage},
    {"Brand", ApBattlemage}, {"Cassiopeia", ApBattlemage}, {"Ryze", ApBattlemage},
    {"Swain", ApBattlemage}, {"Vladimir", ApBattlemage}, {"Sylas", ApBattlemage},
    {"Karthus", ApBattlemage}, {"Fiddlesticks", ApBattlemage}, {"Singed", ApBattlemage},
    {"Aurora", ApMage},
    // AP on-hit
    {"Kayle", ApOnHit}, {"Gwen", ApOnHit}, {"Teemo", ApOnHit},
    // Tanks that DDragon tags as fighters (or vice versa)
    {"Sejuani", Tank}, {"Zac", Tank}, {"Sion", Tank}, {"Ornn", Tank}, {"Chogath", Tank},
    {"Malphite", Tank}, {"KSante", Tank}, {"Poppy", Tank}, {"Rammus", Tank}, {"Amumu", Tank},
    {"DrMundo", Tank}, {"TahmKench", Tank}, {"Maokai", Tank}, {"Nunu", Tank}, {"Skarner", Tank},
    {"Volibear", Bruiser}, {"Shen", Tank}, {"Galio", Tank},
    // Supports
    {"Leona", Tank}, {"Nautilus", Tank}, {"Thresh", Tank}, {"Alistar", Tank}, {"Braum", Tank},
    {"Rell", Tank}, {"Blitzcrank", Tank}, {"Taric", Tank}, {"Rakan", Enchanter},
    {"Soraka", Enchanter}, {"Janna", Enchanter}, {"Lulu", Enchanter}, {"Nami", Enchanter},
    {"Sona", Enchanter}, {"Yuumi", Enchanter}, {"Karma", Enchanter}, {"Milio", Enchanter},
    {"Renata", Enchanter}, {"Seraphine", Enchanter}, {"Bard", Enchanter}, {"Ivern", Enchanter},
    {"Zilean", Enchanter},
};

const std::unordered_map<std::string, std::vector<std::string>> SIGNATURE_ITEMS = {
    {"Jax", {"Trinity Force", "Blade of The Ruined King"}},
    {"Irelia", {"Blade of The Ruined King", "Sundered Sky"}},
    {"Fiora", {"Ravenous Hydra", "Sundered Sky"}},
    {"Camille", {"Trinity Force", "Ravenous Hydra"}},
    {"Riven", {"Profane Hydra", "Sundered Sky", "Black Cleaver"}},
    {"Aatrox", {"Eclipse", "Sundered Sky", "Black Cleaver"}},
    {"Darius", {"Stridebreaker", "Sterak's Gage", "Dead Man's Plate"}},
    {"Garen", {"Stridebreaker", "Phantom Dancer"}},
    {"Sett", {"Stridebreaker", "Sterak's Gage"}},
    {"Ezreal", {"Trinity Force", "Manamune", "Serylda's Grudge"}},
    {"Jayce", {"Manamune", "Axiom Arc"}},
    {"Corki", {"Trinity Force", "Essence Reaver"}},
    {"Kaisa", {"Kraken Slayer", "Nashor's Tooth", "Guinsoo's Rageblade"}},
    {"Varus", {"Blade of The Ruined King", "Guinsoo's Rageblade"}},
    {"Vayne", {"Blade of The Ruined King", "Guinsoo's Rageblade"}},
    {"Kayle", {"Nashor's Tooth", "Guinsoo's Rageblade"}},
    {"Teemo", {"Nashor's Tooth", "Liandry's Torment"}},
    {"Gwen", {"Nashor's Tooth", "Riftmaker"}},
    {"Katarina", {"Stormsurge", "Shadowflame"}},
    {"Ryze", {"Archangel's Staff", "Rod of Ages"}},
    {"Cassiopeia", {"Rod of Ages", "Rylai's Crystal Scepter"}},
    {"Vladimir", {"Riftmaker", "Cosmic Drive", "Rabadon's Deathcap"}},
    {"Swain", {"Liandry's Torment", "Rylai's Crystal Scepter"}},
    {"Sylas", {"Malignance", "Hextech Rocketbelt"}},
    {"Ekko", {"Hextech Rocketbelt", "Lich Bane"}},
    {"Fizz", {"Lich Bane", "Stormsurge"}},
    {"Diana", {"Lich Bane", "Riftmaker"}},
    {"Kassadin", {"Rod of Ages", "Lich Bane"}},
    {"Smolder", {"Essence Reaver", "Infinity Edge"}},
    {"Yasuo", {"Infinity Edge", "Immortal Shieldbow"}},
    {"Yone", {"Infinity Edge", "Blade of The Ruined King"}},
    {"MasterYi", {"Blade of The Ruined King", "Guinsoo's Rageblade", "Wit's End"}},
    {"Shyvana", {"Liandry's Torment", "Riftmaker"}},
    {"Zac", {"Sunfire Aegis", "Heartsteel"}},
    {"Chogath", {"Heartsteel"}},
    {"Sion", {"Heartsteel", "Sunfire Aegis"}},
    {"Malphite", {"Sunfire Aegis", "Iceborn Gauntlet"}},
    {"Nasus", {"Iceborn Gauntlet", "Spirit Visage", "Trinity Force"}},
    {"Seraphine", {"Moonstone Renewer", "Rylai's Crystal Scepter"}},
    {"Janna", {"Shurelya's Battlesong"}},
    {"Lulu", {"Ardent Censer", "Shurelya's Battlesong"}},
};

// Enemy threat traits ------------------------------------------------------

/** Share of damage that is magic (0..1). Unlisted champs use tags/info. */
const std::unordered_map<std::string, double> AP_SHARE = {
    {"Kaisa", 0.5}, {"Kayle", 0.7}, {"Corki", 0.6}, {"KogMaw", 0.45}, {"Varus", 0.4}, {"Shyvana", 0.6},
    {"Ezreal", 0.15}, {"Jax", 0.4}, {"Katarina", 0.85}, {"Gwen", 0.9}, {"Kassadin", 1}, {"Diana", 1},
    {"Ekko", 1}, {"Fizz", 1}, {"Akali", 0.9}, {"Evelynn", 1}, {"Mordekaiser", 1}, {"Sylas", 0.9},
    {"Rumble", 1}, {"Lillia", 1}, {"Elise", 1}, {"Nidalee", 0.9}, {"Gragas", 1}, {"Amumu", 0.9}, {"Zac", 0.9},
    {"Malphite", 0.7}, {"Teemo", 1}, {"Singed", 1}, {"Sejuani", 0.6}, {"Maokai", 0.8}, {"Galio", 0.9},
    {"Chogath", 0.8}, {"Nunu", 0.9}, {"Sion", 0.4}, {"Ornn", 0.5}, {"Rammus", 0.6}, {"Leona", 0.7},
    {"Nautilus", 0.7}, {"Alistar", 0.8}, {"Braum", 0.6}, {"Taric", 0.8}, {"Volibear", 0.5}, {"Udyr", 0.5},
    {"Warwick", 0.4}, {"Skarner", 0.4}, {"Poppy", 0.2}, {"Trundle", 0.1}, {"Twitch", 0.2}, {"Kennen", 1},
    {"Vladimir", 1}, {"Fiddlesticks", 1}, {"Karthus", 1}, {"Brand", 1}, {"Shaco", 0.4}, {"Nasus", 0.2},
    {"Hecarim", 0.1}, {"Kindred", 0.1}, {"Aphelios", 0.1}, {"Akshan", 0.1}, {"Viego", 0.1}, {"Belveth", 0.1},
    {"TahmKench", 0.6}, {"DrMundo", 0.4}, {"KSante", 0.2}, {"Rakan", 0.9}, {"Thresh", 0.6}, {"Pyke", 0},
    {"Blitzcrank", 0.9}, {"Rell", 0.9}, {"Senna", 0.1}, {"Shen", 0.6}, {"Kled", 0}, {"Illaoi", 0}, {"Yorick", 0.1},
};

using TraitTable = std::unordered_map<std::string, double>;

const std::array<TraitTable, kTraitCount>& traitTables() {
  static const std::array<TraitTable, kTraitCount> tables = [] {
    std::array<TraitTable, kTraitCount> t;
    t[idx(Trait::Heal)] = {
        {"Soraka", 1.2}, {"Aatrox", 1}, {"Vladimir", 1}, {"DrMundo", 1}, {"Warwick", 1}, {"Yuumi", 1}, {"Sylas", 0.8},
        {"Swain", 1}, {"Briar", 1}, {"Fiddlesticks", 0.7}, {"Olaf", 0.8}, {"Illaoi", 0.8}, {"Volibear", 0.5},
        {"Nasus", 0.6}, {"Trundle", 0.6}, {"Irelia", 0.5}, {"Fiora", 0.7}, {"Gwen", 0.6}, {"Samira", 0.5},
        {"Seraphine", 0.5}, {"Milio", 0.6}, {"Sona", 0.6}, {"Nami", 0.6}, {"Zac", 0.6}, {"Udyr", 0.5},
        {"Mordekaiser", 0.4}, {"Viego", 0.6}, {"Belveth", 0.6}, {"MasterYi", 0.8}, {"TahmKench", 0.4},
        {"Kayn", 0.6}, {"Renekton", 0.5}, {"Darius", 0.4}, {"Sett", 0.4}, {"Senna", 0.4}, {"Maokai", 0.5},
        {"Ivern", 0.3}, {"Taric", 0.5}, {"Lillia", 0.4}, {"Elise", 0.3}, {"Hecarim", 0.4}, {"Nidalee", 0.5},
        {"XinZhao", 0.4}, {"Ambessa", 0.5}, {"Yorick", 0.4}, {"Zaahen", 0.5},
    };
    t[idx(Trait::Shield)] = {
        {"Lulu", 1}, {"Janna", 1}, {"Karma", 1}, {"Seraphine", 0.6}, {"Lux", 0.6}, {"Sett", 0.8}, {"Mordekaiser", 0.8},
        {"Shen", 0.7}, {"Yasuo", 0.5}, {"Rakan", 0.7}, {"Orianna", 0.8}, {"Ivern", 0.7}, {"Renata", 0.4},
        {"Milio", 1}, {"Taric", 0.8}, {"Braum", 0.5}, {"Riven", 0.5}, {"Skarner", 0.6}, {"Malphite", 0.6},
        {"Sion", 0.5}, {"Urgot", 0.4}, {"Diana", 0.5}, {"Morgana", 0.6}, {"Nautilus", 0.5}, {"Ekko", 0.4},
        {"Vi", 0.4}, {"Camille", 0.3}, {"Kled", 0.3}, {"Rumble", 0.5}, {"Garen", 0.4}, {"Lissandra", 0.2},
        {"KSante", 0.4}, {"Yone", 0.3}, {"Gragas", 0.2}, {"Sona", 0.4}, {"Ambessa", 0.4},
    };
    t[idx(Trait::CC)] = {
        {"Leona", 1.2}, {"Nautilus", 1.2}, {"Amumu", 1}, {"Malphite", 0.8}, {"Sejuani", 1}, {"Lissandra", 1},
        {"Ashe", 0.6}, {"Morgana", 0.8}, {"Thresh", 1}, {"Blitzcrank", 0.9}, {"Rell", 1.2}, {"Alistar", 1},
        {"Annie", 0.7}, {"Veigar", 0.7}, {"Zyra", 0.6}, {"Neeko", 0.8}, {"Maokai", 1}, {"Rammus", 0.9},
        {"Skarner", 1}, {"Malzahar", 0.8}, {"Warwick", 0.6}, {"Urgot", 0.5}, {"TwistedFate", 0.5}, {"Vi", 0.7},
        {"JarvanIV", 0.8}, {"Zac", 0.9}, {"Ornn", 1}, {"Sion", 0.8}, {"Chogath", 0.7}, {"Galio", 0.9}, {"Lux", 0.6},
        {"Rakan", 0.8}, {"MonkeyKing", 0.7}, {"Kennen", 0.8}, {"Nami", 0.6}, {"Braum", 0.7}, {"Poppy", 0.7},
        {"Leblanc", 0.4}, {"Syndra", 0.6}, {"Xerath", 0.4}, {"Taric", 0.6}, {"Renata", 0.6}, {"Mel", 0.4},
        {"Nunu", 0.6}, {"Hecarim", 0.6}, {"LeeSin", 0.4}, {"Pyke", 0.6}, {"Gnar", 0.6}, {"KSante", 0.6},
        {"Ambessa", 0.4}, {"Aurora", 0.4}, {"Hwei", 0.5},
    };
    t[idx(Trait::Suppress)] = {
        {"Malzahar", 1}, {"Skarner", 1}, {"Warwick", 1}, {"Urgot", 1}, {"Mordekaiser", 0.6}, {"Zed", 0.4},
    };
    t[idx(Trait::Burst)] = {
        {"Zed", 1}, {"Talon", 1}, {"Khazix", 1}, {"Rengar", 1}, {"Qiyana", 1}, {"Naafiri", 0.8}, {"Pyke", 0.6},
        {"Kayn", 0.6}, {"Fizz", 1}, {"Ekko", 0.8}, {"Kassadin", 0.8}, {"Akali", 1}, {"Evelynn", 1}, {"Diana", 0.8},
        {"Katarina", 1}, {"Leblanc", 1}, {"Elise", 0.6}, {"Nidalee", 0.7}, {"Syndra", 0.9}, {"Annie", 0.8},
        {"Veigar", 0.8}, {"Lux", 0.6}, {"Ahri", 0.6}, {"Brand", 0.5}, {"Zoe", 0.8}, {"Viktor", 0.4}, {"Hwei", 0.5},
        {"Mel", 0.6}, {"Shaco", 0.6}, {"Camille", 0.5}, {"Pantheon", 0.5}, {"Quinn", 0.5}, {"Vex", 0.7},
        {"Sylas", 0.6}, {"Aurora", 0.6}, {"Draven", 0.5}, {"Samira", 0.6}, {"Gragas", 0.4}, {"Ambessa", 0.6},
    };
    t[idx(Trait::Auto)] = {
        {"MasterYi", 1}, {"Jax", 0.8}, {"Tryndamere", 1}, {"Fiora", 0.7}, {"Yasuo", 0.7}, {"Yone", 0.7},
        {"Belveth", 1}, {"Kayle", 0.8}, {"Shyvana", 0.6}, {"Udyr", 0.6}, {"Trundle", 0.6}, {"Irelia", 0.6},
        {"Viego", 0.6}, {"XinZhao", 0.7}, {"Warwick", 0.6}, {"Nilah", 0.8}, {"Gwen", 0.4}, {"Teemo", 0.4},
        {"Olaf", 0.6},
    };
    t[idx(Trait::Crit)] = {
        {"Yasuo", 1}, {"Yone", 1}, {"Tryndamere", 1}, {"Gangplank", 0.7}, {"Nilah", 0.8}, {"Akshan", 0.6},
    };
    t[idx(Trait::Poke)] = {
        {"Xerath", 1}, {"Ziggs", 1}, {"Jayce", 1}, {"Varus", 0.8}, {"Ezreal", 0.7}, {"Lux", 0.7}, {"Velkoz", 1},
        {"Zoe", 0.8}, {"Nidalee", 0.8}, {"Karma", 0.6}, {"Hwei", 0.8}, {"Mel", 0.6}, {"Caitlyn", 0.5},
        {"Seraphine", 0.5}, {"Heimerdinger", 0.7}, {"Brand", 0.5}, {"Syndra", 0.4}, {"Corki", 0.5},
        {"Kennen", 0.4}, {"Teemo", 0.5}, {"Smolder", 0.4}, {"Senna", 0.5}, {"AurelionSol", 0.5}, {"Taliyah", 0.5},
    };
    t[idx(Trait::Stealth)] = {
        {"Evelynn", 1}, {"Twitch", 1}, {"Akali", 0.8}, {"Shaco", 1}, {"Khazix", 0.8}, {"Rengar", 0.8},
        {"Vayne", 0.5}, {"Pyke", 0.6}, {"Talon", 0.4}, {"MonkeyKing", 0.4}, {"Neeko", 0.3}, {"Senna", 0.3},
    };
    return t;
  }();
  return tables;
}

/** Marksmen that build on-hit rather than crit (don't count as crit threats). */
const std::unordered_set<std::string> NON_CRIT_MARKSMEN = {"Vayne",   "KogMaw",   "Kalista", "Varus",
                                                           "Kaisa",   "MasterYi", "Belveth"};

template <std::size_t N>
std::optional<std::size_t> indexOf(const std::array<std::string_view, N>& keys, std::string_view key) {
  auto it = std::find(keys.begin(), keys.end(), key);
  if (it == keys.end()) return std::nullopt;
  return static_cast<std::size_t>(it - keys.begin());
}

}  // namespace

std::string_view archetypeLabel(Archetype a) { return ARCHETYPE_LABELS[idx(a)]; }
std::string_view archetypeKey(Archetype a) { return ARCHETYPE_KEYS[idx(a)]; }

std::optional<Archetype> archetypeFromKey(std::string_view key) {
  if (auto i = indexOf(ARCHETYPE_KEYS, key)) return kArchetypes[*i];
  return std::nullopt;
}

bool isApArchetype(Archetype a) {
  return a == ApMage || a == ApBattlemage || a == ApAssassin || a == ApOnHit;
}

std::string_view roleLabel(Role r) { return ROLE_LABELS[idx(r)]; }
std::string_view roleKey(Role r) { return ROLE_KEYS[idx(r)]; }

std::optional<Role> roleFromKey(std::string_view key) {
  if (auto i = indexOf(ROLE_KEYS, key)) return kRoles[*i];
  return std::nullopt;
}

const std::vector<std::string>& signatureItems(const std::string& championId) {
  static const std::vector<std::string> none;
  auto it = SIGNATURE_ITEMS.find(championId);
  return it == SIGNATURE_ITEMS.end() ? none : it->second;
}

Archetype archetypeFor(const Champion& c) {
  if (auto it = ARCHETYPE_OVERRIDES.find(c.id); it != ARCHETYPE_OVERRIDES.end()) return it->second;
  std::string primary = c.tags.size() > 0 ? c.tags[0] : "";
  std::string secondary = c.tags.size() > 1 ? c.tags[1] : "";
  bool apLeaning = c.info.magic > c.info.attack;
  if (primary == "Marksman") return AdcCrit;
  if (primary == "Mage") return secondary == "Assassin" ? ApAssassin : ApMage;
  if (primary == "Assassin") return apLeaning ? ApAssassin : AdAssassin;
  if (primary == "Fighter") return apLeaning ? ApBattlemage : Bruiser;
  if (primary == "Tank") return Tank;
  if (primary == "Support") return secondary == "Tank" ? Tank : apLeaning ? Enchanter : Tank;
  return apLeaning ? ApMage : Bruiser;
}

Role defaultRoleFor(const Champion& c) {
  Archetype arch = archetypeFor(c);
  if (arch == Enchanter || (!c.tags.empty() && c.tags[0] == "Support")) return Role::Support;
  if (arch == AdcCrit || arch == AdcOnHit) return c.attackRange > 300 ? Role::Bot : Role::Top;
  if (arch == ApMage || arch == ApAssassin) return Role::Mid;
  if (arch == AdAssassin) return Role::Mid;
  return Role::Top;
}

ThreatProfile threatProfile(const Champion& c) {
  Archetype arch = archetypeFor(c);
  std::string primary = c.tags.empty() ? "" : c.tags[0];
  double apShare;
  if (auto it = AP_SHARE.find(c.id); it != AP_SHARE.end()) apShare = it->second;
  else if (primary == "Mage") apShare = 0.95;
  else if (primary == "Marksman") apShare = 0.1;
  else apShare = c.info.magic / static_cast<double>(std::max(1, c.info.attack + c.info.magic));

  ThreatProfile p{
      .champion = &c,
      .apShare = apShare,
      .damageWeight = arch == Enchanter ? 0.45 : arch == Tank ? 0.6 : 1,
      .isTank = arch == Tank || c.hasTag("Tank") || c.info.defense >= 7,
      .isRanged = c.attackRange > 300,
  };
  const auto& tables = traitTables();
  for (std::size_t t = 0; t < kTraitCount; t++) {
    auto it = tables[t].find(c.id);
    p.traits[t] = it == tables[t].end() ? 0 : it->second;
  }
  if (arch == AdcCrit || arch == AdcOnHit) {
    p.traits[idx(Trait::Auto)] = std::max(p.trait(Trait::Auto), 1.0);
    if (!NON_CRIT_MARKSMEN.contains(c.id)) p.traits[idx(Trait::Crit)] = std::max(p.trait(Trait::Crit), 1.0);
  }
  if (c.hasTag("Assassin")) p.traits[idx(Trait::Burst)] = std::max(p.trait(Trait::Burst), 0.7);
  return p;
}

// Hand-curated champion knowledge layered on top of Data Dragon's tags/info.
// Data Dragon doesn't say *how* a champion deals damage or what threats it poses,
// so these tables encode that. Champions not listed fall back to heuristics.

import type { Champion } from "./ddragon";

export type Archetype =
  | "adc_crit"
  | "adc_onhit"
  | "ad_caster"
  | "ad_assassin"
  | "bruiser"
  | "tank"
  | "ap_mage"
  | "ap_battlemage"
  | "ap_assassin"
  | "ap_onhit"
  | "enchanter";

export const ARCHETYPE_LABELS: Record<Archetype, string> = {
  adc_crit: "Crit marksman",
  adc_onhit: "On-hit attacker",
  ad_caster: "AD caster",
  ad_assassin: "Lethality assassin",
  bruiser: "Bruiser / fighter",
  tank: "Tank",
  ap_mage: "Burst / control mage",
  ap_battlemage: "Battlemage (sustained AP)",
  ap_assassin: "AP assassin",
  ap_onhit: "AP on-hit",
  enchanter: "Enchanter support",
};

export type Role = "top" | "jungle" | "mid" | "bot" | "support";

export const ROLE_LABELS: Record<Role, string> = {
  top: "Top",
  jungle: "Jungle",
  mid: "Mid",
  bot: "Bot (ADC)",
  support: "Support",
};

const ARCHETYPE_OVERRIDES: Record<string, Archetype> = {
  // On-hit / attack-speed scalers
  Vayne: "adc_onhit", KogMaw: "adc_onhit", Kalista: "adc_onhit", Varus: "adc_onhit",
  Kaisa: "adc_onhit", MasterYi: "adc_onhit", Belveth: "adc_onhit", Kindred: "adc_crit",
  // Crit melee
  Yasuo: "adc_crit", Yone: "adc_crit", Tryndamere: "adc_crit", Nilah: "adc_crit",
  Gangplank: "adc_crit",
  // AD casters
  Ezreal: "ad_caster", Jayce: "ad_caster", Corki: "ad_caster", Senna: "ad_caster",
  Pantheon: "ad_caster", Aphelios: "adc_crit",
  // Lethality
  Zed: "ad_assassin", Talon: "ad_assassin", Khazix: "ad_assassin", Rengar: "ad_assassin",
  Qiyana: "ad_assassin", Naafiri: "ad_assassin", Pyke: "ad_assassin", Kayn: "ad_assassin",
  Shaco: "ad_assassin", Quinn: "ad_assassin", Akshan: "adc_crit",
  // AP assassins
  Fizz: "ap_assassin", Ekko: "ap_assassin", Kassadin: "ap_assassin", Akali: "ap_assassin",
  Evelynn: "ap_assassin", Diana: "ap_assassin", Katarina: "ap_assassin", Leblanc: "ap_assassin",
  Elise: "ap_assassin", Nidalee: "ap_assassin",
  // Battlemages / sustained AP
  Mordekaiser: "ap_battlemage", Rumble: "ap_battlemage", Lillia: "ap_battlemage",
  Brand: "ap_battlemage", Cassiopeia: "ap_battlemage", Ryze: "ap_battlemage",
  Swain: "ap_battlemage", Vladimir: "ap_battlemage", Sylas: "ap_battlemage",
  Karthus: "ap_battlemage", Fiddlesticks: "ap_battlemage", Singed: "ap_battlemage",
  Aurora: "ap_mage",
  // AP on-hit
  Kayle: "ap_onhit", Gwen: "ap_onhit", Teemo: "ap_onhit",
  // Tanks that DDragon tags as fighters (or vice versa)
  Sejuani: "tank", Zac: "tank", Sion: "tank", Ornn: "tank", Chogath: "tank",
  Malphite: "tank", KSante: "tank", Poppy: "tank", Rammus: "tank", Amumu: "tank",
  DrMundo: "tank", TahmKench: "tank", Maokai: "tank", Nunu: "tank", Skarner: "tank",
  Volibear: "bruiser", Shen: "tank", Galio: "tank",
  // Supports
  Leona: "tank", Nautilus: "tank", Thresh: "tank", Alistar: "tank", Braum: "tank",
  Rell: "tank", Blitzcrank: "tank", Taric: "tank", Rakan: "enchanter",
  Soraka: "enchanter", Janna: "enchanter", Lulu: "enchanter", Nami: "enchanter",
  Sona: "enchanter", Yuumi: "enchanter", Karma: "enchanter", Milio: "enchanter",
  Renata: "enchanter", Seraphine: "enchanter", Bard: "enchanter", Ivern: "enchanter",
  Zilean: "enchanter",
};

/** Item names that are signature picks for a champion; they get a fit bonus. */
export const SIGNATURE_ITEMS: Record<string, string[]> = {
  Jax: ["Trinity Force", "Blade of The Ruined King"],
  Irelia: ["Blade of The Ruined King", "Sundered Sky"],
  Fiora: ["Ravenous Hydra", "Sundered Sky"],
  Camille: ["Trinity Force", "Ravenous Hydra"],
  Riven: ["Profane Hydra", "Sundered Sky", "Black Cleaver"],
  Aatrox: ["Eclipse", "Sundered Sky", "Black Cleaver"],
  Darius: ["Stridebreaker", "Sterak's Gage", "Dead Man's Plate"],
  Garen: ["Stridebreaker", "Phantom Dancer"],
  Sett: ["Stridebreaker", "Sterak's Gage"],
  Ezreal: ["Trinity Force", "Manamune", "Serylda's Grudge"],
  Jayce: ["Manamune", "Axiom Arc"],
  Corki: ["Trinity Force", "Essence Reaver"],
  Kaisa: ["Kraken Slayer", "Nashor's Tooth", "Guinsoo's Rageblade"],
  Varus: ["Blade of The Ruined King", "Guinsoo's Rageblade"],
  Vayne: ["Blade of The Ruined King", "Guinsoo's Rageblade"],
  Kayle: ["Nashor's Tooth", "Guinsoo's Rageblade"],
  Teemo: ["Nashor's Tooth", "Liandry's Torment"],
  Gwen: ["Nashor's Tooth", "Riftmaker"],
  Katarina: ["Stormsurge", "Shadowflame"],
  Ryze: ["Archangel's Staff", "Rod of Ages"],
  Cassiopeia: ["Rod of Ages", "Rylai's Crystal Scepter"],
  Vladimir: ["Riftmaker", "Cosmic Drive", "Rabadon's Deathcap"],
  Swain: ["Liandry's Torment", "Rylai's Crystal Scepter"],
  Sylas: ["Malignance", "Hextech Rocketbelt"],
  Ekko: ["Hextech Rocketbelt", "Lich Bane"],
  Fizz: ["Lich Bane", "Stormsurge"],
  Diana: ["Lich Bane", "Riftmaker"],
  Kassadin: ["Rod of Ages", "Lich Bane"],
  Smolder: ["Essence Reaver", "Infinity Edge"],
  Yasuo: ["Infinity Edge", "Immortal Shieldbow"],
  Yone: ["Infinity Edge", "Blade of The Ruined King"],
  MasterYi: ["Blade of The Ruined King", "Guinsoo's Rageblade", "Wit's End"],
  Shyvana: ["Liandry's Torment", "Riftmaker"],
  Zac: ["Sunfire Aegis", "Heartsteel"],
  Chogath: ["Heartsteel"],
  Sion: ["Heartsteel", "Sunfire Aegis"],
  Malphite: ["Sunfire Aegis", "Iceborn Gauntlet"],
  Nasus: ["Iceborn Gauntlet", "Spirit Visage", "Trinity Force"],
  Seraphine: ["Moonstone Renewer", "Rylai's Crystal Scepter"],
  Janna: ["Shurelya's Battlesong"],
  Lulu: ["Ardent Censer", "Shurelya's Battlesong"],
};

// Enemy threat traits ------------------------------------------------------

/** Share of damage that is magic (0..1). Unlisted champs use tags/info. */
const AP_SHARE: Record<string, number> = {
  Kaisa: 0.5, Kayle: 0.7, Corki: 0.6, KogMaw: 0.45, Varus: 0.4, Shyvana: 0.6,
  Ezreal: 0.15, Jax: 0.4, Katarina: 0.85, Gwen: 0.9, Kassadin: 1, Diana: 1,
  Ekko: 1, Fizz: 1, Akali: 0.9, Evelynn: 1, Mordekaiser: 1, Sylas: 0.9,
  Rumble: 1, Lillia: 1, Elise: 1, Nidalee: 0.9, Gragas: 1, Amumu: 0.9, Zac: 0.9,
  Malphite: 0.7, Teemo: 1, Singed: 1, Sejuani: 0.6, Maokai: 0.8, Galio: 0.9,
  Chogath: 0.8, Nunu: 0.9, Sion: 0.4, Ornn: 0.5, Rammus: 0.6, Leona: 0.7,
  Nautilus: 0.7, Alistar: 0.8, Braum: 0.6, Taric: 0.8, Volibear: 0.5, Udyr: 0.5,
  Warwick: 0.4, Skarner: 0.4, Poppy: 0.2, Trundle: 0.1, Twitch: 0.2, Kennen: 1,
  Vladimir: 1, Fiddlesticks: 1, Karthus: 1, Brand: 1, Shaco: 0.4, Nasus: 0.2,
  Hecarim: 0.1, Kindred: 0.1, Aphelios: 0.1, Akshan: 0.1, Viego: 0.1, Belveth: 0.1,
  TahmKench: 0.6, DrMundo: 0.4, KSante: 0.2, Rakan: 0.9, Thresh: 0.6, Pyke: 0,
  Blitzcrank: 0.9, Rell: 0.9, Senna: 0.1, Shen: 0.6, Kled: 0, Illaoi: 0, Yorick: 0.1,
};

/** Weighted threat traits. 1 = defining trait, 0.5 = notable. */
type Trait = "heal" | "shield" | "cc" | "suppress" | "burst" | "auto" | "crit" | "poke" | "stealth";

const TRAITS: Record<Trait, Record<string, number>> = {
  heal: {
    Soraka: 1.2, Aatrox: 1, Vladimir: 1, DrMundo: 1, Warwick: 1, Yuumi: 1, Sylas: 0.8,
    Swain: 1, Briar: 1, Fiddlesticks: 0.7, Olaf: 0.8, Illaoi: 0.8, Volibear: 0.5,
    Nasus: 0.6, Trundle: 0.6, Irelia: 0.5, Fiora: 0.7, Gwen: 0.6, Samira: 0.5,
    Seraphine: 0.5, Milio: 0.6, Sona: 0.6, Nami: 0.6, Zac: 0.6, Udyr: 0.5,
    Mordekaiser: 0.4, Viego: 0.6, Belveth: 0.6, MasterYi: 0.8, TahmKench: 0.4,
    Kayn: 0.6, Renekton: 0.5, Darius: 0.4, Sett: 0.4, Senna: 0.4, Maokai: 0.5,
    Ivern: 0.3, Taric: 0.5, Lillia: 0.4, Elise: 0.3, Hecarim: 0.4, Nidalee: 0.5,
    XinZhao: 0.4, Ambessa: 0.5, Yorick: 0.4, Zaahen: 0.5,
  },
  shield: {
    Lulu: 1, Janna: 1, Karma: 1, Seraphine: 0.6, Lux: 0.6, Sett: 0.8, Mordekaiser: 0.8,
    Shen: 0.7, Yasuo: 0.5, Rakan: 0.7, Orianna: 0.8, Ivern: 0.7, Renata: 0.4,
    Milio: 1, Taric: 0.8, Braum: 0.5, Riven: 0.5, Skarner: 0.6, Malphite: 0.6,
    Sion: 0.5, Urgot: 0.4, Diana: 0.5, Morgana: 0.6, Nautilus: 0.5, Ekko: 0.4,
    Vi: 0.4, Camille: 0.3, Kled: 0.3, Rumble: 0.5, Garen: 0.4, Lissandra: 0.2,
    KSante: 0.4, Yone: 0.3, Gragas: 0.2, Sona: 0.4, Ambessa: 0.4,
  },
  cc: {
    Leona: 1.2, Nautilus: 1.2, Amumu: 1, Malphite: 0.8, Sejuani: 1, Lissandra: 1,
    Ashe: 0.6, Morgana: 0.8, Thresh: 1, Blitzcrank: 0.9, Rell: 1.2, Alistar: 1,
    Annie: 0.7, Veigar: 0.7, Zyra: 0.6, Neeko: 0.8, Maokai: 1, Rammus: 0.9,
    Skarner: 1, Malzahar: 0.8, Warwick: 0.6, Urgot: 0.5, TwistedFate: 0.5, Vi: 0.7,
    JarvanIV: 0.8, Zac: 0.9, Ornn: 1, Sion: 0.8, Chogath: 0.7, Galio: 0.9, Lux: 0.6,
    Rakan: 0.8, MonkeyKing: 0.7, Kennen: 0.8, Nami: 0.6, Braum: 0.7, Poppy: 0.7,
    Leblanc: 0.4, Syndra: 0.6, Xerath: 0.4, Taric: 0.6, Renata: 0.6, Mel: 0.4,
    Nunu: 0.6, Hecarim: 0.6, LeeSin: 0.4, Pyke: 0.6, Gnar: 0.6, KSante: 0.6,
    Ambessa: 0.4, Aurora: 0.4, Hwei: 0.5,
  },
  suppress: { Malzahar: 1, Skarner: 1, Warwick: 1, Urgot: 1, Mordekaiser: 0.6, Zed: 0.4 },
  burst: {
    Zed: 1, Talon: 1, Khazix: 1, Rengar: 1, Qiyana: 1, Naafiri: 0.8, Pyke: 0.6, Kayn: 0.6,
    Fizz: 1, Ekko: 0.8, Kassadin: 0.8, Akali: 1, Evelynn: 1, Diana: 0.8, Katarina: 1,
    Leblanc: 1, Elise: 0.6, Nidalee: 0.7, Syndra: 0.9, Annie: 0.8, Veigar: 0.8,
    Lux: 0.6, Ahri: 0.6, Brand: 0.5, Zoe: 0.8, Viktor: 0.4, Hwei: 0.5, Mel: 0.6,
    Shaco: 0.6, Camille: 0.5, Pantheon: 0.5, Quinn: 0.5, Vex: 0.7,
    Sylas: 0.6, Aurora: 0.6, Draven: 0.5, Samira: 0.6, Gragas: 0.4, Ambessa: 0.6,
  },
  auto: {
    MasterYi: 1, Jax: 0.8, Tryndamere: 1, Fiora: 0.7, Yasuo: 0.7, Yone: 0.7, Belveth: 1,
    Kayle: 0.8, Shyvana: 0.6, Udyr: 0.6, Trundle: 0.6, Irelia: 0.6, Viego: 0.6,
    XinZhao: 0.7, Warwick: 0.6, Nilah: 0.8, Gwen: 0.4, Teemo: 0.4, Olaf: 0.6,
  },
  crit: { Yasuo: 1, Yone: 1, Tryndamere: 1, Gangplank: 0.7, Nilah: 0.8, Akshan: 0.6 },
  poke: {
    Xerath: 1, Ziggs: 1, Jayce: 1, Varus: 0.8, Ezreal: 0.7, Lux: 0.7, Velkoz: 1, Zoe: 0.8,
    Nidalee: 0.8, Karma: 0.6, Hwei: 0.8, Mel: 0.6, Caitlyn: 0.5, Seraphine: 0.5,
    Heimerdinger: 0.7, Brand: 0.5, Syndra: 0.4, Corki: 0.5, Kennen: 0.4, Teemo: 0.5,
    Smolder: 0.4, Senna: 0.5, AurelionSol: 0.5, Taliyah: 0.5,
  },
  stealth: {
    Evelynn: 1, Twitch: 1, Akali: 0.8, Shaco: 1, Khazix: 0.8, Rengar: 0.8, Vayne: 0.5,
    Pyke: 0.6, Talon: 0.4, MonkeyKing: 0.4, Neeko: 0.3, Senna: 0.3,
  },
};

/** Marksmen that build on-hit rather than crit (don't count as crit threats). */
const NON_CRIT_MARKSMEN = new Set(["Vayne", "KogMaw", "Kalista", "Varus", "Kaisa", "MasterYi", "Belveth"]);

export function archetypeFor(c: Champion): Archetype {
  const override = ARCHETYPE_OVERRIDES[c.id];
  if (override) return override;
  const [primary, secondary] = c.tags;
  const apLeaning = c.info.magic > c.info.attack;
  switch (primary) {
    case "Marksman":
      return "adc_crit";
    case "Mage":
      return secondary === "Assassin" ? "ap_assassin" : "ap_mage";
    case "Assassin":
      return apLeaning ? "ap_assassin" : "ad_assassin";
    case "Fighter":
      return apLeaning ? "ap_battlemage" : "bruiser";
    case "Tank":
      return "tank";
    case "Support":
      return secondary === "Tank" ? "tank" : apLeaning ? "enchanter" : "tank";
    default:
      return apLeaning ? "ap_mage" : "bruiser";
  }
}

export function defaultRoleFor(c: Champion): Role {
  const arch = archetypeFor(c);
  if (arch === "enchanter" || c.tags[0] === "Support") return "support";
  if (arch === "adc_crit" || arch === "adc_onhit") return c.attackRange > 300 ? "bot" : "top";
  if (arch === "ap_mage" || arch === "ap_assassin") return "mid";
  if (arch === "ad_assassin") return "mid";
  return "top";
}

export interface ThreatProfile {
  champion: Champion;
  apShare: number;
  /** How much of the enemy team's damage this champion represents. */
  damageWeight: number;
  isTank: boolean;
  isRanged: boolean;
  traits: Record<Trait, number>;
}

export function threatProfile(c: Champion): ThreatProfile {
  const arch = archetypeFor(c);
  let apShare = AP_SHARE[c.id];
  if (apShare === undefined) {
    if (c.tags[0] === "Mage") apShare = 0.95;
    else if (c.tags[0] === "Marksman") apShare = 0.1;
    else apShare = c.info.magic / Math.max(1, c.info.attack + c.info.magic);
  }

  const isTank = arch === "tank" || c.tags.includes("Tank") || c.info.defense >= 7;
  const isEnchanter = arch === "enchanter";
  const damageWeight = isEnchanter ? 0.45 : arch === "tank" ? 0.6 : 1;

  const traits = {} as Record<Trait, number>;
  for (const t of Object.keys(TRAITS) as Trait[]) traits[t] = TRAITS[t][c.id] ?? 0;
  if (arch === "adc_crit" || arch === "adc_onhit") {
    traits.auto = Math.max(traits.auto, 1);
    if (!NON_CRIT_MARKSMEN.has(c.id)) traits.crit = Math.max(traits.crit, 1);
  }
  if (c.tags.includes("Assassin")) traits.burst = Math.max(traits.burst, 0.7);

  return { champion: c, apShare, damageWeight, isTank, isRanged: c.attackRange > 300, traits };
}

export type { Trait };

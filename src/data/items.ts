// What each completed item is good for. `fit` is how naturally an item belongs
// in a build style (0-10); `counters` says which enemy needs it answers (0-1).
// Names must match Data Dragon exactly; unknown names are ignored at runtime.

import type { Archetype } from "./champions";

export type Need =
  | "armor"
  | "mr"
  | "antiheal"
  | "antishield"
  | "armorpen"
  | "magicpen"
  | "antiburst"
  | "antiCC"
  | "antiAuto"
  | "antiCrit"
  | "antiPoke";

export interface ItemKnowledge {
  fit: Partial<Record<Archetype, number>>;
  counters?: Partial<Record<Need, number>>;
  /** Items sharing a group have a unique passive — only one per build. */
  group?: string;
  /** Team-utility items that only make sense in the support role. */
  supportOnly?: boolean;
}

const ADC = ["adc_crit", "adc_onhit"] as const;

function many(archetypes: readonly Archetype[], score: number): Partial<Record<Archetype, number>> {
  return Object.fromEntries(archetypes.map((a) => [a, score]));
}

export const ITEM_KNOWLEDGE: Record<string, ItemKnowledge> = {
  // --- Marksman / crit ---
  "Infinity Edge": { fit: { adc_crit: 10 } },
  "Yun Tal Wildarrows": { fit: { adc_crit: 9 } },
  "The Collector": { fit: { adc_crit: 8, ad_assassin: 3 } },
  "Navori Flickerblade": { fit: { adc_crit: 7 } },
  "Phantom Dancer": { fit: { adc_crit: 7, adc_onhit: 4 } },
  "Rapid Firecannon": { fit: { adc_crit: 6 } },
  "Runaan's Hurricane": { fit: { adc_crit: 6, adc_onhit: 6 } },
  "Fiendhunter Bolts": { fit: { adc_crit: 6 } },
  "Hexoptics C44": { fit: { adc_crit: 6 } },
  "Stormrazor": { fit: { adc_crit: 6 } },
  "Essence Reaver": { fit: { adc_crit: 5, ad_caster: 7 }, group: "spellblade" },
  "Lord Dominik's Regards": { fit: { adc_crit: 7, ad_caster: 4 }, counters: { armorpen: 1 }, group: "lastwhisper" },
  "Mortal Reminder": { fit: { adc_crit: 5, adc_onhit: 3 }, counters: { antiheal: 1, armorpen: 0.6 }, group: "lastwhisper" },
  "Bloodthirster": { fit: { adc_crit: 7, adc_onhit: 4 }, counters: { antiburst: 0.3, antiPoke: 0.3 } },
  "Immortal Shieldbow": { fit: { adc_crit: 5 }, counters: { antiburst: 0.9 }, group: "lifeline" },
  "Guardian Angel": { fit: { ...many(ADC, 4), ad_caster: 4, bruiser: 4, ad_assassin: 3 }, counters: { antiburst: 0.8, armor: 0.4 } },
  "Mercurial Scimitar": { fit: { ...many(ADC, 4) }, counters: { antiCC: 1, mr: 0.6 }, group: "quicksilver" },
  "Kraken Slayer": { fit: { adc_crit: 7, adc_onhit: 9 }, counters: { armorpen: 0.4 } },
  "Blade of The Ruined King": { fit: { adc_crit: 5, adc_onhit: 10, bruiser: 4 }, counters: { armorpen: 0.8 } },
  "Guinsoo's Rageblade": { fit: { adc_onhit: 9, ap_onhit: 8 } },
  "Wit's End": { fit: { adc_onhit: 7, bruiser: 3, ap_onhit: 4 }, counters: { mr: 0.9, antiCC: 0.2 } },
  "Terminus": { fit: { adc_onhit: 8 }, counters: { armorpen: 0.6, magicpen: 0.3 } },
  "Statikk Shiv": { fit: { adc_onhit: 4, adc_crit: 3 } },

  // --- AD casters / lethality ---
  "Manamune": { fit: { ad_caster: 10 }, group: "manaflow" },
  "Trinity Force": { fit: { ad_caster: 9, bruiser: 8 }, group: "spellblade" },
  "Serylda's Grudge": { fit: { ad_caster: 8, ad_assassin: 7, bruiser: 4 }, counters: { armorpen: 0.7 } },
  "Black Cleaver": { fit: { ad_caster: 6, bruiser: 9 }, counters: { armorpen: 0.7 } },
  "Spear of Shojin": { fit: { ad_caster: 6, bruiser: 7 } },
  "Eclipse": { fit: { ad_caster: 7, ad_assassin: 7, bruiser: 7 }, counters: { armorpen: 0.3, antiburst: 0.3 } },
  "Axiom Arc": { fit: { ad_caster: 6, ad_assassin: 7 } },
  "Youmuu's Ghostblade": { fit: { ad_caster: 5, ad_assassin: 8 } },
  "Hubris": { fit: { ad_assassin: 8 } },
  "Profane Hydra": { fit: { ad_assassin: 9, bruiser: 4 }, group: "hydra" },
  "Voltaic Cyclosword": { fit: { ad_assassin: 8 } },
  "Umbral Glaive": { fit: { ad_assassin: 6 } },
  "Bastionbreaker": { fit: { ad_assassin: 7 } },
  "Edge of Night": { fit: { ad_assassin: 7, ad_caster: 5, bruiser: 4 }, counters: { antiCC: 0.6, antiburst: 0.5 } },
  "Serpent's Fang": { fit: { ad_assassin: 6 }, counters: { antishield: 1 } },
  "Death's Dance": { fit: { ad_caster: 5, ad_assassin: 5, bruiser: 8 }, counters: { armor: 0.7, antiburst: 0.6 } },
  "Maw of Malmortius": { fit: { ad_caster: 5, ad_assassin: 5, bruiser: 6 }, counters: { mr: 0.9, antiburst: 0.6 }, group: "lifeline" },
  "Chempunk Chainsword": { fit: { ad_assassin: 5, bruiser: 6, ad_caster: 3 }, counters: { antiheal: 1 } },

  // --- Bruisers ---
  "Sterak's Gage": { fit: { bruiser: 9 }, counters: { antiburst: 0.7, antiCC: 0.3 }, group: "lifeline" },
  "Sundered Sky": { fit: { bruiser: 8 } },
  "Stridebreaker": { fit: { bruiser: 7 } },
  "Hullbreaker": { fit: { bruiser: 6 } },
  "Ravenous Hydra": { fit: { bruiser: 7 }, group: "hydra" },
  "Titanic Hydra": { fit: { bruiser: 6, tank: 4 }, group: "hydra" },
  "Overlord's Bloodmail": { fit: { bruiser: 7 } },
  "Experimental Hexplate": { fit: { bruiser: 6 } },
  "Endless Hunger": { fit: { bruiser: 6, ad_assassin: 4 }, counters: { antiCC: 0.4 } },
  "Protoplasm Harness": { fit: { bruiser: 5, tank: 6 }, counters: { antiburst: 0.6, antiCC: 0.3 }, group: "lifeline" },

  // --- Tank ---
  "Sunfire Aegis": { fit: { tank: 8, bruiser: 3 }, counters: { armor: 0.8 } },
  "Hollow Radiance": { fit: { tank: 7, bruiser: 2 }, counters: { mr: 0.8 } },
  "Heartsteel": { fit: { tank: 8, bruiser: 3 } },
  "Jak'Sho, The Protean": { fit: { tank: 9, bruiser: 5 }, counters: { armor: 0.7, mr: 0.7 } },
  "Unending Despair": { fit: { tank: 8 }, counters: { armor: 0.8 } },
  "Thornmail": { fit: { tank: 6, bruiser: 4 }, counters: { armor: 1, antiheal: 1, antiAuto: 0.6 } },
  "Randuin's Omen": { fit: { tank: 6, bruiser: 4 }, counters: { armor: 1, antiCrit: 1, antiAuto: 0.5 } },
  "Frozen Heart": { fit: { tank: 6, bruiser: 3 }, counters: { armor: 1, antiAuto: 1 } },
  "Dead Man's Plate": { fit: { tank: 6, bruiser: 5 }, counters: { armor: 0.8 } },
  "Iceborn Gauntlet": { fit: { tank: 6, bruiser: 4 }, counters: { armor: 0.8 }, group: "spellblade" },
  "Warmog's Armor": { fit: { tank: 6 }, counters: { antiPoke: 0.6 } },
  "Force of Nature": { fit: { tank: 6, bruiser: 4 }, counters: { mr: 1 } },
  "Spirit Visage": { fit: { tank: 6, bruiser: 5 }, counters: { mr: 0.9 } },
  "Kaenic Rookern": { fit: { tank: 7, bruiser: 5 }, counters: { mr: 1, antiPoke: 1 } },
  "Abyssal Mask": { fit: { tank: 5 }, counters: { mr: 0.8 } },
  "Locket of the Iron Solari": { fit: { tank: 6, enchanter: 5 }, counters: { antiburst: 0.6, mr: 0.4, armor: 0.3 }, supportOnly: true },
  "Knight's Vow": { fit: { tank: 5 }, counters: { armor: 0.5 }, supportOnly: true },
  "Zeke's Convergence": { fit: { tank: 4 }, counters: { armor: 0.4, mr: 0.4 }, supportOnly: true },
  "Bandlepipes": { fit: { tank: 4, enchanter: 4 }, counters: { armor: 0.3, mr: 0.3 }, supportOnly: true },

  // --- Mages ---
  "Luden's Echo": { fit: { ap_mage: 9, ap_assassin: 6 } },
  "Rabadon's Deathcap": { fit: { ap_mage: 9, ap_battlemage: 7, ap_assassin: 7, ap_onhit: 8 } },
  "Shadowflame": { fit: { ap_mage: 8, ap_assassin: 8, ap_onhit: 5 }, counters: { magicpen: 0.4 } },
  "Void Staff": { fit: { ap_mage: 7, ap_battlemage: 6, ap_assassin: 6, ap_onhit: 6 }, counters: { magicpen: 1 }, group: "blight" },
  "Cryptbloom": { fit: { ap_mage: 6, ap_assassin: 5 }, counters: { magicpen: 0.7 }, group: "blight" },
  "Bloodletter's Curse": { fit: { ap_mage: 5, ap_battlemage: 7 }, counters: { magicpen: 0.7 }, group: "blight" },
  "Zhonya's Hourglass": { fit: { ap_mage: 7, ap_battlemage: 7, ap_assassin: 7, ap_onhit: 6 }, counters: { antiburst: 1, armor: 0.7 } },
  "Banshee's Veil": { fit: { ap_mage: 6, ap_battlemage: 5, ap_assassin: 6, ap_onhit: 5 }, counters: { mr: 0.8, antiburst: 0.6, antiCC: 0.5 } },
  "Malignance": { fit: { ap_mage: 8, ap_battlemage: 5 } },
  "Horizon Focus": { fit: { ap_mage: 7 } },
  "Blackfire Torch": { fit: { ap_mage: 8, ap_battlemage: 7 } },
  "Actualizer": { fit: { ap_mage: 7 } },
  "Archangel's Staff": { fit: { ap_mage: 7, ap_battlemage: 5 }, group: "manaflow" },
  "Liandry's Torment": { fit: { ap_mage: 6, ap_battlemage: 9, ap_onhit: 5 }, counters: { magicpen: 0.6 } },
  "Rylai's Crystal Scepter": { fit: { ap_mage: 5, ap_battlemage: 7 } },
  "Riftmaker": { fit: { ap_battlemage: 9, ap_onhit: 6, ap_assassin: 4 } },
  "Rod of Ages": { fit: { ap_battlemage: 7, ap_mage: 4 } },
  "Cosmic Drive": { fit: { ap_mage: 6, ap_battlemage: 6, ap_assassin: 6 } },
  "Stormsurge": { fit: { ap_mage: 7, ap_assassin: 8 } },
  "Hextech Rocketbelt": { fit: { ap_assassin: 8, ap_battlemage: 5 } },
  "Lich Bane": { fit: { ap_assassin: 8, ap_mage: 4, ap_onhit: 6 }, group: "spellblade" },
  "Nashor's Tooth": { fit: { ap_onhit: 10 } },
  "Dusk and Dawn": { fit: { ap_onhit: 7 }, group: "spellblade" },
  "Morellonomicon": { fit: { ap_mage: 5, ap_battlemage: 5, ap_assassin: 4 }, counters: { antiheal: 1 } },
  "Hextech Gunblade": { fit: { ap_assassin: 4 } },

  // --- Enchanters ---
  "Moonstone Renewer": { fit: { enchanter: 8 }, supportOnly: true },
  "Echoes of Helia": { fit: { enchanter: 7 }, supportOnly: true },
  "Dawncore": { fit: { enchanter: 7 }, supportOnly: true },
  "Staff of Flowing Water": { fit: { enchanter: 7 }, supportOnly: true },
  "Ardent Censer": { fit: { enchanter: 7 }, supportOnly: true },
  "Shurelya's Battlesong": { fit: { enchanter: 6 }, supportOnly: true },
  "Redemption": { fit: { enchanter: 7 }, counters: { antiPoke: 0.4 }, supportOnly: true },
  "Mikael's Blessing": { fit: { enchanter: 6 }, counters: { antiCC: 0.8 }, supportOnly: true },
  "Imperial Mandate": { fit: { enchanter: 6 }, supportOnly: true },
};


export interface BootsKnowledge {
  fit: Partial<Record<Archetype, number>>;
  counters?: Partial<Record<Need, number>>;
}

export const BOOTS_KNOWLEDGE: Record<string, BootsKnowledge> = {
  "Plated Steelcaps": { fit: { tank: 5, bruiser: 5 }, counters: { armor: 1, antiAuto: 0.8 } },
  "Mercury's Treads": { fit: { tank: 5, bruiser: 5 }, counters: { mr: 1, antiCC: 1 } },
  "Berserker's Greaves": { fit: { adc_crit: 9, adc_onhit: 9, ap_onhit: 6 } },
  "Sorcerer's Shoes": { fit: { ap_mage: 9, ap_battlemage: 8, ap_assassin: 9, ap_onhit: 6 } },
  "Ionian Boots of Lucidity": { fit: { ad_caster: 8, enchanter: 6, ad_assassin: 6, bruiser: 4, ap_mage: 4 } },
  "Boots of Swiftness": { fit: { enchanter: 5, tank: 4 } },
  "Gluttonous Greaves": { fit: { ad_assassin: 5, bruiser: 4 } },
};

/** At most one Grievous Wounds item is ever worth it. */
export const ANTIHEAL_ITEMS = new Set(["Mortal Reminder", "Chempunk Chainsword", "Morellonomicon", "Thornmail"]);

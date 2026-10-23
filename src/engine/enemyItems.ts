// Reads what an enemy has actually bought and folds it into their threat profile.
// A champion's kit says what they *usually* do; their items say what they're doing *this game*.

import type { Item } from "../data/ddragon";
import type { ThreatProfile } from "../data/champions";

const HEALING_ITEMS = new Set([
  "Bloodthirster", "Ravenous Hydra", "Riftmaker", "Sundered Sky", "Spirit Visage", "Warmog's Armor",
  "Moonstone Renewer", "Echoes of Helia", "Redemption", "Blade of The Ruined King", "Death's Dance",
  "Overlord's Bloodmail", "Unending Despair", "Endless Hunger", "Hextech Gunblade", "Maw of Malmortius",
  "Dusk and Dawn", "Mercurial Scimitar", "Rod of Ages", "Protoplasm Harness", "Heartsteel", "Kaenic Rookern",
]);

const SHIELD_ITEMS = new Set([
  "Immortal Shieldbow", "Sterak's Gage", "Maw of Malmortius", "Locket of the Iron Solari", "Eclipse",
  "Archangel's Staff", "Seraph's Embrace", "Moonstone Renewer", "Kaenic Rookern", "Ardent Censer",
  "Echoes of Helia", "Hextech Rocketbelt",
]);

const GRIEVOUS_ITEMS = new Set(["Mortal Reminder", "Chempunk Chainsword", "Morellonomicon", "Thornmail"]);
const CLEANSE_ITEMS = new Set(["Mercurial Scimitar", "Quicksilver Sash"]);
const SPELLSHIELD_ITEMS = new Set(["Banshee's Veil", "Edge of Night"]);
const STASIS_ITEMS = new Set(["Zhonya's Hourglass", "Guardian Angel"]);

export interface EnemyBuild {
  armor: number;
  mr: number;
  hp: number;
  adItems: number;
  apItems: number;
  tankItems: number;
  grievous: string[];
  cleanse: string[];
  spellshield: string[];
  stasis: string[];
  /** One-line intel notes about this enemy's build. */
  intel: string[];
}

const stat = (item: Item, key: string) => item.stats[key] ?? 0;

export function summarizeEnemyBuild(name: string, items: Item[], usualApShare: number): EnemyBuild {
  const b: EnemyBuild = {
    armor: 0, mr: 0, hp: 0, adItems: 0, apItems: 0, tankItems: 0,
    grievous: [], cleanse: [], spellshield: [], stasis: [], intel: [],
  };
  const armorItems: string[] = [];
  const mrItems: string[] = [];
  const healItems: string[] = [];
  const critItems: string[] = [];

  for (const item of items) {
    const armor = stat(item, "FlatArmorMod");
    const mr = stat(item, "FlatSpellBlockMod");
    const hp = stat(item, "FlatHPPoolMod");
    b.armor += armor;
    b.mr += mr;
    b.hp += hp;
    if (armor >= 30) armorItems.push(item.name);
    if (mr >= 30) mrItems.push(item.name);

    const ad = stat(item, "FlatPhysicalDamageMod") > 0 || item.tags.includes("CriticalStrike") || item.tags.includes("ArmorPenetration");
    const ap = stat(item, "FlatMagicDamageMod") > 0 || item.tags.includes("SpellDamage");
    if (!item.isBoots) {
      if (ad && ap) {
        b.adItems += 0.5;
        b.apItems += 0.5;
      } else if (ad) b.adItems += 1;
      else if (ap) b.apItems += 1;
      else if (armor + mr >= 40 || hp >= 350) b.tankItems += 1;
    }

    if (HEALING_ITEMS.has(item.name) || item.tags.includes("LifeSteal") || item.tags.includes("SpellVamp")) healItems.push(item.name);
    if (item.tags.includes("CriticalStrike")) critItems.push(item.name);
    if (GRIEVOUS_ITEMS.has(item.name)) b.grievous.push(item.name);
    if (CLEANSE_ITEMS.has(item.name)) b.cleanse.push(item.name);
    if (SPELLSHIELD_ITEMS.has(item.name)) b.spellshield.push(item.name);
    if (STASIS_ITEMS.has(item.name)) b.stasis.push(item.name);
  }

  const list = (xs: string[]) => xs.slice(0, 3).join(", ");
  if (b.armor >= 60) b.intel.push(`${name} has ${b.armor} bonus armor (${list(armorItems)}): buy armor penetration`);
  if (b.mr >= 60) b.intel.push(`${name} has ${b.mr} bonus MR (${list(mrItems)}): buy magic penetration`);
  // Only worth saying when it's not their usual damage type.
  if (b.apItems > b.adItems && b.apItems >= 1 && usualApShare < 0.6) b.intel.push(`${name} is building AP, so their damage is mostly magic`);
  else if (b.adItems > b.apItems && b.adItems >= 1 && usualApShare > 0.4) b.intel.push(`${name} is building AD, so their damage is mostly physical`);
  if (healItems.length >= 2) b.intel.push(`${name} has lots of sustain (${list(healItems)})`);
  if (critItems.length >= 2) b.intel.push(`${name} has ${critItems.length} crit items`);
  if (b.grievous.length) b.intel.push(`${name} has Grievous Wounds (${list(b.grievous)}): your healing gets cut`);
  if (b.cleanse.length) b.intel.push(`${name} can cleanse CC (${list(b.cleanse)})`);
  if (b.stasis.length) b.intel.push(`${name} has ${list(b.stasis)}, so time your burst around it`);
  return b;
}

/** Returns a copy of the profile adjusted for what the champion has actually built. */
export function applyEnemyBuild(p: ThreatProfile, items: Item[]): { profile: ThreatProfile; build: EnemyBuild } {
  const build = summarizeEnemyBuild(p.champion.name, items, p.apShare);
  if (!items.length) return { profile: p, build };

  const traits = { ...p.traits };
  let apShare = p.apShare;
  let damageWeight = p.damageWeight;
  let isTank = p.isTank;

  const dmgItems = build.adItems + build.apItems;
  if (dmgItems > 0) {
    // Items beat kit stereotypes: an AP Kai'Sa or AD Shyvana shifts the damage split.
    apShare = p.apShare * 0.4 + (build.apItems / dmgItems) * 0.6;
  }
  if (build.tankItems >= 2 && build.tankItems > dmgItems) {
    isTank = true;
    damageWeight *= 0.75;
  } else if (dmgItems >= 2 && p.isTank) {
    // A "tank" building damage hits much harder.
    damageWeight = Math.max(damageWeight, 0.9);
  }

  const count = (pred: (i: Item) => boolean) => items.filter(pred).length;
  traits.heal += 0.35 * count((i) => HEALING_ITEMS.has(i.name) || i.tags.includes("LifeSteal") || i.tags.includes("SpellVamp"));
  traits.shield += 0.4 * count((i) => SHIELD_ITEMS.has(i.name));
  traits.crit += 0.35 * count((i) => i.tags.includes("CriticalStrike"));
  traits.auto += 0.25 * count((i) => !i.isBoots && i.tags.includes("AttackSpeed"));
  traits.burst += 0.2 * count((i) => i.tags.includes("ArmorPenetration") || i.tags.includes("MagicPenetration"));
  for (const k of Object.keys(traits) as (keyof typeof traits)[]) traits[k] = Math.min(traits[k], 1.5);

  return { profile: { ...p, apShare, damageWeight, isTank, traits }, build };
}

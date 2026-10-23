import type { Champion, GameData, Item } from "../data/ddragon";
import { ARCHETYPE_LABELS, SIGNATURE_ITEMS, type Archetype, type Role } from "../data/champions";
import { ANTIHEAL_ITEMS, BOOTS_KNOWLEDGE, ITEM_KNOWLEDGE, type Need } from "../data/items";
import type { TeamAnalysis } from "./analyze";

export interface Pick {
  item: Item;
  score: number;
  reasons: string[];
  /** Already in your inventory (from the live game). */
  owned?: boolean;
}

export interface Build {
  archetype: Archetype;
  starter: Item[];
  boots?: Pick;
  core: Pick[];
  situational: Pick[];
  notes: string[];
  totalGold: number;
}

const CORE_SLOTS = 5;
// Early slots lean on what the champion wants; later slots react to the enemy.
const FIT_WEIGHT = [1, 1, 0.9, 0.8, 0.7];
const COUNTER_WEIGHT = [0.15, 0.3, 0.6, 0.8, 0.9];

const OFFENSIVE_NEEDS = new Set<Need>(["antiheal", "antishield", "armorpen", "magicpen"]);

const SQUISHY: Archetype[] = ["adc_crit", "adc_onhit", "ap_mage", "ap_assassin", "ad_assassin", "enchanter", "ad_caster"];

const NEED_PHRASES: Record<Need, (who: string) => string> = {
  armor: (w) => `Armor vs heavy physical damage${w}`,
  mr: (w) => `Magic resist vs heavy magic damage${w}`,
  antiheal: (w) => `Grievous Wounds to cut healing${w}`,
  antishield: (w) => `Breaks shields${w}`,
  armorpen: (w) => `Armor penetration vs tanky / armor-stacking enemies${w}`,
  magicpen: (w) => `Magic penetration vs tanky / MR-stacking enemies${w}`,
  antiburst: (w) => `Survives burst / assassination${w}`,
  antiCC: (w) => `Answers crowd control${w}`,
  antiAuto: (w) => `Slows down auto-attackers${w}`,
  antiCrit: (w) => `Reduces crit damage${w}`,
  antiPoke: (w) => `Sustain through poke${w}`,
};

function styleName(archetype: Archetype): string {
  return ARCHETYPE_LABELS[archetype].toLowerCase().replace(/\b(ap|ad)\b/g, (m) => m.toUpperCase());
}

function whoText(names: string[]): string {
  if (!names.length) return "";
  return ` (${names.slice(0, 3).join(", ")})`;
}

interface Candidate {
  item: Item;
  fit: number;
  counters: Partial<Record<Need, number>>;
  group?: string;
  signature: boolean;
}

function candidatesFor(data: GameData, champion: Champion, archetype: Archetype, role: Role): Candidate[] {
  const signatures = new Set(SIGNATURE_ITEMS[champion.id] ?? []);
  const out: Candidate[] = [];
  for (const [name, k] of Object.entries(ITEM_KNOWLEDGE)) {
    const item = data.itemsByName.get(name);
    if (!item) continue; // removed or renamed this patch
    if (k.supportOnly && role !== "support") continue;
    const signature = signatures.has(name);
    const fit = (k.fit[archetype] ?? 0) + (signature ? 3 : 0);
    if (fit <= 0) continue;
    out.push({
      item,
      fit,
      counters: k.counters ?? {},
      group: k.group ?? (ANTIHEAL_ITEMS.has(name) ? "antiheal" : undefined),
      signature,
    });
  }
  return out;
}

/** Carries can't afford to over-itemize defensively; tanks don't care about penetration. */
function needScale(archetype: Archetype, need: Need): number {
  if (OFFENSIVE_NEEDS.has(need)) return archetype === "tank" ? 0.5 : 1;
  if (archetype === "tank") return 1;
  return SQUISHY.includes(archetype) ? 0.6 : 0.8;
}

function scoreCandidate(c: Candidate, slot: number, analysis: TeamAnalysis, archetype: Archetype): Pick {
  const s = Math.min(slot, FIT_WEIGHT.length - 1);
  let score = c.fit * FIT_WEIGHT[s];
  const reasons: string[] = [];
  const contributions: [Need, number][] = [];
  for (const [need, amount] of Object.entries(c.counters) as [Need, number][]) {
    const v = amount * analysis.needs[need] * 8 * COUNTER_WEIGHT[s] * needScale(archetype, need);
    score += v;
    if (v >= 1) contributions.push([need, v]);
  }
  contributions.sort((a, b) => b[1] - a[1]);
  for (const [need] of contributions) reasons.push(NEED_PHRASES[need](whoText(analysis.sources[need])));
  if (c.signature) reasons.unshift("Signature item for this champion");
  else if (c.fit >= 8) reasons.unshift(`Core item for ${styleName(archetype)} builds`);
  else if (!reasons.length) reasons.push(`Solid stats for ${styleName(archetype)} builds`);
  return { item: c.item, score, reasons };
}

function pickBoots(data: GameData, archetype: Archetype, analysis: TeamAnalysis): Pick | undefined {
  const squish = SQUISHY.includes(archetype) ? 0.7 : 1;
  let best: Pick | undefined;
  for (const [name, k] of Object.entries(BOOTS_KNOWLEDGE)) {
    const item = data.itemsByName.get(name);
    if (!item) continue;
    let score = k.fit[archetype] ?? 1;
    const reasons: string[] = [];
    for (const [need, amount] of Object.entries(k.counters ?? {}) as [Need, number][]) {
      const v = amount * analysis.needs[need] * 8 * squish;
      score += v;
      if (v >= 2) reasons.push(NEED_PHRASES[need](whoText(analysis.sources[need])));
    }
    if (!reasons.length) reasons.push(`Standard boots for ${styleName(archetype)} builds`);
    if (!best || score > best.score) best = { item, score, reasons };
  }
  return best;
}

function pickStarter(data: GameData, champion: Champion, archetype: Archetype, role: Role, analysis: TeamAnalysis): Item[] {
  const get = (name: string) => data.itemsByName.get(name);
  let first: string;
  if (role === "support") first = "World Atlas";
  else if (role === "jungle") {
    first = archetype === "tank" ? "Mosstomper Seedling"
      : archetype === "bruiser" || archetype === "adc_onhit" || archetype === "adc_crit" ? "Gustwalker Hatchling"
      : "Scorchclaw Pup";
  } else if (archetype.startsWith("ap_")) first = "Doran's Ring";
  else if (archetype === "tank") first = "Doran's Helm";
  else if (role === "top" && champion.attackRange < 300 && analysis.laneOpponent?.isRanged) first = "Doran's Shield";
  else first = "Doran's Blade";

  const potions = role === "jungle" || role === "support" ? 1 : 2;
  return [get(first) ?? get("Doran's Blade"), ...Array(potions).fill(get("Health Potion"))].filter(Boolean) as Item[];
}

function buildNotes(analysis: TeamAnalysis, archetype: Archetype, core: Pick[]): string[] {
  const notes: string[] = [];
  const isAP = archetype.startsWith("ap_") || archetype === "enchanter";
  const coreNames = new Set(core.map((p) => p.item.name));

  if (analysis.needs.antiheal >= 0.5) {
    const component = isAP ? "Oblivion Orb" : archetype === "tank" ? "Bramble Vest" : "Executioner's Calling";
    const hasGW = [...ANTIHEAL_ITEMS].some((n) => coreNames.has(n));
    notes.push(
      `Heavy healing on the enemy team (${analysis.sources.antiheal.slice(0, 3).join(", ")}). ` +
        `Pick up ${component} early${hasGW ? " and finish it into your Grievous Wounds item" : ""} — even the component is worth it.`,
    );
  }
  if (analysis.suppressors.length) {
    notes.push(
      `${analysis.suppressors.join(", ")} can lock you down with suppression/ultimate CC — Quicksilver Sash ` +
        `(or Mercurial Scimitar) removes it, and Cleanse is a strong summoner spell.`,
    );
  }
  const grievousOwners = analysis.profiles.filter((p) => analysis.builds.get(p.champion.id)?.grievous.length);
  if (grievousOwners.length && ["bruiser", "ap_battlemage", "enchanter", "adc_onhit"].includes(archetype)) {
    notes.push(
      `${grievousOwners.map((p) => p.champion.name).join(", ")} already bought Grievous Wounds, so your healing is cut. ` +
        `Resistances and shields are worth more than extra lifesteal or healing.`,
    );
  }
  if (analysis.stealth.length) {
    notes.push(`${analysis.stealth.join(", ")} use stealth — carry Control Wards and consider Oracle Lens.`);
  }
  if (analysis.needs.antiPoke >= 0.6 && SQUISHY.includes(archetype)) {
    notes.push(`Poke-heavy enemy comp (${analysis.sources.antiPoke.slice(0, 3).join(", ")}) — consider Second Wind/Bone Plating runes and extra potions.`);
  }
  if (analysis.needs.antiburst >= 0.7 && SQUISHY.includes(archetype)) {
    notes.push(`Lots of burst — a Stopwatch / early defensive component can save a back-line carry.`);
  }
  return notes;
}

export function recommendBuild(
  data: GameData,
  champion: Champion,
  archetype: Archetype,
  role: Role,
  analysis: TeamAnalysis,
  owned: Item[] = [],
): Build {
  const candidates = candidatesFor(data, champion, archetype, role);
  const core: Pick[] = [];
  const usedGroups = new Set<string>();
  const used = new Set<string>();

  // What you've already bought stays in the build; the engine plans the rest around it.
  const ownedBoots = owned.find((i) => i.isBoots);
  for (const item of owned.filter((i) => !i.isBoots)) {
    core.push({ item, score: 0, reasons: ["Already bought"], owned: true });
    used.add(item.name);
    const g = ITEM_KNOWLEDGE[item.name]?.group ?? (ANTIHEAL_ITEMS.has(item.name) ? "antiheal" : undefined);
    if (g) usedGroups.add(g);
  }

  for (let slot = core.length; slot < CORE_SLOTS; slot++) {
    let best: Pick | undefined;
    for (const c of candidates) {
      if (used.has(c.item.name) || (c.group && usedGroups.has(c.group))) continue;
      const p = scoreCandidate(c, slot, analysis, archetype);
      if (!best || p.score > best.score) best = p;
    }
    if (!best) break;
    core.push(best);
    used.add(best.item.name);
    const g = candidates.find((c) => c.item.name === best!.item.name)?.group;
    if (g) usedGroups.add(g);
  }

  // Situational: items that answer the comp better than they fit generically.
  const situational = candidates
    .filter((c) => !used.has(c.item.name) && Object.keys(c.counters).length)
    .map((c) => {
      const p = scoreCandidate(c, CORE_SLOTS - 1, analysis, archetype);
      const counterValue = p.score - c.fit * FIT_WEIGHT[FIT_WEIGHT.length - 1];
      return { p, counterValue };
    })
    .filter((x) => x.counterValue >= 1.5)
    .sort((a, b) => b.p.score - a.p.score)
    .slice(0, 5)
    .map((x) => ({ ...x.p, reasons: x.p.reasons.filter((r) => !r.startsWith("Core item") && !r.startsWith("Solid stats")) }));

  const boots = ownedBoots
    ? { item: ownedBoots, score: 0, reasons: ["Already bought"], owned: true }
    : pickBoots(data, archetype, analysis);
  const starter = pickStarter(data, champion, archetype, role, analysis);
  const totalGold = core.reduce((g, p) => g + p.item.gold, 0) + (boots?.item.gold ?? 0);

  return { archetype, starter, boots, core, situational, notes: buildNotes(analysis, archetype, core), totalGold };
}

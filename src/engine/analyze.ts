import type { Champion, Item } from "../data/ddragon";
import { threatProfile, type ThreatProfile, type Trait } from "../data/champions";
import type { Need } from "../data/items";
import { applyEnemyBuild, type EnemyBuild } from "./enemyItems";

export const LANE_OPPONENT_WEIGHT = 1.6;

export interface TeamAnalysis {
  profiles: ThreatProfile[];
  laneOpponent?: ThreatProfile;
  adShare: number;
  apShare: number;
  /** 0..1 — how much the enemy comp demands each kind of answer. */
  needs: Record<Need, number>;
  /** Enemy champion names driving each need, biggest contributor first. */
  sources: Record<Need, string[]>;
  stealth: string[];
  suppressors: string[];
  /** What each enemy has bought, keyed by champion id. */
  builds: Map<string, EnemyBuild>;
  /** Notes read from enemy items ("Malphite has 150 bonus armor…"). */
  intel: string[];
}

export interface Enemy {
  champion: Champion;
  items: Item[];
}

const clamp01 = (n: number) => Math.max(0, Math.min(1, n));

const NEEDS: Need[] = [
  "armor", "mr", "antiheal", "antishield", "armorpen", "magicpen",
  "antiburst", "antiCC", "antiAuto", "antiCrit", "antiPoke",
];

export function analyzeEnemies(enemies: Enemy[], laneOpponentId?: string): TeamAnalysis {
  const builds = new Map<string, EnemyBuild>();
  const profiles = enemies.map((e) => {
    const { profile, build } = applyEnemyBuild(threatProfile(e.champion), e.items);
    builds.set(e.champion.id, build);
    return profile;
  });
  const weightOf = (p: ThreatProfile) => (p.champion.id === laneOpponentId ? LANE_OPPONENT_WEIGHT : 1);

  let ad = 0;
  let ap = 0;
  for (const p of profiles) {
    const w = weightOf(p) * p.damageWeight;
    ap += w * p.apShare;
    ad += w * (1 - p.apShare);
  }
  const total = ad + ap || 1;
  const adShare = profiles.length ? ad / total : 0.5;
  const apShare = profiles.length ? ap / total : 0.5;

  // Per-champion contribution to each need, so we can both sum and explain.
  const contrib = (p: ThreatProfile): Record<Need, number> => {
    const t = p.traits;
    const w = weightOf(p);
    const dmg = w * p.damageWeight;
    const tankiness = p.isTank ? 1 + (p.champion.info.defense >= 8 ? 0.3 : 0) : 0;
    // Bought resistances count on top of the kit: 150 bonus armor ≈ one more tank to chew through.
    const b = builds.get(p.champion.id);
    const hpBulk = (b?.hp ?? 0) / 2000;
    return {
      armor: dmg * (1 - p.apShare),
      mr: dmg * p.apShare,
      antiheal: w * t.heal,
      antishield: w * t.shield,
      armorpen: w * (tankiness + (b?.armor ?? 0) / 150 + hpBulk),
      magicpen: w * (tankiness + (b?.mr ?? 0) / 150 + hpBulk),
      antiburst: w * t.burst,
      antiCC: w * (t.cc + t.suppress * 0.6),
      antiAuto: dmg * t.auto,
      antiCrit: w * t.crit,
      antiPoke: w * t.poke,
    };
  };

  const perChamp = profiles.map((p) => ({ p, c: contrib(p) }));
  const sum = (n: Need) => perChamp.reduce((acc, x) => acc + x.c[n], 0);

  const needs: Record<Need, number> = {
    armor: profiles.length ? clamp01((adShare - 0.3) / 0.5) : 0,
    mr: profiles.length ? clamp01((apShare - 0.3) / 0.5) : 0,
    antiheal: clamp01(sum("antiheal") / 1.6),
    antishield: clamp01(sum("antishield") / 2),
    armorpen: clamp01(sum("armorpen") / 3),
    magicpen: clamp01(sum("magicpen") / 3),
    antiburst: clamp01(sum("antiburst") / 1.8),
    antiCC: clamp01(sum("antiCC") / 3),
    antiAuto: clamp01(sum("antiAuto") / 2),
    antiCrit: clamp01(sum("antiCrit") / 1.5),
    antiPoke: clamp01(sum("antiPoke") / 1.8),
  };

  const sources = {} as Record<Need, string[]>;
  for (const n of NEEDS) {
    // Penetration sources must be genuinely tanky, not just someone with one defensive item.
    const threshold = n === "armor" || n === "mr" ? 0.5 : n === "armorpen" || n === "magicpen" ? 0.9 : 0.3;
    sources[n] = perChamp
      .filter((x) => x.c[n] >= threshold)
      .sort((a, b) => b.c[n] - a.c[n])
      .map((x) => x.p.champion.name);
  }

  const named = (trait: Trait, min: number) =>
    profiles.filter((p) => p.traits[trait] >= min).map((p) => p.champion.name);

  return {
    profiles,
    laneOpponent: profiles.find((p) => p.champion.id === laneOpponentId),
    adShare,
    apShare,
    needs,
    sources,
    stealth: named("stealth", 0.5),
    suppressors: named("suppress", 1),
    builds,
    intel: [...builds.values()].flatMap((b) => b.intel),
  };
}

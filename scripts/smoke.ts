// Prints recommended builds for a few sample matchups against live Data Dragon data.
import { loadGameData } from "../src/data/ddragon";
import { archetypeFor, defaultRoleFor } from "../src/data/champions";
import { ITEM_KNOWLEDGE, BOOTS_KNOWLEDGE } from "../src/data/items";
import { analyzeEnemies } from "../src/engine/analyze";
import { recommendBuild } from "../src/engine/recommend";

const SCENARIOS: [string, string[], string[][]?][] = [
  ["Jinx", ["Draven", "Leona", "Zed", "Malphite", "Soraka"]],
  ["Ahri", ["Fizz", "Kassadin", "Elise", "Mordekaiser", "Lulu"]],
  ["Darius", ["Teemo", "Aatrox", "Vladimir", "Zed", "Yuumi"]],
  ["Ornn", ["Yasuo", "Tryndamere", "MasterYi", "Jinx", "Thresh"]],
  ["Zed", ["Ahri", "Malphite", "Sejuani", "Lulu", "Caitlyn"]],
  ["Kayle", ["Malzahar", "Warwick", "Garen", "Ashe", "Janna"]],
  // Same comp as the first, but with items: Malphite stacking armor, Draven with Mortal Reminder.
  ["Jinx", ["Draven", "Leona", "Zed", "Malphite", "Soraka"],
    [["Mortal Reminder", "Infinity Edge"], [], ["Edge of Night"], ["Thornmail", "Frozen Heart", "Plated Steelcaps"], ["Moonstone Renewer"]]],
  // Kai'Sa building AP should flip the damage read.
  ["Ornn", ["Kaisa", "Elise", "Ahri", "Sejuani", "Lulu"],
    [["Nashor's Tooth", "Riftmaker", "Zhonya's Hourglass"], [], [], [], []]],
];

const data = await loadGameData();
console.log("Patch", data.version);
const missing = [...Object.keys(ITEM_KNOWLEDGE), ...Object.keys(BOOTS_KNOWLEDGE)].filter((n) => !data.itemsByName.has(n));
if (missing.length) console.log("Unknown item names:", missing);

for (const [you, enemyIds, enemyItems] of SCENARIOS) {
  const champ = data.championsById.get(you)!;
  const enemies = enemyIds.map((id, i) => ({
    champion: data.championsById.get(id)!,
    items: (enemyItems?.[i] ?? []).map((n) => data.itemsByName.get(n)!),
  }));
  const analysis = analyzeEnemies(enemies, enemyIds[0]);
  const build = recommendBuild(data, champ, archetypeFor(champ), defaultRoleFor(champ), analysis);
  console.log(`\n=== ${champ.name} (${build.archetype}) vs ${enemies.map((e) => e.champion.name).join(", ")}`);
  analysis.intel.forEach((n) => console.log(" i", n));
  console.log(`AD ${(analysis.adShare * 100).toFixed(0)}% / AP ${(analysis.apShare * 100).toFixed(0)}%`,
    Object.entries(analysis.needs).filter(([, v]) => v > 0.25).map(([k, v]) => `${k}=${v.toFixed(2)}`).join(" "));
  console.log("Start:", build.starter.map((i) => i.name).join(" + "));
  console.log("Boots:", build.boots?.item.name, "—", build.boots?.reasons.join("; "));
  build.core.forEach((p, i) => console.log(` ${i + 1}. ${p.item.name} [${p.score.toFixed(1)}] — ${p.reasons.join("; ")}`));
  console.log(" Situational:", build.situational.map((p) => p.item.name).join(", "));
  build.notes.forEach((n) => console.log(" *", n));
}

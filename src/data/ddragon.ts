// Live champion/item data from Riot's public Data Dragon CDN (no API key needed).

const BASE = "https://ddragon.leagueoflegends.com";

export interface Champion {
  id: string; // e.g. "MonkeyKing"
  name: string; // e.g. "Wukong"
  title: string;
  tags: string[];
  info: { attack: number; defense: number; magic: number; difficulty: number };
  attackRange: number;
  image: string;
}

export interface Item {
  id: string;
  name: string;
  description: string; // HTML from Riot
  plaintext: string;
  gold: number;
  tags: string[];
  image: string;
  stats: Record<string, number>;
  isBoots: boolean;
  isCompleted: boolean;
}

export interface GameData {
  version: string;
  champions: Champion[];
  championsById: Map<string, Champion>;
  itemsByName: Map<string, Item>;
  itemsById: Map<string, Item>;
}

interface RawChampion {
  id: string;
  name: string;
  title: string;
  tags: string[];
  info: Champion["info"];
  stats: { attackrange: number };
  image: { full: string };
}

interface RawItem {
  name: string;
  description: string;
  plaintext: string;
  gold: { total: number; purchasable: boolean };
  tags?: string[];
  maps?: Record<string, boolean>;
  into?: string[];
  inStore?: boolean;
  requiredChampion?: string;
  requiredAlly?: string;
  stats?: Record<string, number>;
  image: { full: string };
}

async function getJson<T>(url: string): Promise<T> {
  const res = await fetch(url);
  if (!res.ok) throw new Error(`Request failed (${res.status}): ${url}`);
  return res.json() as Promise<T>;
}

export async function loadGameData(): Promise<GameData> {
  const versions = await getJson<string[]>(`${BASE}/api/versions.json`);
  const version = versions[0];
  const [champRes, itemRes] = await Promise.all([
    getJson<{ data: Record<string, RawChampion> }>(`${BASE}/cdn/${version}/data/en_US/champion.json`),
    getJson<{ data: Record<string, RawItem> }>(`${BASE}/cdn/${version}/data/en_US/item.json`),
  ]);
  return parseGameData(version, champRes.data, itemRes.data);
}

export function parseGameData(
  version: string,
  rawChampions: Record<string, RawChampion>,
  rawItems: Record<string, RawItem>,
): GameData {
  const champions = Object.values(rawChampions)
    .map<Champion>((c) => ({
      id: c.id,
      name: c.name,
      title: c.title,
      tags: c.tags,
      info: c.info,
      attackRange: c.stats.attackrange,
      image: `${BASE}/cdn/${version}/img/champion/${c.image.full}`,
    }))
    .sort((a, b) => a.name.localeCompare(b.name));

  const itemsByName = new Map<string, Item>();
  for (const [id, raw] of Object.entries(rawItems)) {
    // Summoner's Rift only; ids >= 10000 are mode-specific copies (Arena, ARAM, ...).
    if (Number(id) >= 10000 || !raw.maps?.["11"] || !raw.gold.purchasable) continue;
    if (raw.inStore === false || raw.requiredChampion || raw.requiredAlly) continue;
    if (itemsByName.has(raw.name)) continue;
    const tags = raw.tags ?? [];
    itemsByName.set(raw.name, {
      id,
      name: raw.name,
      description: raw.description,
      plaintext: raw.plaintext,
      gold: raw.gold.total,
      tags,
      image: `${BASE}/cdn/${version}/img/item/${raw.image.full}`,
      stats: raw.stats ?? {},
      isBoots: tags.includes("Boots"),
      isCompleted: !raw.into?.length,
    });
  }

  return {
    version,
    champions,
    championsById: new Map(champions.map((c) => [c.id, c])),
    itemsByName,
    itemsById: new Map([...itemsByName.values()].map((i) => [i.id, i])),
  };
}

/** Items worth tracking on an enemy: finished items and boots, no starters/consumables. */
export function isTrackableItem(item: Item): boolean {
  if (item.tags.includes("Consumable") || item.tags.includes("Trinket")) return false;
  return (item.isCompleted || item.isBoots) && item.gold >= 700;
}

/** Riot descriptions are HTML; turn them into readable plain text. */
export function describeItem(item: Item): string {
  return item.description
    .replace(/<br\s*\/?>/gi, "\n")
    .replace(/<\/(stats|passive|active|li)>/gi, "\n")
    .replace(/<[^>]+>/g, " ")
    .replace(/[ \t]+/g, " ")
    .replace(/ *\n */g, "\n")
    .replace(/\n{2,}/g, "\n")
    .trim();
}

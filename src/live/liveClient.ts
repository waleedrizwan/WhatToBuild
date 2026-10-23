// Reads the in-progress game from League's Live Client Data API
// (https://developer.riotgames.com/docs/lol#game-client-api_live-client-data-api).
// Requests go to /liveclientdata/*, which the Vite dev server proxies to 127.0.0.1:2999.

import type { Champion, GameData } from "../data/ddragon";
import { isTrackableItem } from "../data/ddragon";
import type { Role } from "../data/champions";

interface RawLivePlayer {
  championName: string;
  rawChampionName?: string;
  riotId?: string;
  riotIdGameName?: string;
  summonerName?: string;
  team: "ORDER" | "CHAOS";
  position?: string;
  items: { itemID: number; displayName: string }[];
}

interface RawLiveGame {
  activePlayer?: { riotId?: string; riotIdGameName?: string; summonerName?: string };
  allPlayers: RawLivePlayer[];
  gameData?: { gameMode?: string; gameTime?: number; mapNumber?: number };
}

export interface LiveGame {
  you: string;
  yourItems: string[];
  role?: Role;
  /** Up to 5 enemies, champion id + item names. */
  enemies: { id: string; items: string[] }[];
  /** Index into `enemies` of your lane opponent, if positions are known. */
  laneIndex?: number;
  gameTime: number;
  gameMode?: string;
}

export type LiveStatus =
  | { state: "off" }
  | { state: "waiting"; message: string }
  | { state: "connected"; game: LiveGame };

const POSITION_TO_ROLE: Record<string, Role> = {
  TOP: "top",
  JUNGLE: "jungle",
  MIDDLE: "mid",
  BOTTOM: "bot",
  UTILITY: "support",
};

function championFor(p: RawLivePlayer, data: GameData): Champion | undefined {
  // rawChampionName is "game_character_displayname_MonkeyKing" — the Data Dragon id, and not localized.
  const rawId = p.rawChampionName?.replace(/^game_character_displayname_/, "");
  if (rawId && data.championsById.has(rawId)) return data.championsById.get(rawId);
  return data.champions.find((c) => c.name === p.championName);
}

function itemNames(p: RawLivePlayer, data: GameData): string[] {
  return p.items
    .map((i) => data.itemsById.get(String(i.itemID)))
    .filter((i) => i !== undefined && isTrackableItem(i))
    .map((i) => i!.name);
}

function isActivePlayer(p: RawLivePlayer, active: RawLiveGame["activePlayer"]): boolean {
  if (!active) return false;
  if (active.riotId && p.riotId) return active.riotId === p.riotId;
  if (active.riotIdGameName && p.riotIdGameName) return active.riotIdGameName === p.riotIdGameName;
  return !!active.summonerName && active.summonerName === p.summonerName;
}

export function parseLiveGame(raw: RawLiveGame, data: GameData): LiveGame | null {
  const me = raw.allPlayers.find((p) => isActivePlayer(p, raw.activePlayer));
  if (!me) return null; // spectating, or the game is still loading
  const myChamp = championFor(me, data);
  if (!myChamp) return null;

  const role = me.position ? POSITION_TO_ROLE[me.position] : undefined;
  const enemyPlayers = raw.allPlayers.filter((p) => p.team !== me.team).slice(0, 5);
  const enemies = enemyPlayers.flatMap((p) => {
    const c = championFor(p, data);
    return c ? [{ id: c.id, items: itemNames(p, data), position: p.position }] : [];
  });
  const laneIndex = me.position ? enemies.findIndex((e) => e.position === me.position) : -1;

  return {
    you: myChamp.id,
    yourItems: itemNames(me, data),
    role,
    enemies: enemies.map(({ id, items }) => ({ id, items })),
    laneIndex: laneIndex >= 0 ? laneIndex : undefined,
    gameTime: raw.gameData?.gameTime ?? 0,
    gameMode: raw.gameData?.gameMode,
  };
}

export async function fetchLiveGame(data: GameData, signal?: AbortSignal): Promise<LiveStatus> {
  let res: Response;
  try {
    res = await fetch("/liveclientdata/allgamedata", { signal, cache: "no-store" });
  } catch {
    return { state: "waiting", message: "Can't reach the app's dev server" };
  }
  // The proxy answers 5xx when nothing is listening on port 2999, i.e. no game running.
  if (res.status >= 500) return { state: "waiting", message: "No game running. Waiting for a match to start…" };
  if (!res.ok) return { state: "waiting", message: "Game is loading…" };
  try {
    const game = parseLiveGame((await res.json()) as RawLiveGame, data);
    return game ? { state: "connected", game } : { state: "waiting", message: "Game is loading…" };
  } catch {
    return { state: "waiting", message: "Game is loading…" };
  }
}

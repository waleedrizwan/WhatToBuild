// Fake League Live Client Data API for trying "Live game" mode without being in a match.
//   npm run mock-live              (terminal 1)
//   npm run dev:mock               (terminal 2)
// Items are bought over time, so the build updates as the "game" progresses.

import { createServer } from "node:http";

const PORT = 2998;
const started = Date.now();

type Team = "ORDER" | "CHAOS";
interface MockPlayer {
  champ: string; // Data Dragon id
  name: string; // display name
  team: Team;
  position: string;
  riotId: string;
  /** Item ids, bought one every ~20s of mock time. */
  buys: number[];
}

const PLAYERS: MockPlayer[] = [
  { champ: "Jinx", name: "Jinx", team: "ORDER", position: "BOTTOM", riotId: "You#NA1", buys: [3006, 3031, 3032] },
  { champ: "Ornn", name: "Ornn", team: "ORDER", position: "TOP", riotId: "Ally1#NA1", buys: [3047, 6665] },
  { champ: "LeeSin", name: "Lee Sin", team: "ORDER", position: "JUNGLE", riotId: "Ally2#NA1", buys: [3071] },
  { champ: "Ahri", name: "Ahri", team: "ORDER", position: "MIDDLE", riotId: "Ally3#NA1", buys: [6655] },
  { champ: "Thresh", name: "Thresh", team: "ORDER", position: "UTILITY", riotId: "Ally4#NA1", buys: [3190] },
  { champ: "Malphite", name: "Malphite", team: "CHAOS", position: "TOP", riotId: "Enemy1#NA1", buys: [3047, 3075, 3110] },
  { champ: "Vi", name: "Vi", team: "CHAOS", position: "JUNGLE", riotId: "Enemy2#NA1", buys: [3071, 3053] },
  { champ: "Kaisa", name: "Kai'Sa", team: "CHAOS", position: "BOTTOM", riotId: "Enemy3#NA1", buys: [3115, 4633, 3157] },
  { champ: "Zed", name: "Zed", team: "CHAOS", position: "MIDDLE", riotId: "Enemy4#NA1", buys: [6698, 3814] },
  { champ: "Soraka", name: "Soraka", team: "CHAOS", position: "UTILITY", riotId: "Enemy5#NA1", buys: [6617] },
];

function snapshot() {
  const gameTime = 300 + (Date.now() - started) / 1000; // start at 5:00
  const bought = Math.floor((Date.now() - started) / 20000) + 1;
  return {
    activePlayer: { riotId: "You#NA1", riotIdGameName: "You", summonerName: "You" },
    allPlayers: PLAYERS.map((p) => ({
      championName: p.name,
      rawChampionName: `game_character_displayname_${p.champ}`,
      riotId: p.riotId,
      riotIdGameName: p.riotId.split("#")[0],
      summonerName: p.riotId.split("#")[0],
      team: p.team,
      position: p.position,
      items: p.buys.slice(0, bought).map((id, slot) => ({ itemID: id, displayName: String(id), slot, count: 1 })),
    })),
    gameData: { gameMode: "CLASSIC", gameTime, mapNumber: 11 },
  };
}

createServer((req, res) => {
  if (req.url?.startsWith("/liveclientdata/allgamedata")) {
    res.writeHead(200, { "Content-Type": "application/json" });
    res.end(JSON.stringify(snapshot()));
  } else {
    res.writeHead(404).end();
  }
}).listen(PORT, "127.0.0.1", () => console.log(`Mock Live Client API on http://127.0.0.1:${PORT}`));

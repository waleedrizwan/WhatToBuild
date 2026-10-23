import { useEffect, useMemo, useState } from "react";
import { isTrackableItem, loadGameData, type GameData, type Item } from "./data/ddragon";
import {
  ARCHETYPE_LABELS,
  ROLE_LABELS,
  archetypeFor,
  defaultRoleFor,
  type Archetype,
  type Role,
} from "./data/champions";
import { analyzeEnemies, type Enemy } from "./engine/analyze";
import { recommendBuild } from "./engine/recommend";
import { ChampionPicker } from "./components/ChampionPicker";
import { EnemyItemsPicker } from "./components/EnemyItemsPicker";
import { ItemCard } from "./components/ItemCard";
import { TeamAnalysisPanel } from "./components/TeamAnalysisPanel";
import { useLiveGame } from "./live/useLiveGame";
import type { LiveGame } from "./live/liveClient";

interface Selection {
  you: string | null;
  role: Role | "auto";
  style: Archetype | "auto";
  enemies: (string | null)[];
  /** Item names each enemy has built, parallel to `enemies`. */
  enemyItems: string[][];
  laneIndex: number;
  /** Items you've already bought; the build plans what to buy next. */
  yourItems: string[];
}

const EMPTY: Selection = {
  you: null,
  role: "auto",
  style: "auto",
  enemies: [null, null, null, null, null],
  enemyItems: [[], [], [], [], []],
  laneIndex: 0,
  yourItems: [],
};
const STORAGE_KEY = "tolleagle:selection";

/** Overwrite the picks with what the live game reports; keep the user's build-style choice. */
function applyLiveGame(s: Selection, game: LiveGame): Selection {
  const pad = <T,>(xs: T[], fill: T) => [...xs, ...Array(Math.max(0, 5 - xs.length)).fill(fill)].slice(0, 5);
  return {
    ...s,
    you: game.you,
    role: game.role ?? (s.you === game.you ? s.role : "auto"),
    style: s.you === game.you ? s.style : "auto",
    yourItems: game.yourItems,
    enemies: pad<string | null>(game.enemies.map((e) => e.id), null),
    enemyItems: pad<string[]>(game.enemies.map((e) => e.items), []),
    laneIndex: game.laneIndex ?? (s.you === game.you ? s.laneIndex : 0),
  };
}

function formatGameTime(seconds: number): string {
  const m = Math.floor(seconds / 60);
  return `${m}:${String(Math.floor(seconds % 60)).padStart(2, "0")}`;
}

function loadSelection(): Selection {
  try {
    const raw = localStorage.getItem(STORAGE_KEY);
    if (raw) {
      const saved = { ...EMPTY, ...JSON.parse(raw) } as Selection;
      if (saved.enemyItems.length !== saved.enemies.length) saved.enemyItems = EMPTY.enemyItems;
      return saved;
    }
  } catch {
    // storage unavailable — start fresh
  }
  return EMPTY;
}

export default function App() {
  const [data, setData] = useState<GameData | null>(null);
  const [error, setError] = useState<string | null>(null);
  const [sel, setSel] = useState<Selection>(loadSelection);
  const [liveEnabled, setLiveEnabled] = useState(false);
  const live = useLiveGame(data, liveEnabled, (game) => setSel((s) => applyLiveGame(s, game)));

  useEffect(() => {
    loadGameData().then(setData, (e: Error) => setError(e.message));
  }, []);

  useEffect(() => {
    try {
      localStorage.setItem(STORAGE_KEY, JSON.stringify(sel));
    } catch {
      // ignore
    }
  }, [sel]);

  const you = sel.you ? data?.championsById.get(sel.you) : undefined;
  const enemyChamps = useMemo(
    () => (data ? sel.enemies.map((id) => (id ? data.championsById.get(id) : undefined)) : []),
    [data, sel.enemies],
  );
  const itemCatalog = useMemo(
    () => (data ? [...data.itemsByName.values()].filter(isTrackableItem).sort((a, b) => a.name.localeCompare(b.name)) : []),
    [data],
  );
  const enemies = useMemo<Enemy[]>(
    () =>
      enemyChamps.flatMap((champion, i) =>
        champion
          ? [{
              champion,
              items: (sel.enemyItems[i] ?? []).map((n) => data!.itemsByName.get(n)).filter((x): x is Item => !!x),
            }]
          : [],
      ),
    [data, enemyChamps, sel.enemyItems],
  );
  const laneOpponentId = sel.enemies[sel.laneIndex] ?? undefined;
  const ownedItems = useMemo(
    () => (data ? sel.yourItems.map((n) => data.itemsByName.get(n)).filter((x): x is Item => !!x) : []),
    [data, sel.yourItems],
  );

  const archetype = you ? (sel.style === "auto" ? archetypeFor(you) : sel.style) : undefined;
  const role = you ? (sel.role === "auto" ? defaultRoleFor(you) : sel.role) : undefined;

  const analysis = useMemo(
    () => analyzeEnemies(enemies, laneOpponentId),
    [enemies, laneOpponentId],
  );
  const build = useMemo(
    () => (data && you && archetype && role ? recommendBuild(data, you, archetype, role, analysis, ownedItems) : null),
    [data, you, archetype, role, analysis, ownedItems],
  );

  const nextBuy = build && [build.boots, ...build.core].find((p) => p && !p.owned);
  const goldLeft = build
    ? build.totalGold - [build.boots, ...build.core].reduce((g, p) => g + (p?.owned ? p.item.gold : 0), 0)
    : 0;

  const taken = new Set([sel.you, ...sel.enemies].filter((x): x is string => !!x));
  const enemyCount = enemyChamps.filter(Boolean).length;

  if (error) {
    return (
      <div className="center">
        <p>Couldn't load champion data from Riot's Data Dragon: {error}</p>
      </div>
    );
  }
  if (!data) {
    return (
      <div className="center">
        <div className="spinner" />
        <p className="muted">Loading the latest patch data…</p>
      </div>
    );
  }

  // A new champion in a slot starts with no items.
  const setEnemy = (i: number, id: string | null) =>
    setSel((s) => ({
      ...s,
      enemies: s.enemies.map((e, j) => (j === i ? id : e)),
      enemyItems: s.enemyItems.map((items, j) => (j === i ? [] : items)),
    }));
  const setEnemyItems = (i: number, names: string[]) =>
    setSel((s) => ({ ...s, enemyItems: s.enemyItems.map((items, j) => (j === i ? names : items)) }));

  return (
    <div className="app">
      <header>
        <h1>Tolleagle</h1>
        <p className="muted">
          Matchup-aware item builds · patch {data.version}
          <button className="link" onClick={() => setSel(EMPTY)} disabled={live.state === "connected"}>
            Reset
          </button>
        </p>
      </header>

      <main>
        <aside className="setup">
          <section className={`panel live live-${live.state}`}>
            <div className="live-head">
              <h2>Live game</h2>
              <label className="switch">
                <input type="checkbox" checked={liveEnabled} onChange={(e) => setLiveEnabled(e.target.checked)} />
                <span />
              </label>
            </div>
            {live.state === "off" && (
              <p className="muted small">
                Turn on during a match to fill in your champion, the enemy team and everyone's items automatically.
              </p>
            )}
            {live.state === "waiting" && (
              <p className="small">
                <span className="dot" /> {live.message}
              </p>
            )}
            {live.state === "connected" && (
              <p className="small">
                <span className="dot on" /> Connected · {formatGameTime(live.game.gameTime)}
                {live.game.gameMode && ` · ${live.game.gameMode}`}. Updates every 5s.
              </p>
            )}
          </section>

          <section className="panel">
            <h2>Your champion</h2>
            <ChampionPicker
              champions={data.champions}
              value={sel.you}
              onChange={(id) => setSel((s) => ({ ...s, you: id, role: "auto", style: "auto" }))}
              placeholder="Search champion…"
              taken={taken}
            />
            {you && (
              <div className="row">
                <label>
                  Role
                  <select value={sel.role} onChange={(e) => setSel((s) => ({ ...s, role: e.target.value as Selection["role"] }))}>
                    <option value="auto">Auto ({ROLE_LABELS[defaultRoleFor(you)]})</option>
                    {Object.entries(ROLE_LABELS).map(([k, v]) => (
                      <option key={k} value={k}>
                        {v}
                      </option>
                    ))}
                  </select>
                </label>
                <label>
                  Build style
                  <select value={sel.style} onChange={(e) => setSel((s) => ({ ...s, style: e.target.value as Selection["style"] }))}>
                    <option value="auto">Auto ({ARCHETYPE_LABELS[archetypeFor(you)]})</option>
                    {Object.entries(ARCHETYPE_LABELS).map(([k, v]) => (
                      <option key={k} value={k}>
                        {v}
                      </option>
                    ))}
                  </select>
                </label>
              </div>
            )}
            {you && (
              <div className="your-items">
                <span className="muted small">Your items</span>
                <EnemyItemsPicker
                  catalog={itemCatalog}
                  value={sel.yourItems}
                  onChange={(names) => setSel((s) => ({ ...s, yourItems: names }))}
                  championName={you.name}
                />
              </div>
            )}
          </section>

          <section className="panel">
            <h2>Enemy team</h2>
            <p className="muted small">
              Pick your lane opponent with the ◎; they count most toward your early items. Add what each enemy has built
              with <strong>+</strong> to adapt to their actual items.
            </p>
            <div className="enemies">
              {sel.enemies.map((id, i) => (
                <div key={i} className={`enemy-slot${sel.laneIndex === i ? " lane" : ""}`}>
                <div className="enemy-row">
                  <button
                    type="button"
                    className="lane-toggle"
                    title="Lane opponent"
                    aria-pressed={sel.laneIndex === i}
                    onClick={() => setSel((s) => ({ ...s, laneIndex: i }))}
                  >
                    {sel.laneIndex === i ? "◉" : "◎"}
                  </button>
                  <ChampionPicker
                    champions={data.champions}
                    value={id}
                    onChange={(v) => setEnemy(i, v)}
                    placeholder={i === sel.laneIndex ? "Lane opponent…" : `Enemy ${i + 1}…`}
                    taken={taken}
                  />
                </div>
                {enemyChamps[i] && (
                  <EnemyItemsPicker
                    catalog={itemCatalog}
                    value={sel.enemyItems[i] ?? []}
                    onChange={(names) => setEnemyItems(i, names)}
                    championName={enemyChamps[i]!.name}
                  />
                )}
                </div>
              ))}
            </div>
          </section>
        </aside>

        <div className="results">
          {!you && (
            <section className="panel empty">
              <h2>Pick your champion to get started</h2>
              <p className="muted">
                Then add as many enemies as you know. Every pick updates the build: armor vs AD-heavy comps, Grievous Wounds
                vs healers, penetration vs tanks, and so on.
              </p>
            </section>
          )}

          {you && build && (
            <>
              <section className="panel hero">
                <img src={you.image} alt="" />
                <div>
                  <h2>{you.name}</h2>
                  <p className="muted">
                    {ARCHETYPE_LABELS[build.archetype]} · {ROLE_LABELS[role!]}
                    {enemyCount > 0 && ` · vs ${enemyChamps.filter(Boolean).map((c) => c!.name).join(", ")}`}
                  </p>
                </div>
                <div className="total">
                  <span className="muted small">{goldLeft < build.totalGold ? "Gold left to full build" : "Full build"}</span>
                  <span className="gold big">{goldLeft.toLocaleString()}g</span>
                </div>
              </section>

              {enemyCount > 0 && <TeamAnalysisPanel analysis={analysis} />}

              <section className="panel">
                <h2>Start</h2>
                <div className="starter">
                  {build.starter.map((item, i) => (
                    <ItemCard key={i} item={item} compact />
                  ))}
                </div>
              </section>

              <section className="panel">
                <h2>Build order</h2>
                <div className="core">
                  {build.boots && (
                    <ItemCard
                      item={build.boots.item}
                      reasons={build.boots.reasons}
                      index={0}
                      status={build.boots.owned ? "owned" : build.boots === nextBuy ? "next" : undefined}
                    />
                  )}
                  {build.core.map((p, i) => (
                    <ItemCard
                      key={p.item.id}
                      item={p.item}
                      reasons={p.reasons}
                      index={i + 1}
                      status={p.owned ? "owned" : p === nextBuy ? "next" : undefined}
                    />
                  ))}
                </div>
                <p className="muted small">Boots (0) usually come after your first item; buy tier-1 Boots early as needed.</p>
              </section>

              {build.situational.length > 0 && (
                <section className="panel">
                  <h2>Situational swaps</h2>
                  <div className="core">
                    {build.situational.map((p) => (
                      <ItemCard key={p.item.id} item={p.item} reasons={p.reasons} />
                    ))}
                  </div>
                </section>
              )}

              {build.notes.length > 0 && (
                <section className="panel notes">
                  <h2>Matchup tips</h2>
                  <ul>
                    {build.notes.map((n) => (
                      <li key={n}>{n}</li>
                    ))}
                  </ul>
                </section>
              )}

              {enemyCount === 0 && (
                <p className="muted small center-text">
                  This is a generic build — add enemies to tailor it.
                </p>
              )}
            </>
          )}
        </div>
      </main>

      <footer className="muted small">
        Data from Riot Data Dragon and the League Live Client Data API. Tolleagle isn't endorsed by Riot Games. Recommendations are heuristic — use your judgment.
      </footer>
    </div>
  );
}

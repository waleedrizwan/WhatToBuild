import { LANE_OPPONENT_WEIGHT, type TeamAnalysis } from "../engine/analyze";
import type { Need } from "../data/items";

const THREAT_LABELS: [Need, string][] = [
  ["antiheal", "Healing"],
  ["antiburst", "Burst"],
  ["antiCC", "Crowd control"],
  ["armorpen", "Tanks"],
  ["antishield", "Shields"],
  ["antiAuto", "Auto-attackers"],
  ["antiCrit", "Crit"],
  ["antiPoke", "Poke"],
];

export function TeamAnalysisPanel({ analysis }: { analysis: TeamAnalysis }) {
  const ad = Math.round(analysis.adShare * 100);
  const threats = THREAT_LABELS.map(([need, label]) => ({ need, label, level: analysis.needs[need] }))
    .filter((t) => t.level >= 0.3)
    .sort((a, b) => b.level - a.level);

  return (
    <section className="panel analysis">
      <h2>Enemy threat read</h2>
      <div className="dmg-split" aria-label={`${ad}% physical, ${100 - ad}% magic`}>
        <div className="dmg-ad" style={{ width: `${ad}%` }}>
          {ad >= 12 && `${ad}% AD`}
        </div>
        <div className="dmg-ap" style={{ width: `${100 - ad}%` }}>
          {100 - ad >= 12 && `${100 - ad}% AP`}
        </div>
      </div>
      {analysis.laneOpponent && (
        <p className="muted small">
          Lane opponent <strong>{analysis.laneOpponent.champion.name}</strong> is weighted {LANE_OPPONENT_WEIGHT}× — you'll fight them most early.
        </p>
      )}
      {analysis.intel.length > 0 && (
        <div className="intel">
          <h3>From their items</h3>
          <ul>
            {analysis.intel.map((n) => (
              <li key={n}>{n}</li>
            ))}
          </ul>
        </div>
      )}
      <div className="threats">
        {threats.length === 0 && <span className="muted small">No standout threats.</span>}
        {threats.map((t) => (
          <div key={t.need} className="threat" title={analysis.sources[t.need].join(", ")}>
            <div className="threat-head">
              <span>{t.label}</span>
              <span className="threat-who">{analysis.sources[t.need].slice(0, 3).join(", ")}</span>
            </div>
            <div className="meter">
              <div style={{ width: `${Math.round(t.level * 100)}%` }} className={t.level >= 0.7 ? "high" : ""} />
            </div>
          </div>
        ))}
      </div>
    </section>
  );
}

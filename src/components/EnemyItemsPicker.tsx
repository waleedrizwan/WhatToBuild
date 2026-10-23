import { useEffect, useMemo, useRef, useState } from "react";
import { describeItem, type Item } from "../data/ddragon";

const MAX_ITEMS = 6;

interface Props {
  /** All items that can be tracked, sorted by name. */
  catalog: Item[];
  value: string[];
  onChange: (names: string[]) => void;
  championName: string;
}

export function EnemyItemsPicker({ catalog, value, onChange, championName }: Props) {
  const [open, setOpen] = useState(false);
  const [query, setQuery] = useState("");
  const rootRef = useRef<HTMLDivElement>(null);
  const byName = useMemo(() => new Map(catalog.map((i) => [i.name, i])), [catalog]);
  const owned = value.map((n) => byName.get(n)).filter((i): i is Item => !!i);

  const matches = useMemo(() => {
    const q = query.trim().toLowerCase();
    return catalog.filter((i) => !value.includes(i.name) && (!q || i.name.toLowerCase().includes(q)));
  }, [catalog, value, query]);

  useEffect(() => {
    if (!open) return;
    const onDown = (e: MouseEvent) => {
      if (!rootRef.current?.contains(e.target as Node)) setOpen(false);
    };
    document.addEventListener("mousedown", onDown);
    return () => document.removeEventListener("mousedown", onDown);
  }, [open]);

  const add = (item: Item) => {
    // One pair of boots at a time.
    const next = item.isBoots ? value.filter((n) => !byName.get(n)?.isBoots) : value;
    onChange([...next, item.name].slice(0, MAX_ITEMS));
    setQuery("");
    if (next.length + 1 >= MAX_ITEMS) setOpen(false);
  };

  return (
    <div className="enemy-items" ref={rootRef}>
      {owned.map((item) => (
        <button
          key={item.name}
          type="button"
          className="enemy-item"
          title={`${item.name}: click to remove`}
          onClick={() => onChange(value.filter((n) => n !== item.name))}
        >
          <img src={item.image} alt={item.name} />
        </button>
      ))}
      {owned.length < MAX_ITEMS && (
        <button
          type="button"
          className="enemy-item add"
          title={`Add an item ${championName} has built`}
          onClick={() => setOpen((o) => !o)}
        >
          +
        </button>
      )}
      {owned.length === 0 && !open && <span className="enemy-items-hint">their items</span>}
      {open && (
        <div className="item-popover">
          <input
            autoFocus
            className="picker-input"
            placeholder={`${championName}'s items…`}
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            onKeyDown={(e) => {
              if (e.key === "Enter" && matches[0]) add(matches[0]);
              if (e.key === "Escape") setOpen(false);
            }}
          />
          <div className="item-grid">
            {matches.map((item) => (
              <button
                key={item.name}
                type="button"
                className="item-grid-cell"
                title={`${item.name} (${item.gold}g)\n\n${describeItem(item)}`}
                onClick={() => add(item)}
              >
                <img src={item.image} alt="" loading="lazy" />
                <span>{item.name}</span>
              </button>
            ))}
            {matches.length === 0 && <span className="muted small">No item found</span>}
          </div>
        </div>
      )}
    </div>
  );
}

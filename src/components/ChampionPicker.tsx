import { useEffect, useMemo, useRef, useState } from "react";
import type { Champion } from "../data/ddragon";

interface Props {
  champions: Champion[];
  value: string | null;
  onChange: (id: string | null) => void;
  placeholder: string;
  /** Champions picked elsewhere; shown disabled. */
  taken?: Set<string>;
}

export function ChampionPicker({ champions, value, onChange, placeholder, taken }: Props) {
  const [open, setOpen] = useState(false);
  const [query, setQuery] = useState("");
  const [active, setActive] = useState(0);
  const rootRef = useRef<HTMLDivElement>(null);
  const inputRef = useRef<HTMLInputElement>(null);
  const selected = champions.find((c) => c.id === value);

  const matches = useMemo(() => {
    const q = query.trim().toLowerCase().replace(/[^a-z]/g, "");
    if (!q) return champions;
    const norm = (s: string) => s.toLowerCase().replace(/[^a-z]/g, "");
    return champions
      .filter((c) => norm(c.name).includes(q) || norm(c.id).includes(q))
      .sort((a, b) => Number(!norm(a.name).startsWith(q)) - Number(!norm(b.name).startsWith(q)));
  }, [champions, query]);

  useEffect(() => setActive(0), [query]);

  useEffect(() => {
    if (!open) return;
    const onDown = (e: MouseEvent) => {
      if (!rootRef.current?.contains(e.target as Node)) setOpen(false);
    };
    document.addEventListener("mousedown", onDown);
    return () => document.removeEventListener("mousedown", onDown);
  }, [open]);

  const choose = (c: Champion) => {
    if (taken?.has(c.id) && c.id !== value) return;
    onChange(c.id);
    setQuery("");
    setOpen(false);
  };

  const onKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === "ArrowDown") {
      e.preventDefault();
      setActive((a) => Math.min(a + 1, matches.length - 1));
    } else if (e.key === "ArrowUp") {
      e.preventDefault();
      setActive((a) => Math.max(a - 1, 0));
    } else if (e.key === "Enter" && matches[active]) {
      e.preventDefault();
      choose(matches[active]);
    } else if (e.key === "Escape") {
      setOpen(false);
    }
  };

  return (
    <div className="picker" ref={rootRef}>
      {selected && !open ? (
        <button
          type="button"
          className="picker-selected"
          onClick={() => {
            setOpen(true);
            setTimeout(() => inputRef.current?.focus());
          }}
        >
          <img src={selected.image} alt="" />
          <span className="picker-name">{selected.name}</span>
          <span
            className="picker-clear"
            role="button"
            aria-label={`Clear ${selected.name}`}
            onClick={(e) => {
              e.stopPropagation();
              onChange(null);
            }}
          >
            ×
          </span>
        </button>
      ) : (
        <input
          ref={inputRef}
          className="picker-input"
          placeholder={placeholder}
          value={query}
          onFocus={() => setOpen(true)}
          onChange={(e) => {
            setQuery(e.target.value);
            setOpen(true);
          }}
          onKeyDown={onKeyDown}
        />
      )}
      {open && (
        <ul className="picker-list" role="listbox">
          {matches.length === 0 && <li className="picker-empty">No champion found</li>}
          {matches.slice(0, 60).map((c, i) => {
            const disabled = taken?.has(c.id) && c.id !== value;
            return (
              <li
                key={c.id}
                role="option"
                aria-selected={i === active}
                className={`picker-option${i === active ? " active" : ""}${disabled ? " disabled" : ""}`}
                onMouseEnter={() => setActive(i)}
                onMouseDown={(e) => {
                  e.preventDefault();
                  choose(c);
                }}
              >
                <img src={c.image} alt="" loading="lazy" />
                <span>{c.name}</span>
                <small>{c.tags.join(" · ")}</small>
              </li>
            );
          })}
        </ul>
      )}
    </div>
  );
}

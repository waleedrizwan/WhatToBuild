import { describeItem, type Item } from "../data/ddragon";

interface Props {
  item: Item;
  index?: number;
  reasons?: string[];
  compact?: boolean;
  /** "owned": already in inventory; "next": the item to buy next. */
  status?: "owned" | "next";
}

export function ItemCard({ item, index, reasons, compact, status }: Props) {
  return (
    <div className={`item-card${compact ? " compact" : ""}${status ? ` ${status}` : ""}`}>
      <div className="item-icon" tabIndex={0}>
        <img src={item.image} alt={item.name} />
        {index !== undefined && <span className="item-index">{index}</span>}
        <div className="item-tooltip" role="tooltip">
          <strong>{item.name}</strong>
          <span className="gold">{item.gold}g</span>
          <p>{describeItem(item)}</p>
        </div>
      </div>
      {!compact && (
        <div className="item-body">
          <div className="item-title">
            {item.name} <span className="gold">{item.gold}g</span>
            {status === "next" && <span className="badge next">Buy next</span>}
            {status === "owned" && <span className="badge">Owned</span>}
          </div>
          {reasons && (
            <ul className="item-reasons">
              {reasons.map((r) => (
                <li key={r}>{r}</li>
              ))}
            </ul>
          )}
        </div>
      )}
    </div>
  );
}

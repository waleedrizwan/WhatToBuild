import { useEffect, useRef, useState } from "react";
import type { GameData } from "../data/ddragon";
import { fetchLiveGame, type LiveGame, type LiveStatus } from "./liveClient";

const POLL_MS = 5000;

/** Polls the Live Client Data API while `enabled`, calling `onGame` on every successful read. */
export function useLiveGame(data: GameData | null, enabled: boolean, onGame: (game: LiveGame) => void): LiveStatus {
  const [status, setStatus] = useState<LiveStatus>({ state: "off" });
  const onGameRef = useRef(onGame);
  onGameRef.current = onGame;

  useEffect(() => {
    if (!enabled || !data) {
      setStatus({ state: "off" });
      return;
    }
    setStatus({ state: "waiting", message: "Looking for a game…" });
    const controller = new AbortController();
    let timer: ReturnType<typeof setTimeout>;

    const poll = async () => {
      const next = await fetchLiveGame(data, controller.signal);
      if (controller.signal.aborted) return;
      setStatus(next);
      if (next.state === "connected") onGameRef.current(next.game);
      timer = setTimeout(poll, POLL_MS);
    };
    poll();

    return () => {
      controller.abort();
      clearTimeout(timer);
    };
  }, [data, enabled]);

  return status;
}

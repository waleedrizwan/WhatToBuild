# Tolleagle

```
   YOU PICK (or Live mode fills in)            RIOT DATA (live, no API key)
 ┌──────────────────────────────────┐      ┌───────────────────────────────┐
 │ Your champion + role + items     │      │ Data Dragon CDN               │
 │ Enemy team + their items         │      │  champions · items · icons    │
 │ Lane opponent ◉                  │      └───────────────┬───────────────┘
 └───────────────┬──────────────────┘                      │
        ▲        │                                         │
        │        ▼                                         ▼
 ┌──────┴────────────────┐   ┌─────────────────────────────────────────────┐
 │ LIVE GAME (optional)  │   │ 1. ANALYZE ENEMIES          engine/analyze  │
 │ League client :2999   │   │   each enemy → damage type + threat traits  │
 │   ↓ every 5s          │   │   (healing, burst, CC, tanks, crit, poke…)  │
 │ Vite proxy            │   │   + their items (armor stacking, AP Kai'Sa, │
 │ /liveclientdata/*     │   │   lifesteal, shields…)   engine/enemyItems  │
 └───────────────────────┘   │   lane opponent counts 1.6×                 │
                             │   ⇒ NEEDS e.g. armor 0.8, anti-heal 0.9     │
                             └──────────────────────┬──────────────────────┘
                                                    ▼
                             ┌─────────────────────────────────────────────┐
                             │ 2. SCORE ITEMS            engine/recommend  │
                             │   score = fit for your build style          │
                             │         + how well it answers each NEED     │
                             │   early slots → your core damage items      │
                             │   later slots → counter the enemy team      │
                             │   one per unique passive (Lifeline, Hydra…) │
                             │   items you own are locked in first         │
                             └──────────────────────┬──────────────────────┘
                                                    ▼
                             ┌─────────────────────────────────────────────┐
                             │ 3. YOUR BUILD                               │
                             │   Start · Boots · Items 1-5                 │
                             │   marked Owned / BUY NEXT, each with "why"  │
                             │   + situational swaps · matchup tips        │
                             └─────────────────────────────────────────────┘
```


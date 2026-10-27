# Riot Developer Portal submission: Tolleagle

This is a draft for registering Tolleagle at <https://developer.riotgames.com> (sign in with your Riot account, then **Register Product**). Copy the sections into the matching form fields, and attach the screenshots listed at the end.

---

## Product name

Tolleagle

## Short description

A desktop companion for League of Legends. It suggests item options that fit your matchup, using the enemy team's champions and the items they've bought.

## Full description

Tolleagle is a free, open-source desktop app for Windows and macOS that helps players adapt their item builds to the game they're in.

While a match is running, it reads the local **Live Client Data API** (`https://127.0.0.1:2999/liveclientdata/allgamedata`): the player's champion, position and items, and each enemy's champion and items. These are the same values the player can see on the in-game scoreboard. It then looks at the enemy team's overall profile and suggests items that answer it. Examples: armor against a physical-damage-heavy team, Grievous Wounds against healers, penetration against tanks that are stacking resistances.

The app shows:

- **Options for your next item:** the three best-scoring choices for the next open slot, each with the reason it was suggested. The player picks from these; the app never buys anything or makes the choice for them.
- **Enemy threat read:** the physical/magic damage split, plus notes taken from enemy items (for example "Malphite has 175 bonus armor").
- **A full example build order and situational swaps,** with explanations.
- **A small overlay** in a separate, always-on-top, click-through OS window. It shows the same next-item options while the game is running. It does not inject into, hook or modify the game client in any way.

Champion, item and icon data comes from **Data Dragon**. The app doesn't use any key-based Riot API endpoints.

## APIs and data used

| Source | Use |
| --- | --- |
| Live Client Data API (`/liveclientdata/allgamedata`) | Player's own champion, position and items; the enemy team's champions and items. Polled every 5 seconds during a game. |
| Data Dragon (`ddragon.leagueoflegends.com`) | Champion and item metadata and icons for the current patch. |

No Riot API key is used. No match history, summoner lookups or account data are requested, and no data leaves the player's computer.

## How it follows the Game Integrity policy

- **No information that isn't already in the game client.** Every input is visible to the player in game: their own items, plus enemy champions and items on the TAB scoreboard. There are no cooldown, summoner-spell, jungle or ultimate timers. It doesn't reveal hidden players or vision, and it doesn't track anything else that's obscured.
- **Highlights decisions instead of making them.** Tolleagle presents several item options with explanations. It doesn't act on the player's behalf, automate purchases or inputs, or present a single mandatory choice.
- **Increases build diversity.** Suggestions change with each matchup (enemy damage types, healing, tanks, crit, crowd control), so it pushes players away from a single fixed build.
- **No win rates, MMR, ELO or ranking substitutes,** and no Arena augment or item win rates.
- **No de-anonymization.** It only uses the champion names the game itself shows. Player names from the Live Client feed are used only to find which entry is the local player, and are never displayed or stored.
- **No interaction with the game process.** It doesn't read memory, inject code or hook graphics. The overlay is a normal top-most window that clicks pass through.

## Monetization

None. It's free and open source.

## Distribution

Source code on GitHub, with prebuilt downloads for Windows and macOS. *(Fill in the repository URL before submitting.)*

## Screenshots to attach

1. Main window during a game: next-item options, threat read and build order.
2. The overlay over the game in Borderless mode.
3. The "Waiting for a game" screen.

---

### Before you submit

- Make sure the product page says Tolleagle isn't endorsed by Riot Games. The app already shows this in its footer.
- Riot may ask for changes, especially to the overlay's contents. Changes shipped after approval also have to go through review again.

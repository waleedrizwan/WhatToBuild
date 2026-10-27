#pragma once
// The Tolleagle window. Everything comes from the running game: champions, role, both teams'
// items. There is nothing to enter; between games it waits for the next match.

#include <imgui.h>

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "data/champions.h"
#include "data/ddragon.h"
#include "engine/analyze.h"
#include "engine/recommend.h"
#include "live/live_client.h"
#include "ui/icons.h"

struct Fonts {
  ImFont* body = nullptr;
  ImFont* small = nullptr;
  ImFont* bold = nullptr;
  ImFont* heading = nullptr;
  ImFont* title = nullptr;
  ImFont* big = nullptr;
};

/**
 * Loads fonts and applies the theme. `contentScale` is the monitor's DPI scale and
 * `framebufferScale` the pixels per window unit (2 on a Retina Mac, 1 on Windows).
 * Returns the UI scale to apply to hard-coded sizes.
 */
float setupStyle(Fonts& fonts, float contentScale, float framebufferScale);

class App {
 public:
  /** `wake` asks the main loop for another frame; it may be called from any thread. */
  App(Fonts fonts, float uiScale, std::function<void()> wake);
  App(const App&) = delete;
  App& operator=(const App&) = delete;

  void frame();

 private:
  enum class CardStatus { None, Owned, Next };

  /** The live game resolved against Data Dragon. */
  struct Game {
    const Champion* you = nullptr;
    Role role = Role::Top;
    std::vector<const Item*> owned;
    std::vector<Enemy> enemies;
    std::string laneOpponentId;
  };
  Game resolve(const LiveGame& live) const;

  void drawHeader();
  void drawWaiting(const LiveStatus& status);
  void drawStatusLine(const LiveStatus& status);
  void drawYouPanel(const Game& game, const LiveStatus& status);
  void drawEnemyPanel(const Game& game);
  void drawResults(const Game& game, const TeamAnalysis& analysis);
  void drawAnalysis(const TeamAnalysis& analysis);

  void itemCard(const Item& item, const std::vector<std::string>* reasons, int index, CardStatus status);
  void itemIcon(const Item& item, float size);
  void itemRow(const std::vector<const Item*>& items, float size, const char* empty);
  void itemTooltip(const Item& item);

  void beginPanel(const char* id, const char* title, ImU32 border = 0);
  void endPanel();
  void heading(const char* text);
  void mutedWrapped(const std::string& text, bool small = true);
  void bullets(const std::vector<std::string>& lines, ImFont* font);
  void badge(const char* text, ImU32 bg, ImU32 fg);
  void drawIcon(ImDrawList* dl, const std::string& url, ImVec2 min, ImVec2 max, float rounding);
  void textAt(ImDrawList* dl, ImFont* font, ImVec2 pos, ImU32 col, const std::string& text, float clipRight = 0);
  ImVec2 measure(ImFont* font, const std::string& text) const;
  const std::string& description(const Item& item);
  float dp(float v) const { return v * uiScale_; }

  Fonts fonts_;
  float uiScale_;
  std::function<void()> wake_;

  std::future<std::unique_ptr<GameData>> loading_;
  std::unique_ptr<GameData> data_;
  std::string error_;
  std::unordered_map<std::string, std::string> descriptions_;

  IconCache icons_;
  LivePoller live_;  // after data_, so it stops before the data it reads goes away
};

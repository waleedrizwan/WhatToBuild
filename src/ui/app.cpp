#include "ui/app.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdlib>
#include <filesystem>

namespace {

namespace color {
constexpr unsigned bg = 0x0a0e14;
constexpr unsigned panel = 0x111823;
constexpr unsigned panel2 = 0x172131;
constexpr unsigned border = 0x23324a;
constexpr unsigned text = 0xe6e9ef;
constexpr unsigned muted = 0x8b97ab;
constexpr unsigned gold = 0xc8aa6e;
constexpr unsigned goldBright = 0xf0e6d2;
constexpr unsigned ad = 0xd4553c;
constexpr unsigned ap = 0x4f8ee8;
constexpr unsigned accent = 0x0ac8b9;
}  // namespace color

ImVec4 rgba(unsigned hex, float a = 1) {
  return {((hex >> 16) & 255) / 255.f, ((hex >> 8) & 255) / 255.f, (hex & 255) / 255.f, a};
}
ImU32 u32(unsigned hex, float a = 1) { return ImGui::ColorConvertFloat4ToU32(rgba(hex, a)); }

ImVec2 operator+(ImVec2 a, ImVec2 b) { return {a.x + b.x, a.y + b.y}; }

// --- Fonts -----------------------------------------------------------------

const std::vector<const char*> SANS = {
    "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
    "C:\\Windows\\Fonts\\segoeui.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
};
const std::vector<const char*> SANS_BOLD = {
    "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
    "C:\\Windows\\Fonts\\segoeuib.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
};
const std::vector<const char*> SERIF_BOLD = {
    "/System/Library/Fonts/Supplemental/Georgia Bold.ttf",
    "C:\\Windows\\Fonts\\georgiab.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSerif-Bold.ttf",
};
// Merged into every font for ◉ ◎ and friends where the main font lacks them.
const std::vector<const char*> SYMBOLS = {
    "/System/Library/Fonts/Supplemental/Arial Unicode.ttf",
    "C:\\Windows\\Fonts\\seguisym.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
};

const char* firstExisting(const std::vector<const char*>& paths) {
  for (const char* p : paths) {
    std::error_code ec;
    if (std::filesystem::exists(p, ec)) return p;
  }
  return nullptr;
}

ImFont* addFont(const std::vector<const char*>& paths, float px) {
  // Latin-1, general punctuation (— … ·), arrows, geometric shapes (◉ ◎), ×.
  static const ImWchar ranges[] = {0x0020, 0x00FF, 0x2010, 0x2027, 0x2190, 0x2193, 0x25A0, 0x25FF, 0};
  ImGuiIO& io = ImGui::GetIO();
  const char* path = firstExisting(paths);
  ImFont* font = nullptr;
  if (path) {
    ImFontConfig cfg;
    cfg.OversampleH = 2;
    font = io.Fonts->AddFontFromFileTTF(path, px, &cfg, ranges);
  }
  if (!font) {
    ImFontConfig cfg;
    cfg.SizePixels = px;
    font = io.Fonts->AddFontDefault(&cfg);
  }
  if (const char* sym = firstExisting(SYMBOLS); sym && (!path || std::string(sym) != path)) {
    ImFontConfig cfg;
    cfg.MergeMode = true;
    io.Fonts->AddFontFromFileTTF(sym, px, &cfg, ranges);
  }
  return font;
}

std::string formatGameTime(double seconds) {
  int m = static_cast<int>(seconds / 60);
  int sec = static_cast<int>(std::fmod(seconds, 60));
  return std::to_string(m) + ":" + (sec < 10 ? "0" : "") + std::to_string(sec);
}

std::string withCommas(int n) {
  std::string digits = std::to_string(std::abs(n));
  for (int i = static_cast<int>(digits.size()) - 3; i > 0; i -= 3) digits.insert(i, ",");
  return (n < 0 ? "-" : "") + digits;
}

std::string join(const std::vector<std::string>& xs, std::size_t limit, const char* sep = ", ") {
  std::string out;
  for (std::size_t i = 0; i < xs.size() && i < limit; i++) out += (i ? sep : "") + xs[i];
  return out;
}

const std::array<std::pair<Need, const char*>, 8> THREAT_LABELS = {{
    {Need::Antiheal, "Healing"},
    {Need::Antiburst, "Burst"},
    {Need::AntiCC, "Crowd control"},
    {Need::Armorpen, "Tanks"},
    {Need::Antishield, "Shields"},
    {Need::AntiAuto, "Auto-attackers"},
    {Need::AntiCrit, "Crit"},
    {Need::AntiPoke, "Poke"},
}};

}  // namespace

float setupStyle(Fonts& fonts, float contentScale, float framebufferScale) {
  ImGuiIO& io = ImGui::GetIO();
  // Rasterize at physical pixel size, then draw at logical size, so text is sharp on HiDPI.
  float px = contentScale;
  fonts.body = addFont(SANS, 15 * px);
  fonts.small = addFont(SANS, 13 * px);
  fonts.bold = addFont(SANS_BOLD, 15 * px);
  fonts.heading = addFont(SERIF_BOLD, 15 * px);
  fonts.title = addFont(SERIF_BOLD, 30 * px);
  fonts.big = addFont(SANS_BOLD, 22 * px);
  io.FontDefault = fonts.body;
  io.FontGlobalScale = 1 / framebufferScale;

  ImGuiStyle& s = ImGui::GetStyle();
  s.WindowPadding = {16, 16};
  s.FramePadding = {8, 6};
  s.ItemSpacing = {8, 8};
  s.ItemInnerSpacing = {6, 6};
  s.ChildRounding = 10;
  s.FrameRounding = 6;
  s.PopupRounding = 8;
  s.GrabRounding = 6;
  s.ScrollbarRounding = 6;
  s.ScrollbarSize = 10;
  s.WindowBorderSize = 0;
  s.ChildBorderSize = 1;
  s.PopupBorderSize = 1;
  s.FrameBorderSize = 1;

  ImVec4* c = s.Colors;
  c[ImGuiCol_Text] = rgba(color::text);
  c[ImGuiCol_TextDisabled] = rgba(color::muted);
  c[ImGuiCol_WindowBg] = rgba(color::bg);
  c[ImGuiCol_ChildBg] = rgba(0, 0);
  c[ImGuiCol_PopupBg] = rgba(color::panel2);
  c[ImGuiCol_Border] = rgba(color::border);
  c[ImGuiCol_FrameBg] = rgba(color::bg);
  c[ImGuiCol_FrameBgHovered] = rgba(color::panel2);
  c[ImGuiCol_FrameBgActive] = rgba(color::panel2);
  c[ImGuiCol_Button] = rgba(color::bg);
  c[ImGuiCol_ButtonHovered] = rgba(color::panel2);
  c[ImGuiCol_ButtonActive] = rgba(color::border);
  c[ImGuiCol_Header] = rgba(color::border);
  c[ImGuiCol_HeaderHovered] = rgba(color::border);
  c[ImGuiCol_HeaderActive] = rgba(color::border);
  c[ImGuiCol_CheckMark] = rgba(color::accent);
  c[ImGuiCol_ScrollbarBg] = rgba(0, 0);
  c[ImGuiCol_ScrollbarGrab] = rgba(color::border);
  c[ImGuiCol_ScrollbarGrabHovered] = rgba(color::muted);
  c[ImGuiCol_ScrollbarGrabActive] = rgba(color::muted);
  c[ImGuiCol_Separator] = rgba(color::border);
  c[ImGuiCol_TextSelectedBg] = rgba(color::accent, 0.35f);

  float uiScale = contentScale / framebufferScale;
  s.ScaleAllSizes(uiScale);
  return uiScale;
}

App::App(Fonts fonts, float uiScale, std::function<void()> wake)
    : fonts_(fonts), uiScale_(uiScale), wake_(std::move(wake)), icons_(wake_), live_(wake_) {
  loading_ = std::async(std::launch::async, [wake = wake_] {
    struct WakeOnExit {
      std::function<void()> wake;
      ~WakeOnExit() { wake(); }
    } guard{wake};
    return loadGameData();
  });
}

void App::frame() {
  icons_.pump();
  if (loading_.valid() && loading_.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
    try {
      data_ = loading_.get();
      live_.start(*data_);  // watch for a game from now on
    } catch (const std::exception& e) {
      error_ = e.what();
    }
  }

  const ImGuiViewport* vp = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(vp->WorkPos);
  ImGui::SetNextWindowSize(vp->WorkSize);
  ImGui::Begin("Tolleagle", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar);
  if (!error_.empty() || !data_) {
    std::string msg = !error_.empty() ? "Couldn't load champion data from Riot's Data Dragon: " + error_
                                      : "Loading the latest patch data…";
    ImVec2 size = ImGui::CalcTextSize(msg.c_str(), nullptr, false, dp(600));
    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGui::SetCursorPos({(avail.x - size.x) / 2, (avail.y - size.y) / 2});
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + dp(600));
    ImGui::TextColored(rgba(error_.empty() ? color::muted : color::text), "%s", msg.c_str());
    ImGui::PopTextWrapPos();
    ImGui::End();
    return;
  }

  refresh();
  const LiveStatus& status = status_;
  float avail = ImGui::GetContentRegionAvail().x;
  float pageWidth = std::min(avail, dp(1200));
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - pageWidth) / 2);
  ImGui::BeginChild("page", {pageWidth, 0});
  drawHeader();

  if (!snapshot_) {
    drawWaiting(status);
  } else {
    ImGui::BeginChild("game", {dp(340), 0});
    drawYouPanel(snapshot_->game, status);
    drawEnemyPanel(snapshot_->game);
    ImGui::EndChild();
    ImGui::SameLine(0, dp(20));
    ImGui::BeginChild("results", {0, 0});
    drawResults(*snapshot_);
    ImGui::PushTextWrapPos(0);
    ImGui::PushFont(fonts_.small);
    ImGui::TextDisabled(
        "%s", "Data from Riot Data Dragon and the League Live Client Data API. Tolleagle isn't endorsed by Riot Games. "
              "Recommendations are heuristic — use your judgment.");
    ImGui::PopFont();
    ImGui::PopTextWrapPos();
    ImGui::EndChild();
  }
  ImGui::EndChild();
  ImGui::End();
}

void App::refresh() {
  status_ = live_.status();
  snapshot_.reset();
  if (status_.state != LiveStatus::State::Connected) return;
  Game game = resolve(status_.game);
  if (!game.you) return;
  Snapshot& snap = snapshot_.emplace();
  snap.analysis = analyzeEnemies(game.enemies, game.laneOpponentId);
  snap.build = recommendBuild(*data_, *game.you, archetypeFor(*game.you), game.role, snap.analysis, game.owned);
  snap.game = std::move(game);
  int ownedGold = 0;
  if (snap.build.boots && snap.build.boots->owned) ownedGold += snap.build.boots->item->gold;
  for (std::size_t i = 0; i < snap.build.core.size(); i++) {
    const Pick& p = snap.build.core[i];
    if (p.owned) ownedGold += p.item->gold;
    else if (snap.nextCore < 0) snap.nextCore = static_cast<int>(i);
  }
  snap.goldLeft = snap.build.totalGold - ownedGold;
}

bool App::wantsOverlay() const { return overlayEnabled_ && snapshot_.has_value(); }

void App::drawOverlay() {
  if (!snapshot_) return;
  const Snapshot& snap = *snapshot_;
  const Build& build = snap.build;
  float width = dp(OVERLAY_WIDTH);
  ImGui::PushTextWrapPos(width);
  ImDrawList* dl = ImGui::GetWindowDrawList();

  // Title row: app name + game clock, gold left on the right.
  ImGui::PushFont(fonts_.heading);
  ImGui::TextColored(rgba(color::gold), "TOLLEAGLE");
  ImGui::PopFont();
  ImGui::SameLine();
  ImGui::PushFont(fonts_.small);
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + dp(1));
  ImGui::TextDisabled("%s", formatGameTime(status_.game.gameTime).c_str());
  std::string gold = withCommas(snap.goldLeft) + "g left";
  ImGui::SameLine(width - measure(fonts_.small, gold).x);
  ImGui::TextColored(rgba(color::gold), "%s", gold.c_str());
  ImGui::PopFont();
  ImGui::Dummy({width, dp(2)});

  ImGui::PushFont(fonts_.small);
  ImGui::TextDisabled(build.nextOptions.empty() ? "Your build is complete." : "Options for your next item");
  ImGui::PopFont();
  for (const Pick& p : build.nextOptions) {
    float s = dp(30);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    drawIcon(dl, p.item->image, pos, {pos.x + s, pos.y + s}, dp(4));
    ImGui::Dummy({s, s});
    ImGui::SameLine(0, dp(8));
    ImGui::BeginGroup();
    ImGui::PushFont(fonts_.bold);
    ImGui::TextUnformatted(p.item->name.c_str());
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::TextColored(rgba(color::gold), "%dg", p.item->gold);
    if (!p.reasons.empty()) {
      ImGui::PushFont(fonts_.small);
      ImGui::TextDisabled("%s", p.reasons.front().c_str());
      ImGui::PopFont();
    }
    ImGui::EndGroup();
  }

  ImGui::PushFont(fonts_.small);
  if (build.boots && !build.boots->owned)
    ImGui::TextDisabled("Boots: %s (%dg)", build.boots->item->name.c_str(), build.boots->item->gold);
  std::vector<std::string> threats;
  for (const auto& [need, label] : THREAT_LABELS)
    if (snap.analysis.need(need) >= 0.6) threats.push_back(label);
  if (!threats.empty()) ImGui::TextDisabled("Watch for: %s", join(threats, 3).c_str());
  ImGui::PopFont();
  ImGui::PopTextWrapPos();
}

App::Game App::resolve(const LiveGame& live) const {
  Game g;
  g.you = data_->championById(live.you);
  if (!g.you) return g;
  g.role = live.role.value_or(defaultRoleFor(*g.you));
  for (const std::string& n : live.yourItems)
    if (const Item* item = data_->itemByName(n)) g.owned.push_back(item);
  for (std::size_t i = 0; i < live.enemies.size(); i++) {
    const Champion* c = data_->championById(live.enemies[i].id);
    if (!c) continue;
    Enemy e{c, {}};
    for (const std::string& n : live.enemies[i].items)
      if (const Item* item = data_->itemByName(n)) e.items.push_back(item);
    if (static_cast<int>(i) == live.laneIndex) g.laneOpponentId = c->id;
    g.enemies.push_back(std::move(e));
  }
  return g;
}

void App::drawHeader() {
  ImGui::PushFont(fonts_.title);
  ImGui::TextColored(rgba(color::goldBright), "Tolleagle");
  ImGui::PopFont();

  std::string tagline = "Matchup-aware item builds · patch " + data_->version;
  float afterTitle = ImGui::GetCursorPosY();
  float right = ImGui::GetWindowContentRegionMax().x;
  ImGui::SetCursorPos({right - ImGui::CalcTextSize(tagline.c_str()).x,
                       afterTitle - ImGui::GetStyle().ItemSpacing.y - ImGui::GetTextLineHeight() - dp(4)});
  ImGui::TextDisabled("%s", tagline.c_str());
  ImGui::SetCursorPosY(afterTitle);

  ImVec2 p = ImGui::GetCursorScreenPos();
  ImGui::GetWindowDrawList()->AddLine({p.x, p.y + dp(4)}, {p.x + ImGui::GetContentRegionAvail().x, p.y + dp(4)},
                                      u32(color::border));
  ImGui::Dummy({0, dp(14)});
}

void App::drawStatusLine(const LiveStatus& status) {
  bool on = status.state == LiveStatus::State::Connected;
  std::string text = on ? "Connected · " + formatGameTime(status.game.gameTime) +
                              (status.game.gameMode.empty() ? "" : " · " + status.game.gameMode) + " · updates every 5s"
                        : status.message;
  ImGui::PushFont(fonts_.small);
  ImVec2 p = ImGui::GetCursorScreenPos();
  float r = dp(4);
  ImGui::GetWindowDrawList()->AddCircleFilled({p.x + r, p.y + ImGui::GetTextLineHeight() / 2}, r,
                                              u32(on ? color::accent : color::muted));
  ImGui::Dummy({r * 2, ImGui::GetTextLineHeight()});
  ImGui::SameLine(0, dp(6));
  ImGui::PushStyleColor(ImGuiCol_Text, rgba(on ? color::text : color::muted));
  ImGui::PushTextWrapPos(0);
  ImGui::TextUnformatted(text.c_str());
  ImGui::PopTextWrapPos();
  ImGui::PopStyleColor();
  ImGui::PopFont();
}

void App::drawWaiting(const LiveStatus& status) {
  float width = std::min(ImGui::GetContentRegionAvail().x, dp(560));
  ImGui::Dummy({0, dp(60)});
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - width) / 2);
  ImGui::BeginChild("waiting-column", {width, 0});
  beginPanel("waiting", "Waiting for a game");
  drawStatusLine(status);
  ImGui::Spacing();
  mutedWrapped(
      "Start a match in League of Legends. Tolleagle reads your champion, role, both teams and everyone's items "
      "straight from the game client and builds around them, updating as items are bought. There's nothing to "
      "enter.",
      false);
  if (std::string url = liveClientUrl(); url != "https://127.0.0.1:2999")
    mutedWrapped("Reading from " + url + " (LIVE_CLIENT_URL).");
  endPanel();
  ImGui::EndChild();
}

void App::drawYouPanel(const Game& game, const LiveStatus& status) {
  beginPanel("you", nullptr, u32(color::accent, 0.6f));
  drawStatusLine(status);
  ImGui::Spacing();

  float s = dp(48);
  ImVec2 p = ImGui::GetCursorScreenPos();
  drawIcon(ImGui::GetWindowDrawList(), game.you->image, p, {p.x + s, p.y + s}, dp(6));
  ImGui::Dummy({s, s});
  ImGui::SameLine(0, dp(12));
  ImGui::BeginGroup();
  ImGui::PushFont(fonts_.big);
  ImGui::TextColored(rgba(color::goldBright), "%s", game.you->name.c_str());
  ImGui::PopFont();
  ImGui::PushFont(fonts_.small);
  ImGui::TextDisabled("%s · %s", std::string(roleLabel(game.role)).c_str(),
                      std::string(archetypeLabel(archetypeFor(*game.you))).c_str());
  ImGui::PopFont();
  ImGui::EndGroup();

  ImGui::Spacing();
  ImGui::PushFont(fonts_.small);
  ImGui::TextDisabled("Your items");
  ImGui::PopFont();
  itemRow(game.owned, dp(30), "Nothing bought yet");
  ImGui::Spacing();
  ImGui::PushFont(fonts_.small);
  ImGui::Checkbox("Show overlay in game", &overlayEnabled_);
  ImGui::PopFont();
  endPanel();
}

void App::drawEnemyPanel(const Game& game) {
  beginPanel("enemies", "Enemy team");
  if (game.enemies.empty()) mutedWrapped("No enemies reported yet.");
  for (std::size_t i = 0; i < game.enemies.size(); i++) {
    const Enemy& e = game.enemies[i];
    ImGui::PushID(static_cast<int>(i));
    bool lane = e.champion->id == game.laneOpponentId;
    float s = dp(34);
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    drawIcon(dl, e.champion->image, p, {p.x + s, p.y + s}, dp(5));
    if (lane) dl->AddRect(p, {p.x + s, p.y + s}, u32(color::gold), dp(5), 0, dp(2));
    ImGui::Dummy({s, s});
    ImGui::SameLine(0, dp(10));
    ImGui::BeginGroup();
    ImGui::TextUnformatted(e.champion->name.c_str());
    if (lane) {
      ImGui::SameLine();
      badge("LANE", u32(color::gold), u32(color::bg));
    }
    itemRow(e.items, dp(22), "no items yet");
    ImGui::EndGroup();
    ImGui::PopID();
  }
  if (!game.laneOpponentId.empty())
    mutedWrapped("Your lane opponent counts most toward your early items.");
  endPanel();
}

void App::itemRow(const std::vector<const Item*>& items, float size, const char* empty) {
  if (items.empty()) {
    ImGui::PushFont(fonts_.small);
    ImGui::TextDisabled("%s", empty);
    ImGui::PopFont();
    return;
  }
  for (std::size_t i = 0; i < items.size(); i++) {
    if (i) ImGui::SameLine(0, dp(4));
    ImGui::PushID(static_cast<int>(i));
    itemIcon(*items[i], size);
    ImGui::PopID();
  }
}

void App::drawResults(const Snapshot& snap) {
  const Game& game = snap.game;
  const TeamAnalysis& analysis = snap.analysis;
  const Build& build = snap.build;
  const Pick* nextCore = snap.nextCore >= 0 ? &build.core[snap.nextCore] : nullptr;

  drawNextOptions(snap);

  if (!analysis.profiles.empty()) drawAnalysis(analysis);

  // Starting items only matter before your first back.
  if (game.owned.empty()) {
    beginPanel("start", "Start");
    for (std::size_t i = 0; i < build.starter.size(); i++) {
      if (i) ImGui::SameLine(0, dp(8));
      ImGui::PushID(static_cast<int>(i));
      itemIcon(*build.starter[i], dp(40));
      ImGui::PopID();
    }
    endPanel();
  }

  beginPanel("build", "Build order");
  auto statusOf = [&](const Pick& p) {
    return p.owned ? CardStatus::Owned : &p == nextCore ? CardStatus::Next : CardStatus::None;
  };
  if (build.boots) itemCard(*build.boots->item, &build.boots->reasons, 0, statusOf(*build.boots));
  for (std::size_t i = 0; i < build.core.size(); i++)
    itemCard(*build.core[i].item, &build.core[i].reasons, static_cast<int>(i) + 1, statusOf(build.core[i]));
  mutedWrapped("Boots (0) usually come after your first item; buy tier-1 Boots early as needed.");
  endPanel();

  if (!build.situational.empty()) {
    beginPanel("situational", "Situational swaps");
    for (const Pick& p : build.situational) itemCard(*p.item, &p.reasons, -1, CardStatus::None);
    endPanel();
  }

  if (!build.notes.empty()) {
    beginPanel("notes", "Matchup tips");
    bullets(build.notes, fonts_.body);
    endPanel();
  }
}

void App::drawNextOptions(const Snapshot& snap) {
  const Build& build = snap.build;
  beginPanel("next", "Options for your next item", build.nextOptions.empty() ? 0 : u32(color::accent, 0.6f));
  float rightEdge = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x;
  std::string goldLabel = snap.goldLeft < build.totalGold ? "Gold left to full build" : "Full build";
  std::string goldText = withCommas(snap.goldLeft) + "g";
  float blockWidth = std::max(measure(fonts_.small, goldLabel).x, measure(fonts_.big, goldText).x);
  float top = ImGui::GetCursorPosY();

  if (build.nextOptions.empty()) mutedWrapped("You own every item in the build.", false);
  ImGui::PushTextWrapPos(rightEdge - blockWidth - dp(16));
  for (std::size_t i = 0; i < build.nextOptions.size(); i++) {
    const Pick& p = build.nextOptions[i];
    ImGui::PushID(static_cast<int>(i));
    itemIcon(*p.item, dp(40));
    ImGui::SameLine(0, dp(12));
    ImGui::BeginGroup();
    ImGui::PushFont(fonts_.bold);
    ImGui::TextUnformatted(p.item->name.c_str());
    ImGui::PopFont();
    ImGui::SameLine();
    ImGui::TextColored(rgba(color::gold), "%dg", p.item->gold);
    ImGui::PushFont(fonts_.small);
    ImGui::TextDisabled("%s", join(p.reasons, 2, " · ").c_str());
    ImGui::PopFont();
    ImGui::EndGroup();
    ImGui::PopID();
  }
  if (build.boots && !build.boots->owned) {
    ImGui::PushFont(fonts_.small);
    ImGui::TextDisabled("Boots when you can: %s (%dg)", build.boots->item->name.c_str(), build.boots->item->gold);
    ImGui::PopFont();
  }
  ImGui::PopTextWrapPos();
  float bottom = ImGui::GetCursorPosY();

  ImGui::SetCursorPos({rightEdge - measure(fonts_.small, goldLabel).x, top});
  ImGui::PushFont(fonts_.small);
  ImGui::TextDisabled("%s", goldLabel.c_str());
  ImGui::PopFont();
  ImGui::SetCursorPosX(rightEdge - measure(fonts_.big, goldText).x);
  ImGui::PushFont(fonts_.big);
  ImGui::TextColored(rgba(color::gold), "%s", goldText.c_str());
  ImGui::PopFont();
  ImGui::SetCursorPosY(std::max(ImGui::GetCursorPosY(), bottom));
  endPanel();
}

void App::drawAnalysis(const TeamAnalysis& analysis) {
  beginPanel("analysis", "Enemy threat read");
  ImDrawList* dl = ImGui::GetWindowDrawList();

  // AD / AP split bar.
  int ad = static_cast<int>(std::lround(analysis.adShare * 100));
  float w = ImGui::GetContentRegionAvail().x;
  float h = dp(22);
  ImVec2 p = ImGui::GetCursorScreenPos();
  float split = p.x + w * ad / 100.f;
  float r = dp(5);
  dl->AddRectFilled(p, {split, p.y + h}, u32(color::ad), r, ImDrawFlags_RoundCornersLeft);
  dl->AddRectFilled({split, p.y}, {p.x + w, p.y + h}, u32(color::ap), r, ImDrawFlags_RoundCornersRight);
  auto label = [&](const std::string& text, float x0, float x1) {
    ImVec2 size = measure(fonts_.small, text);
    textAt(dl, fonts_.small, {(x0 + x1 - size.x) / 2, p.y + (h - size.y) / 2}, u32(0xffffff), text);
  };
  if (ad >= 12) label(std::to_string(ad) + "% AD", p.x, split);
  if (100 - ad >= 12) label(std::to_string(100 - ad) + "% AP", split, p.x + w);
  ImGui::Dummy({w, h});

  if (const ThreatProfile* lane = analysis.laneOpponent()) {
    char weight[16];
    std::snprintf(weight, sizeof weight, "%g", LANE_OPPONENT_WEIGHT);
    mutedWrapped("Lane opponent " + lane->champion->name + " is weighted " + weight +
                 "× — you'll fight them most early.");
  }

  if (!analysis.intel.empty()) {
    ImGui::Spacing();
    ImGui::PushFont(fonts_.bold);
    ImGui::TextColored(rgba(color::goldBright), "From their items");
    ImGui::PopFont();
    bullets(analysis.intel, fonts_.small);
  }

  struct Threat {
    Need need;
    const char* label;
    double level;
  };
  std::vector<Threat> threats;
  for (const auto& [need, text] : THREAT_LABELS)
    if (analysis.need(need) >= 0.3) threats.push_back({need, text, analysis.need(need)});
  std::stable_sort(threats.begin(), threats.end(), [](const Threat& a, const Threat& b) { return a.level > b.level; });

  ImGui::Spacing();
  if (threats.empty()) {
    mutedWrapped("No standout threats.");
  } else {
    int columns = std::max(1, static_cast<int>(ImGui::GetContentRegionAvail().x / dp(220)));
    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, {dp(9), dp(5)});
    if (ImGui::BeginTable("threats", columns, ImGuiTableFlags_SizingStretchSame)) {
      for (const Threat& t : threats) {
        ImGui::TableNextColumn();
        ImGui::PushFont(fonts_.small);
        float colW = ImGui::GetContentRegionAvail().x;
        ImVec2 q = ImGui::GetCursorScreenPos();
        ImGui::TextUnformatted(t.label);
        float labelW = ImGui::GetItemRectSize().x;
        const auto& who = analysis.sourcesOf(t.need);
        std::string names = join(who, 3);
        float namesW = measure(fonts_.small, names).x;
        float x = std::max(q.x + labelW + dp(8), q.x + colW - namesW);
        textAt(ImGui::GetWindowDrawList(), fonts_.small, {x, q.y}, u32(color::muted), names, q.x + colW);
        ImGui::PopFont();
        if (!who.empty() && ImGui::IsMouseHoveringRect(q, {q.x + colW, q.y + ImGui::GetTextLineHeight()}))
          ImGui::SetTooltip("%s", join(who, who.size()).c_str());

        ImVec2 m = ImGui::GetCursorScreenPos();
        float mh = dp(6);
        ImDrawList* tdl = ImGui::GetWindowDrawList();
        tdl->AddRectFilled(m, {m.x + colW, m.y + mh}, u32(color::bg), mh / 2);
        tdl->AddRectFilled(m, {m.x + colW * static_cast<float>(t.level), m.y + mh},
                           u32(t.level >= 0.7 ? color::ad : color::gold), mh / 2);
        ImGui::Dummy({colW, mh});
      }
      ImGui::EndTable();
    }
    ImGui::PopStyleVar();
  }
  endPanel();
}

void App::itemIcon(const Item& item, float size) {
  ImGui::InvisibleButton("##icon", {size, size});
  ImVec2 min = ImGui::GetItemRectMin(), max = ImGui::GetItemRectMax();
  ImDrawList* dl = ImGui::GetWindowDrawList();
  drawIcon(dl, item.image, min, max, dp(6));
  dl->AddRect(min, max, u32(color::border), dp(6));
  itemTooltip(item);
}

void App::itemTooltip(const Item& item) {
  if (!ImGui::BeginItemTooltip()) return;
  ImGui::PushTextWrapPos(dp(340));
  ImGui::PushFont(fonts_.bold);
  ImGui::TextColored(rgba(color::goldBright), "%s", item.name.c_str());
  ImGui::PopFont();
  ImGui::SameLine();
  ImGui::TextColored(rgba(color::gold), "%dg", item.gold);
  ImGui::PushFont(fonts_.small);
  ImGui::TextUnformatted(description(item).c_str());
  ImGui::PopFont();
  ImGui::PopTextWrapPos();
  ImGui::EndTooltip();
}

void App::itemCard(const Item& item, const std::vector<std::string>* reasons, int index, CardStatus status) {
  ImGui::PushID(item.id.c_str());
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 start = ImGui::GetCursorScreenPos();
  float w = ImGui::GetContentRegionAvail().x;
  float pad = dp(10);
  float iconSize = dp(44);
  if (status == CardStatus::Owned) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.55f);

  dl->ChannelsSplit(2);
  dl->ChannelsSetCurrent(1);
  ImGui::SetCursorScreenPos({start.x + pad, start.y + pad});
  itemIcon(item, iconSize);
  ImVec2 iconMin = ImGui::GetItemRectMin();
  if (index >= 0) {
    std::string n = std::to_string(index);
    float r = dp(9);
    ImVec2 c = {iconMin.x + dp(2), iconMin.y + dp(2)};
    dl->AddCircleFilled(c, r, u32(color::bg));
    dl->AddCircle(c, r, u32(color::gold));
    ImVec2 size = measure(fonts_.small, n);
    textAt(dl, fonts_.small, {c.x - size.x / 2, c.y - size.y / 2}, u32(color::gold), n);
  }
  ImGui::SameLine(0, dp(12));

  ImGui::BeginGroup();
  float right = start.x + w - pad;
  ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + (right - ImGui::GetCursorScreenPos().x));
  ImGui::PushFont(fonts_.bold);
  ImGui::TextUnformatted(item.name.c_str());
  ImGui::PopFont();
  ImGui::SameLine();
  ImGui::TextColored(rgba(color::gold), "%dg", item.gold);
  if (status == CardStatus::Next) {
    ImGui::SameLine();
    badge("NEXT SLOT", u32(color::accent), u32(color::bg));
  } else if (status == CardStatus::Owned) {
    ImGui::SameLine();
    badge("OWNED", u32(color::border), u32(color::muted));
  }
  if (reasons) {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {dp(6), dp(3)});
    bullets(*reasons, fonts_.small);
    ImGui::PopStyleVar();
  }
  ImGui::PopTextWrapPos();
  ImGui::EndGroup();

  float bottom = std::max(ImGui::GetItemRectMax().y, start.y + pad + iconSize) + pad;
  dl->ChannelsSetCurrent(0);
  bool next = status == CardStatus::Next;
  dl->AddRectFilled(start, {start.x + w, bottom}, IM_COL32(0, 0, 0, 51), dp(8));
  dl->AddRect(start, {start.x + w, bottom}, u32(next ? color::accent : color::border), dp(8), 0, next ? dp(2) : dp(1));
  dl->ChannelsMerge();

  ImGui::SetCursorScreenPos(start);
  ImGui::Dummy({w, bottom - start.y});
  if (status == CardStatus::Owned) ImGui::PopStyleVar();
  ImGui::PopID();
}

void App::beginPanel(const char* id, const char* title, ImU32 border) {
  ImGui::PushStyleColor(ImGuiCol_ChildBg, rgba(color::panel));
  ImGui::PushStyleColor(ImGuiCol_Border, border ? ImGui::ColorConvertU32ToFloat4(border) : rgba(color::border));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {dp(16), dp(16)});
  ImGui::BeginChild(id, {0, 0},
                    ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AlwaysUseWindowPadding,
                    ImGuiWindowFlags_NoScrollbar);
  ImGui::PopStyleVar();
  ImGui::PopStyleColor(2);
  if (title) heading(title);
}

void App::endPanel() {
  ImGui::EndChild();
  ImGui::Dummy({0, dp(8)});
}

void App::heading(const char* text) {
  std::string upper = text;
  for (char& ch : upper) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
  ImGui::PushFont(fonts_.heading);
  ImGui::TextColored(rgba(color::gold), "%s", upper.c_str());
  ImGui::PopFont();
  ImGui::Dummy({0, dp(2)});
}

void App::mutedWrapped(const std::string& text, bool small) {
  if (small) ImGui::PushFont(fonts_.small);
  ImGui::PushStyleColor(ImGuiCol_Text, rgba(color::muted));
  ImGui::PushTextWrapPos(0);
  ImGui::TextUnformatted(text.c_str());
  ImGui::PopTextWrapPos();
  ImGui::PopStyleColor();
  if (small) ImGui::PopFont();
}

void App::bullets(const std::vector<std::string>& lines, ImFont* font) {
  ImGui::PushFont(font);
  for (const std::string& line : lines) {
    ImGui::Bullet();
    ImGui::SameLine();
    ImGui::TextWrapped("%s", line.c_str());
  }
  ImGui::PopFont();
}

void App::badge(const char* text, ImU32 bg, ImU32 fg) {
  ImVec2 size = measure(fonts_.small, text);
  ImVec2 pad = {dp(6), dp(1)};
  ImVec2 p = ImGui::GetCursorScreenPos();
  p.y += (ImGui::GetTextLineHeight() - size.y - pad.y * 2) / 2;
  ImDrawList* dl = ImGui::GetWindowDrawList();
  dl->AddRectFilled(p, {p.x + size.x + pad.x * 2, p.y + size.y + pad.y * 2}, bg, dp(4));
  textAt(dl, fonts_.small, {p.x + pad.x, p.y + pad.y}, fg, text);
  ImGui::Dummy({size.x + pad.x * 2, ImGui::GetTextLineHeight()});
}

void App::drawIcon(ImDrawList* dl, const std::string& url, ImVec2 min, ImVec2 max, float rounding) {
  if (ImTextureID tex = icons_.get(url)) dl->AddImageRounded(tex, min, max, {0, 0}, {1, 1}, IM_COL32_WHITE, rounding);
  else dl->AddRectFilled(min, max, u32(color::panel2), rounding);
}

void App::textAt(ImDrawList* dl, ImFont* font, ImVec2 pos, ImU32 col, const std::string& text, float clipRight) {
  float size = font->FontSize * ImGui::GetIO().FontGlobalScale;
  if (clipRight > 0) {
    ImVec4 clip = {pos.x, pos.y, clipRight, pos.y + size * 2};
    dl->AddText(font, size, pos, col, text.c_str(), nullptr, 0, &clip);
  } else {
    dl->AddText(font, size, pos, col, text.c_str());
  }
}

ImVec2 App::measure(ImFont* font, const std::string& text) const {
  return font->CalcTextSizeA(font->FontSize * ImGui::GetIO().FontGlobalScale, FLT_MAX, 0, text.c_str());
}

const std::string& App::description(const Item& item) {
  auto [it, inserted] = descriptions_.try_emplace(item.id);
  if (inserted) it->second = describeItem(item);
  return it->second;
}

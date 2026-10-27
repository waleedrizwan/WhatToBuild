#pragma once
// Reads the in-progress game from League's Live Client Data API
// (https://developer.riotgames.com/docs/lol#game-client-api_live-client-data-api),
// which the game serves on https://127.0.0.1:2999 while a match is running.

#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "data/champions.h"
#include "data/ddragon.h"

struct LiveEnemy {
  std::string id;  // champion id
  std::vector<std::string> items;
};

struct LiveGame {
  std::string you;
  std::vector<std::string> yourItems;
  std::optional<Role> role;
  /** Up to 5 enemies. */
  std::vector<LiveEnemy> enemies;
  /** Index into `enemies` of your lane opponent, or -1 if positions are unknown. */
  int laneIndex = -1;
  double gameTime = 0;
  std::string gameMode;
};

struct LiveStatus {
  enum class State { Off, Waiting, Connected };
  State state = State::Off;
  std::string message;  // while waiting
  LiveGame game;        // while connected
};

/** The API base URL; LIVE_CLIENT_URL overrides it (see tools/mock_live_client.cpp). */
std::string liveClientUrl();

/** Parses an /allgamedata response; nullopt while spectating or still loading. */
std::optional<LiveGame> parseLiveGame(const std::string& body, const GameData& data);

LiveStatus fetchLiveGame(const GameData& data, const std::string& baseUrl);

/** Polls the Live Client API on a background thread every few seconds while started. */
class LivePoller {
 public:
  /** `onUpdate` runs on the polling thread after each poll, e.g. to wake the UI. */
  explicit LivePoller(std::function<void()> onUpdate);
  ~LivePoller();
  LivePoller(const LivePoller&) = delete;
  LivePoller& operator=(const LivePoller&) = delete;

  void start(const GameData& data);
  void stop();
  /** The latest poll result, including the game while connected. */
  LiveStatus status() const;

 private:
  void run(const GameData* data, std::string url);

  std::function<void()> onUpdate_;
  mutable std::mutex mu_;
  std::condition_variable cv_;
  bool stopping_ = false;
  LiveStatus status_;
  std::thread thread_;
};

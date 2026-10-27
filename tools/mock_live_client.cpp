// Fake League Live Client Data API for trying "Live game" mode without being in a match.
//   ./build/tolleagle-mock-live                                        (terminal 1)
//   LIVE_CLIENT_URL=http://127.0.0.1:2998 ./build/tolleagle            (terminal 2)
// Items are bought over time, so the build updates as the "game" progresses.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdio>
#include <string>
#include <vector>

using nlohmann::json;

namespace {

constexpr int PORT = 2998;

struct MockPlayer {
  std::string champ;  // Data Dragon id
  std::string name;   // display name
  std::string team;
  std::string position;
  std::string riotId;
  /** Item ids, bought one every ~20s of mock time. */
  std::vector<int> buys;
};

const std::vector<MockPlayer> PLAYERS = {
    {"Jinx", "Jinx", "ORDER", "BOTTOM", "You#NA1", {3006, 3031, 3032}},
    {"Ornn", "Ornn", "ORDER", "TOP", "Ally1#NA1", {3047, 6665}},
    {"LeeSin", "Lee Sin", "ORDER", "JUNGLE", "Ally2#NA1", {3071}},
    {"Ahri", "Ahri", "ORDER", "MIDDLE", "Ally3#NA1", {6655}},
    {"Thresh", "Thresh", "ORDER", "UTILITY", "Ally4#NA1", {3190}},
    {"Malphite", "Malphite", "CHAOS", "TOP", "Enemy1#NA1", {3047, 3075, 3110}},
    {"Vi", "Vi", "CHAOS", "JUNGLE", "Enemy2#NA1", {3071, 3053}},
    {"Kaisa", "Kai'Sa", "CHAOS", "BOTTOM", "Enemy3#NA1", {3115, 4633, 3157}},
    {"Zed", "Zed", "CHAOS", "MIDDLE", "Enemy4#NA1", {6698, 3814}},
    {"Soraka", "Soraka", "CHAOS", "UTILITY", "Enemy5#NA1", {6617}},
};

const auto started = std::chrono::steady_clock::now();

json snapshot() {
  double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
  std::size_t bought = static_cast<std::size_t>(elapsed / 20) + 1;
  json players = json::array();
  for (const MockPlayer& p : PLAYERS) {
    std::string gameName = p.riotId.substr(0, p.riotId.find('#'));
    json items = json::array();
    for (std::size_t slot = 0; slot < p.buys.size() && slot < bought; slot++)
      items.push_back({{"itemID", p.buys[slot]}, {"displayName", std::to_string(p.buys[slot])}, {"slot", slot}, {"count", 1}});
    players.push_back({
        {"championName", p.name},
        {"rawChampionName", "game_character_displayname_" + p.champ},
        {"riotId", p.riotId},
        {"riotIdGameName", gameName},
        {"summonerName", gameName},
        {"team", p.team},
        {"position", p.position},
        {"items", items},
    });
  }
  return {
      {"activePlayer", {{"riotId", "You#NA1"}, {"riotIdGameName", "You"}, {"summonerName", "You"}}},
      {"allPlayers", players},
      {"gameData", {{"gameMode", "CLASSIC"}, {"gameTime", 300 + elapsed}, {"mapNumber", 11}}},  // start at 5:00
  };
}

void respond(int client) {
  char buf[4096];
  ssize_t n = read(client, buf, sizeof buf - 1);
  if (n <= 0) return;
  buf[n] = '\0';
  std::string request(buf);
  bool match = request.starts_with("GET /liveclientdata/allgamedata");
  std::string body = match ? snapshot().dump() : "";
  std::string head = std::string(match ? "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n" : "HTTP/1.1 404 Not Found\r\n") +
                     "Content-Length: " + std::to_string(body.size()) + "\r\nConnection: close\r\n\r\n";
  std::string out = head + body;
  for (std::size_t sent = 0; sent < out.size();) {
    ssize_t w = write(client, out.data() + sent, out.size() - sent);
    if (w <= 0) return;
    sent += static_cast<std::size_t>(w);
  }
}

}  // namespace

int main() {
  int server = socket(AF_INET, SOCK_STREAM, 0);
  int yes = 1;
  setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof addr) != 0 || listen(server, 16) != 0) {
    std::perror("Couldn't listen on 127.0.0.1:2998");
    return 1;
  }
  std::printf("Mock Live Client API on http://127.0.0.1:%d\n", PORT);
  std::fflush(stdout);
  while (true) {
    int client = accept(server, nullptr, nullptr);
    if (client < 0) continue;
    respond(client);
    close(client);
  }
}

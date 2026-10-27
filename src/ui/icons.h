#pragma once
// Champion and item icons, downloaded in the background and uploaded as GL textures.

#include <imgui.h>

#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class IconCache {
 public:
  /** `wake` runs on a worker thread whenever a download finishes. */
  explicit IconCache(std::function<void()> wake, int workers = 4);
  ~IconCache();
  IconCache(const IconCache&) = delete;
  IconCache& operator=(const IconCache&) = delete;

  /** Texture for the image at `url`, or 0 while it is still downloading (or failed). */
  ImTextureID get(const std::string& url);
  /** Uploads finished downloads to the GPU. Call once per frame on the GL thread. */
  void pump();

 private:
  struct Decoded {
    std::string url;
    int width = 0;
    int height = 0;
    std::vector<unsigned char> rgba;
  };

  void work();

  std::function<void()> wake_;
  std::mutex mu_;
  std::condition_variable cv_;
  bool stopping_ = false;
  std::deque<std::string> queue_;
  std::vector<Decoded> done_;
  std::vector<std::thread> workers_;
  // GL thread only. 0 = requested but not (yet) available.
  std::unordered_map<std::string, unsigned> textures_;
};

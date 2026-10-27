#include "ui/icons.h"

// GLFW pulls in the platform's OpenGL header; only GL 1.1 calls are used here.
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#include "stb_image.h"

#include "net/http.h"

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

IconCache::IconCache(std::function<void()> wake, int workers) : wake_(std::move(wake)) {
  for (int i = 0; i < workers; i++) workers_.emplace_back(&IconCache::work, this);
}

IconCache::~IconCache() {
  {
    std::lock_guard lock(mu_);
    stopping_ = true;
  }
  cv_.notify_all();
  for (auto& t : workers_) t.join();
  for (auto& [url, tex] : textures_)
    if (tex) glDeleteTextures(1, &tex);
}

ImTextureID IconCache::get(const std::string& url) {
  if (url.empty()) return 0;
  auto [it, inserted] = textures_.try_emplace(url, 0);
  if (inserted) {
    {
      std::lock_guard lock(mu_);
      queue_.push_back(url);
    }
    cv_.notify_one();
  }
  return static_cast<ImTextureID>(it->second);
}

void IconCache::pump() {
  std::vector<Decoded> ready;
  {
    std::lock_guard lock(mu_);
    ready.swap(done_);
  }
  for (Decoded& d : ready) {
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, d.width, d.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, d.rgba.data());
    textures_[d.url] = tex;
  }
}

void IconCache::work() {
  while (true) {
    std::string url;
    {
      std::unique_lock lock(mu_);
      cv_.wait(lock, [this] { return stopping_ || !queue_.empty(); });
      if (stopping_) return;
      url = std::move(queue_.front());
      queue_.pop_front();
    }
    HttpResponse res = httpGet(url);
    if (!res.ok()) continue;  // stays blank; not worth retrying for an icon
    int w = 0, h = 0, channels = 0;
    unsigned char* pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(res.body.data()),
                                                  static_cast<int>(res.body.size()), &w, &h, &channels, 4);
    if (!pixels) continue;
    Decoded d{url, w, h, std::vector<unsigned char>(pixels, pixels + static_cast<std::size_t>(w) * h * 4)};
    stbi_image_free(pixels);
    {
      std::lock_guard lock(mu_);
      done_.push_back(std::move(d));
    }
    if (wake_) wake_();
  }
}

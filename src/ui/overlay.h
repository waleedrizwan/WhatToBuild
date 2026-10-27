#pragma once
// A small always-on-top panel drawn over the game while a match runs. It's a separate,
// click-through OS window, not anything injected into the game process.

#include <imgui.h>

#include <functional>

struct GLFWwindow;

class Overlay {
 public:
  /**
   * Shares GL objects with `mainWindow` and fonts with the current ImGui context.
   * `contentWidth` is how wide the drawn contents are, in window units.
   */
  Overlay(GLFWwindow* mainWindow, float framebufferScale, float contentWidth);
  ~Overlay();
  Overlay(const Overlay&) = delete;
  Overlay& operator=(const Overlay&) = delete;

  /** Draws one frame with `draw` (ImGui calls) when `visible`, otherwise hides the window. */
  void frame(bool visible, const std::function<void()>& draw);

 private:
  GLFWwindow* main_;
  GLFWwindow* window_ = nullptr;
  ImGuiContext* context_ = nullptr;
  bool shown_ = false;
  ImVec2 size_ = {0, 100};
};

/** Keeps a window above other apps, including full-screen ones and other Spaces on macOS. */
void floatAboveOtherApps(GLFWwindow* window);

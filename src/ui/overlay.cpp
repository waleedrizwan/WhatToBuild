#include "ui/overlay.h"

#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>
#if defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>
#include <objc/message.h>
#include <objc/runtime.h>
#endif

#include <cmath>

namespace {

constexpr int MARGIN = 24;

}  // namespace

Overlay::Overlay(GLFWwindow* mainWindow, float framebufferScale, float contentWidth) : main_(mainWindow) {
  size_.x = contentWidth + ImGui::GetStyle().WindowPadding.x * 2;
  glfwDefaultWindowHints();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
  glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
  glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);  // clicks go to the game underneath
  glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
  glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
  window_ = glfwCreateWindow(static_cast<int>(size_.x), static_cast<int>(size_.y), "Tolleagle overlay", nullptr,
                             mainWindow);
  glfwDefaultWindowHints();
  if (!window_) return;
  floatAboveOtherApps(window_);

  ImGuiContext* mainContext = ImGui::GetCurrentContext();
  const ImGuiIO& mainIo = ImGui::GetIO();
  ImGuiStyle style = ImGui::GetStyle();
  ImFont* defaultFont = mainIo.FontDefault;

  glfwMakeContextCurrent(window_);
  glfwSwapInterval(0);  // the main window already paces the loop
  context_ = ImGui::CreateContext(mainIo.Fonts);
  ImGui::SetCurrentContext(context_);
  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  io.ConfigFlags |= ImGuiConfigFlags_NoMouse | ImGuiConfigFlags_NoMouseCursorChange;
  io.FontDefault = defaultFont;
  io.FontGlobalScale = 1 / framebufferScale;
  // A floating card: the panel colour, slightly see-through, with rounded corners.
  style.Colors[ImGuiCol_WindowBg] = {0x11 / 255.f, 0x18 / 255.f, 0x23 / 255.f, 0.92f};
  style.WindowRounding = style.ChildRounding;
  style.WindowBorderSize = 1;
  ImGui::GetStyle() = style;
  ImGui_ImplGlfw_InitForOpenGL(window_, false);
  ImGui_ImplOpenGL3_Init("#version 150");

  ImGui::SetCurrentContext(mainContext);
  glfwMakeContextCurrent(main_);
}

Overlay::~Overlay() {
  if (!window_) return;
  ImGuiContext* mainContext = ImGui::GetCurrentContext();
  glfwMakeContextCurrent(window_);
  ImGui::SetCurrentContext(context_);
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext(context_);
  ImGui::SetCurrentContext(mainContext);
  glfwMakeContextCurrent(main_);
  glfwDestroyWindow(window_);
}

void Overlay::frame(bool visible, const std::function<void()>& draw) {
  if (!window_) return;
  if (!visible) {
    if (shown_) glfwHideWindow(window_);
    shown_ = false;
    return;
  }

  ImGuiContext* mainContext = ImGui::GetCurrentContext();
  glfwMakeContextCurrent(window_);
  ImGui::SetCurrentContext(context_);

  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  // The panel fills the OS window; the OS window is then fitted to the content's height.
  // (ImGui won't auto-size a window past its OS window, so this can't use AlwaysAutoResize.)
  ImGui::SetNextWindowPos({0, 0});
  ImGui::SetNextWindowSize(size_);
  ImGui::Begin("Tolleagle overlay", nullptr,
               ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings |
                   ImGuiWindowFlags_NoFocusOnAppearing);
  draw();
  float height = std::ceil(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y + ImGui::GetStyle().WindowPadding.y);
  ImGui::End();
  ImGui::Render();

  if (std::abs(height - size_.y) >= 1 || !shown_) {
    bool resized = std::abs(height - size_.y) >= 1;
    size_.y = height;
    int w = static_cast<int>(std::ceil(size_.x)), h = static_cast<int>(height);
    glfwSetWindowSize(window_, w, h);
    // Pin it to the right edge of the primary screen, a little below the top.
    int areaX = 0, areaY = 0, areaW = 0, areaH = 0;
    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &areaX, &areaY, &areaW, &areaH);
    glfwSetWindowPos(window_, areaX + areaW - w - MARGIN, areaY + areaH / 5);
    if (resized) glfwPostEmptyEvent();  // draw again at the new size
  }

  int fbw = 0, fbh = 0;
  glfwGetFramebufferSize(window_, &fbw, &fbh);
  glViewport(0, 0, fbw, fbh);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT);
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  glfwSwapBuffers(window_);
  if (!shown_) glfwShowWindow(window_);
  shown_ = true;

  ImGui::SetCurrentContext(mainContext);
  glfwMakeContextCurrent(main_);
}

#if defined(__APPLE__)
void floatAboveOtherApps(GLFWwindow* window) {
  // Plain C++ calls into AppKit through the Objective-C runtime.
  constexpr long NSStatusWindowLevel = 25;
  constexpr unsigned long CanJoinAllSpaces = 1 << 0, Stationary = 1 << 4, IgnoresCycle = 1 << 6,
                          FullScreenAuxiliary = 1 << 8;
  id nsWindow = static_cast<id>(glfwGetCocoaWindow(window));
  auto send = [&](const char* selector, auto arg) {
    reinterpret_cast<void (*)(id, SEL, decltype(arg))>(objc_msgSend)(nsWindow, sel_registerName(selector), arg);
  };
  send("setLevel:", NSStatusWindowLevel);
  // Follow the user across Spaces, including League's full-screen one.
  send("setCollectionBehavior:", CanJoinAllSpaces | Stationary | IgnoresCycle | FullScreenAuxiliary);
  send("setHidesOnDeactivate:", static_cast<signed char>(false));
}
#else
// GLFW_FLOATING is enough elsewhere; League needs to run borderless or windowed to sit underneath.
void floatAboveOtherApps(GLFWwindow*) {}
#endif

// Tolleagle: matchup-aware League of Legends item build advisor.

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <cstdio>

#include "ui/app.h"

int main() {
  glfwSetErrorCallback([](int code, const char* message) { std::fprintf(stderr, "GLFW error %d: %s\n", code, message); });
  if (!glfwInit()) return 1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
  GLFWwindow* window = glfwCreateWindow(1240, 860, "Tolleagle — LoL Build Advisor", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  glfwSetWindowSizeLimits(window, 900, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().IniFilename = nullptr;

  float contentScale = 1;
  glfwGetWindowContentScale(window, &contentScale, nullptr);
  int windowWidth = 0, framebufferWidth = 0;
  glfwGetWindowSize(window, &windowWidth, nullptr);
  glfwGetFramebufferSize(window, &framebufferWidth, nullptr);
  float framebufferScale = windowWidth > 0 ? static_cast<float>(framebufferWidth) / windowWidth : 1;

  Fonts fonts;
  float uiScale = setupStyle(fonts, contentScale, framebufferScale);
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 150");

  {
    App app(fonts, uiScale, [] { glfwPostEmptyEvent(); });
    while (!glfwWindowShouldClose(window)) {
      // Sleep until input arrives or a background download/poll finishes.
      glfwWaitEventsTimeout(0.5);
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      app.frame();
      ImGui::Render();

      int w = 0, h = 0;
      glfwGetFramebufferSize(window, &w, &h);
      glViewport(0, 0, w, h);
      glClearColor(0x0a / 255.f, 0x0e / 255.f, 0x14 / 255.f, 1);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      glfwSwapBuffers(window);
    }
  }  // App (and its GL textures) go away while the context still exists

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

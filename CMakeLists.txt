cmake_minimum_required(VERSION 3.24)
project(tolleagle LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(FetchContent)

FetchContent_Declare(json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw
  URL https://github.com/glfw/glfw/releases/download/3.4/glfw-3.4.zip)

FetchContent_Declare(imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.8.tar.gz)

FetchContent_MakeAvailable(json glfw imgui)

find_package(CURL REQUIRED)
find_package(OpenGL REQUIRED)

# --- Dear ImGui (no CMake build of its own) ---
add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends ${imgui_SOURCE_DIR}/misc/cpp)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# --- Engine + data (no UI), shared by the app and the smoke test ---
add_library(tolleagle_core STATIC
  src/net/http.cpp
  src/data/ddragon.cpp
  src/data/champions.cpp
  src/data/items.cpp
  src/engine/enemy_items.cpp
  src/engine/analyze.cpp
  src/engine/recommend.cpp
  src/live/live_client.cpp)
target_include_directories(tolleagle_core PUBLIC src)
target_link_libraries(tolleagle_core PUBLIC CURL::libcurl PRIVATE nlohmann_json::nlohmann_json)
find_package(Threads REQUIRED)
target_link_libraries(tolleagle_core PUBLIC Threads::Threads)

# A .app bundle on macOS, so it can live in /Applications and be opened from Finder.
add_executable(tolleagle MACOSX_BUNDLE src/main.cpp src/ui/app.cpp src/ui/icons.cpp)
set_target_properties(tolleagle PROPERTIES
  OUTPUT_NAME $<IF:$<PLATFORM_ID:Darwin>,Tolleagle,tolleagle>
  MACOSX_BUNDLE_BUNDLE_NAME Tolleagle
  MACOSX_BUNDLE_GUI_IDENTIFIER com.tolleagle.app
  MACOSX_BUNDLE_SHORT_VERSION_STRING 0.2.0)
target_include_directories(tolleagle PRIVATE third_party)
target_link_libraries(tolleagle PRIVATE tolleagle_core imgui)
if(APPLE)
  target_compile_definitions(imgui PUBLIC GL_SILENCE_DEPRECATION)
endif()

add_executable(tolleagle-smoke tools/smoke.cpp)
target_link_libraries(tolleagle-smoke PRIVATE tolleagle_core)

if(NOT WIN32)
  add_executable(tolleagle-mock-live tools/mock_live_client.cpp)
  target_link_libraries(tolleagle-mock-live PRIVATE nlohmann_json::nlohmann_json)
endif()

cmake_minimum_required(VERSION 3.20)

project(NetscapeMatrix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Replace "C:/vcpkg" with the actual path to your vcpkg installation
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

# Find ALL packages for the entire project at the top level.
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(duktape CONFIG REQUIRED)

# --- THIS IS THE DEFINITIVE FIX ---
# Create a robust, modern "INTERFACE" library target for Duktape.
# This bundles the include directories and library files into a single, linkable item.
add_library(duktape_lib INTERFACE)
target_include_directories(duktape_lib INTERFACE ${duktape_INCLUDE_DIRS})
target_link_libraries(duktape_lib INTERFACE ${duktape_LIBRARIES})
# ------------------------------------

# Add all our components
add_subdirectory(components/shared)
add_subdirectory(components/engine)
add_subdirectory(components/gpu)
add_subdirectory(components/net)
add_subdirectory(components/ui)
cmake_minimum_required(VERSION 3.20)

project(NetscapeMatrix LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Replace "C:/vcpkg" with the actual path to your vcpkg installation
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

# Find packages
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(cpr CONFIG REQUIRED)

# Manual Duktape setup - vcpkg's duktape doesn't provide modern CMake targets
find_path(DUKTAPE_INCLUDE_DIR duktape.h)
find_library(DUKTAPE_LIBRARY NAMES duktape)

if(NOT DUKTAPE_INCLUDE_DIR OR NOT DUKTAPE_LIBRARY)
    message(FATAL_ERROR "Duktape not found. Please install with: vcpkg install duktape")
endif()

# Create duktape interface library
add_library(duktape_lib INTERFACE)
target_include_directories(duktape_lib INTERFACE ${DUKTAPE_INCLUDE_DIR})
target_link_libraries(duktape_lib INTERFACE ${DUKTAPE_LIBRARY})

message(STATUS "Found Duktape: ${DUKTAPE_INCLUDE_DIR} ${DUKTAPE_LIBRARY}")

# Add all our components
add_subdirectory(components/shared)
add_subdirectory(components/engine)
add_subdirectory(components/gpu)
add_subdirectory(components/net)
add_subdirectory(components/ui)
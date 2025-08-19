add_library(gpu
    src/gpu_process.cpp
)

target_include_directories(gpu PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
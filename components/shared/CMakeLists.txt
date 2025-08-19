add_library(shared
    src/ipc_messages.cpp
)

target_include_directories(shared PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)
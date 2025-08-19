add_library(net
    src/network_process.cpp
)

target_include_directories(net PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Link 'net' to 'engine' (for the content blocker) and now 'cpr'
target_link_libraries(net PRIVATE engine cpr::cpr)
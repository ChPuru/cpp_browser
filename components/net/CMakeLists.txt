add_library(net
    src/network_process.cpp
)

target_include_directories(net PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# --- ADD THIS LINE ---
# The 'net' library needs to link to the 'engine' library to find its headers.
target_link_libraries(net PRIVATE engine)
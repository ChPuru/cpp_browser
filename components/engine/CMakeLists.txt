add_library(engine
    src/dom.cpp
    src/html_parser.cpp
    src/css.cpp
    src/css_parser.cpp
    src/style.cpp
    src/layout.cpp
    src/content_blocker.cpp
    src/javascript.cpp
)

target_include_directories(engine PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# Link the engine against our duktape library target
target_link_libraries(engine PUBLIC duktape_lib)

# Additional debugging - print Duktape info if found
if(TARGET duktape_lib)
    message(STATUS "Duktape successfully linked to engine component")
else()
    message(WARNING "duktape_lib target not found - check root CMakeLists.txt")
endif()
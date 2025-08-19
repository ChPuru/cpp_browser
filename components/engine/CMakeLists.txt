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

# --- THIS IS THE FIX ---
# Link the engine against our new virtual library target.
# The PUBLIC keyword ensures that any target linking to 'engine' (like our UI)
# will also automatically get the Duktape properties.
target_link_libraries(engine PUBLIC duktape_lib)
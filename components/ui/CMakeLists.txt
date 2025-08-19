add_executable(browser
    src/main.cpp
)

target_link_directories(browser PRIVATE ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)

# The ui component links to the engine. The engine brings its own dependencies.
# We no longer need to mention Duktape here at all.
target_link_libraries(browser PRIVATE
    engine
    gpu
    net
    shared
    glfw
    glad
    imgui::imgui
)
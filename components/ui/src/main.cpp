#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional>

// Graphics and Windowing
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Our Engine and Network components
#include "html_parser.h"
#include "css_parser.h"
#include "style.h"
#include "layout.h"
#include "content_blocker.h"
#include "network_process.h"
#include "javascript.h"

// A simple struct to hold the state of our browser's UI
struct UIState {
    char address_bar_text[1024] = "http://info.cern.ch"; // The first website!
    bool load_requested = true; // Load the page on startup
    std::string url_to_load = "http://info.cern.ch";
    bool show_dev_console = false;
    bool show_about_window = false;
};

// A function to apply our custom retro theme
void ApplyNetscapeTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f; style.ChildRounding = 0.0f; style.FrameRounding = 2.0f;
    style.GrabRounding = 0.0f; style.PopupRounding = 0.0f; style.ScrollbarRounding = 0.0f;
    style.FrameBorderSize = 1.0f;
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.00f, 0.50f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.79f, 0.79f, 0.79f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.01f, 0.0f, 0.01f, 1.00f);
}

// The rendering function for our layout tree
void render_layout_box(const Layout::LayoutBox* box, ImDrawList* draw_list, ImVec2 viewport_pos) {
    if (!box || !box->styled_node) return;

    ImVec2 current_pos = ImVec2(viewport_pos.x + box->dimensions.x, viewport_pos.y + box->dimensions.y);

    if (box->box_type == Layout::BoxType::Block) {
        auto it = box->styled_node->specified_values.find("background-color");
        if (it != box->styled_node->specified_values.end()) {
            if (const CSS::Color* color = std::get_if<CSS::Color>(&it->second)) {
                ImVec2 p_max(current_pos.x + box->dimensions.width, current_pos.y + box->dimensions.height);
                draw_list->AddRectFilled(current_pos, p_max, IM_COL32(color->r, color->g, color->b, color->a));
            }
        }
    }

    if (box->box_type == Layout::BoxType::Anonymous && box->styled_node->node->type == DOM::NodeType::Text) {
        auto it = box->styled_node->specified_values.find("color");
        if (it != box->styled_node->specified_values.end()) {
            if (const CSS::Color* color = std::get_if<CSS::Color>(&it->second)) {
                float font_size = 16.0f; // Default font size
                auto fs_it = box->styled_node->specified_values.find("font-size");
                if (fs_it != box->styled_node->specified_values.end()) {
                    if (const float* fs = std::get_if<float>(&fs_it->second)) {
                        font_size = *fs;
                    }
                }
                ImGui::GetFont()->Scale = font_size / ImGui::GetFontSize();
                ImGui::PushFont(ImGui::GetFont());
                
                ImVec2 text_pos(current_pos.x, current_pos.y);
                draw_list->AddText(text_pos, IM_COL32(color->r, color->g, color->b, color->a), box->styled_node->node->text_data.c_str());

                ImGui::PopFont();
            }
        }
    }

    for (const auto& child : box->children) {
        render_layout_box(child.get(), draw_list, viewport_pos);
    }
}

int main() {
    auto content_blocker = std::make_shared<Engine::ContentBlocker>();
    Net::NetworkProcess network_process(content_blocker);
    JS::JSEngine js_engine;

    if (!glfwInit()) { return -1; }
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Netscape Matrix", NULL, NULL);
    if (window == NULL) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { return -1; }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ApplyNetscapeTheme();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    UIState ui_state;
    std::unique_ptr<Style::StyledNode> style_root = nullptr;
    std::unique_ptr<Layout::LayoutBox> layout_root = nullptr;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (ui_state.load_requested && ui_state.url_to_load.length() > 0) {
            auto resource = network_process.request(ui_state.url_to_load);
            std::string html_source = "<h1>Error: Page failed to load.</h1>";
            if (resource) {
                html_source = resource->data;
            }
            
            std::string css_source = R"(
                h1, h2, h3, h4, h5, h6, p, li, dt, dd, a { display: block; color: #cccccc; }
                h1 { font-size: 32px; margin-top: 20px; margin-bottom: 20px; color: #00ff00; }
                h2 { font-size: 28px; margin-top: 15px; margin-bottom: 15px; color: #00dd00; }
                h3 { font-size: 24px; margin-top: 12px; margin-bottom: 12px; color: #00bb00; }
                p, li, dt, dd { font-size: 16px; margin-bottom: 10px; }
                a { color: #8888ff; }
            )";

            HTML::Parser html_parser(html_source);
            auto dom_nodes = html_parser.parse_nodes();
            if (!dom_nodes.empty()) {
                CSS::Parser css_parser(css_source);
                auto stylesheet = css_parser.parse_stylesheet();
                style_root = Style::style_tree(dom_nodes[0].get(), stylesheet);
            } else {
                style_root = nullptr;
            }
            
            ui_state.load_requested = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Browser", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) { if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window, true); } ImGui::EndMenu(); }
            if (ImGui::BeginMenu("Help")) { if (ImGui::MenuItem("About")) { ui_state.show_about_window = true; } ImGui::EndMenu(); }
            ImGui::EndMenuBar();
        }
        
        if (ImGui::Button("Reload")) {
            ui_state.url_to_load = ui_state.address_bar_text;
            ui_state.load_requested = true;
        }
        ImGui::SameLine();
        ImGui::PushItemWidth(-1);
        if (ImGui::InputText("##AddressBar", ui_state.address_bar_text, sizeof(ui_state.address_bar_text), ImGuiInputTextFlags_EnterReturnsTrue)) {
            ui_state.url_to_load = ui_state.address_bar_text;
            ui_state.load_requested = true;
        }
        ImGui::PopItemWidth();

        ImGui::BeginChild("ContentView", ImVec2(0, 0), true);
        
        if (style_root) {
            Layout::Dimensions viewport;
            viewport.width = ImGui::GetContentRegionAvail().x;
            viewport.height = ImGui::GetContentRegionAvail().y;
            layout_root = Layout::layout_tree(*style_root, viewport);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 viewport_pos = ImGui::GetCursorScreenPos();
            render_layout_box(layout_root.get(), draw_list, viewport_pos);
        }

        ImGui::EndChild();
        ImGui::End();

        if (ui_state.show_about_window) {
            ImGui::Begin("About Netscape Matrix", &ui_state.show_about_window, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("A C++ Browser by an intrepid developer.");
            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.86f, 0.86f, 0.86f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <functional> // For std::function

// Graphics and Windowing
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Our Engine and other components
#include "html_parser.h"
#include "css_parser.h"
#include "style.h"
#include "layout.h"
#include "content_blocker.h"
#include "network_process.h"
#include "javascript.h"

struct UIState {
    char address_bar_text[1024] = "test.html";
    bool show_dev_console = true;
    bool show_about_window = false;
};

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

void render_layout_box(const Layout::LayoutBox* box, ImDrawList* draw_list) {
    if (!box || !box->styled_node) return;
    if (box->box_type == Layout::BoxType::Block) {
        auto it = box->styled_node->specified_values.find("background-color");
        if (it != box->styled_node->specified_values.end()) {
            if (const CSS::Color* color = std::get_if<CSS::Color>(&it->second)) {
                ImVec2 p_min(box->dimensions.x, box->dimensions.y);
                ImVec2 p_max(box->dimensions.x + box->dimensions.width, box->dimensions.y + box->dimensions.height);
                draw_list->AddRectFilled(p_min, p_max, IM_COL32(color->r, color->g, color->b, color->a));
            }
        }
    }
    if (box->box_type == Layout::BoxType::Anonymous && box->styled_node->node->type == DOM::NodeType::Text) {
        auto it = box->styled_node->specified_values.find("color");
        if (it != box->styled_node->specified_values.end()) {
            if (const CSS::Color* color = std::get_if<CSS::Color>(&it->second)) {
                ImVec2 text_pos(box->dimensions.x + 5, box->dimensions.y + 2);
                draw_list->AddText(text_pos, IM_COL32(color->r, color->g, color->b, color->a), box->styled_node->node->text_data.c_str());
            }
        }
    }
    for (const auto& child : box->children) {
        render_layout_box(child.get(), draw_list);
    }
}

int main() {
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

    std::string html_source = R"(
        <div id="main">
            <h1>Netscape Matrix</h1>
            <p>Check the dev console for JS output!</p>
            <script>
                console.log("Hello from JavaScript!");
                let x = 10;
                let y = 20;
                console.log("The answer is", x + y);
            </script>
        </div>
    )";
    std::string css_source = R"(
        div { display: block; padding: 20px; background-color: #333333; }
        h1 { display: block; height: 40px; color: #00ff00; }
        p { display: block; height: 30px; color: #cccccc; }
        script { display: none; }
    )";

    HTML::Parser html_parser(html_source);
    auto dom_root = html_parser.parse_nodes()[0].get();
    
    std::function<void(const DOM::Node*)> execute_scripts = 
        [&](const DOM::Node* node) {
        if (node->type == DOM::NodeType::Element && node->element_data.tag_name == "script") {
            if (!node->children.empty() && node->children[0]->type == DOM::NodeType::Text) {
                js_engine.run_script(node->children[0]->text_data);
            }
        }
        for (const auto& child : node->children) {
            execute_scripts(child.get());
        }
    };
    execute_scripts(dom_root);

    CSS::Parser css_parser(css_source);
    auto stylesheet = css_parser.parse_stylesheet();
    auto style_root = Style::style_tree(dom_root, stylesheet);
    std::unique_ptr<Layout::LayoutBox> layout_root = nullptr;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Browser", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(window, true); }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Bookmarks")) {
                if (ImGui::MenuItem("Search Engine")) {}
                if (ImGui::MenuItem("Cool Site")) {}
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("Toggle Dev Console")) { ui_state.show_dev_console = !ui_state.show_dev_console; }
                if (ImGui::MenuItem("About")) { ui_state.show_about_window = true; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        
        if (ImGui::BeginTabBar("##Tabs")) {
            if (ImGui::BeginTabItem("test.html")) {
                if (ImGui::Button("Back")) {}
                ImGui::SameLine();
                if (ImGui::Button("Forward")) {}
                ImGui::SameLine();
                if (ImGui::Button("Reload")) {}
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                ImGui::InputText("##AddressBar", ui_state.address_bar_text, sizeof(ui_state.address_bar_text));
                ImGui::PopItemWidth();

                ImGui::BeginChild("ContentView", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true);
                Layout::Dimensions viewport;
                viewport.width = ImGui::GetContentRegionAvail().x;
                viewport.height = ImGui::GetContentRegionAvail().y;
                layout_root = Layout::layout_tree(*style_root, viewport);
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 viewport_pos = ImGui::GetCursorScreenPos();
                render_layout_box(layout_root.get(), draw_list);
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("+")) { ImGui::EndTabItem(); }
            ImGui::EndTabBar();
        }

        ImGui::End();

        if (ui_state.show_dev_console) {
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 200), ImGuiCond_Once);
            ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 200), ImGuiCond_Once);
            ImGui::Begin("Developer Console", &ui_state.show_dev_console);
            for (const auto& log : js_engine.get_logs()) {
                ImGui::TextUnformatted(log.c_str());
            }
            ImGui::End();
        }
        
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
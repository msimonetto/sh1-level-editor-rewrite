#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>

#include "mgmt/Workspace.h"
#include "gui/Window.h"

Window::Window(
    Workspace* workspace,
    WindowConfig config)
    :
    workspace_(workspace), config_(std::move(config)) {
    Init();
}

Window::~Window() {
    rlImGuiShutdown();
    CloseWindow();
}

void Window::Init() {
    InitWindow(config_.width, config_.height, config_.title.c_str());
    SetTargetFPS(60);

    rlImGuiSetup(true);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void Window::Open() {
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);
        
        rlImGuiBegin();
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
        
        RenderPanels();

        rlImGuiEnd();

        EndDrawing();
    }
}

void Window::RenderPanels() {
    ImGui::Begin("Test");
    ImGui::Text("Test");
    ImGui::End();
}
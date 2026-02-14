#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>
#include <unistd.h>

static float g_ui_scale = 1.7f;
static bool g_collapsed = false;  // 悬浮球状态
static ImVec2 g_ball_pos = ImVec2(150, 250);

void init_My_drawdata() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    
    // 问号修复方案
    const char* fonts[] = {"/system/fonts/NotoSansSC-Regular.otf", "/system/fonts/DroidSansFallback.ttf", "/system/fonts/NotoSansCJK-Regular.ttc"};
    for (auto p : fonts) {
        if (access(p, R_OK) == 0) {
            io.Fonts->AddFontFromFileTTF(p, 32.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
            break;
        }
    }
    io.Fonts->Build();
    io.FontGlobalScale = g_ui_scale;
}

// 绘制高端流光渐变条
void RenderGlowLine() {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float t = (float)ImGui::GetTime();
    
    // HSV动态色彩
    ImU32 c1 = ImColor::HSV(fmodf(t * 0.12f, 1.0f), 0.7f, 0.9f);
    ImU32 c2 = ImColor::HSV(fmodf(t * 0.12f + 0.3f, 1.0f), 0.7f, 0.9f);
    
    dl->AddRectFilledMultiColor(p, ImVec2(p.x + w, p.y + 3), c1, c2, c2, c1);
    ImGui::Dummy(ImVec2(0, 12));
}

void Layout_tick_UI(bool *p_open) {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* bg_dl = ImGui::GetBackgroundDrawList();

    if (g_collapsed) {
        // --- 悬浮球模式：带呼吸灯的高端圆标 ---
        ImGui::SetNextWindowPos(g_ball_pos);
        ImGui::SetNextWindowSize(ImVec2(100, 100));
        ImGui::Begin("Ball", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
        
        ImVec2 center = ImVec2(g_ball_pos.x + 50, g_ball_pos.y + 50);
        float pulse = sinf((float)ImGui::GetTime() * 3.5f) * 4.0f;
        
        // 绘制质感圆球
        bg_dl->AddCircleFilled(center, 35 + pulse, IM_COL32(255, 45, 85, 180), 64);
        bg_dl->AddCircle(center, 38 + pulse, IM_COL32(255, 255, 255, 220), 64, 2.5f);
        bg_dl->AddText(NULL, 36.0f, ImVec2(center.x - 12, center.y - 18), IM_COL32(255, 255, 255, 255), "G");
        
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) g_collapsed = false;
        if (ImGui::IsMouseDragging(0) && ImGui::IsWindowHovered()) {
            g_ball_pos.x += io.MouseDelta.x; g_ball_pos.y += io.MouseDelta.y;
        }
        ImGui::End();
        return;
    }

    // --- 高端菜单模式 ---
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 14.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.WindowBorderSize = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.08f, 0.97f);
    style.Colors[ImGuiCol_Border] = ImColor(255, 45, 85, 200);

    ImGui::SetNextWindowSize(ImVec2(750, 480), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("GALAXY_HUB", p_open, ImGuiWindowFlags_NoTitleBar)) {
        RenderGlowLine();

        // 自定义 Header
        ImGui::TextColored(ImColor(255, 45, 85), "GALAXY PREMIUM | %0.1f FPS", io.Framerate);
        ImGui::SameLine(ImGui::GetWindowWidth() - 95);
        if (ImGui::Button("收起菜单", ImVec2(85, 38))) g_collapsed = true;
        
        ImGui::Separator();
        ImGui::Spacing();

        // 双栏功能区
        ImGui::Columns(2, NULL, false);
        ImGui::BeginChild("AimSection", ImVec2(0, 320), true);
        {
            ImGui::TextColored(ImColor(100, 200, 255), "⚔ 战斗辅助");
            ImGui::Separator();
            static bool aim = false, recoil = false;
            ImGui::Checkbox(" 全自动智能锁定", &aim);
            ImGui::Checkbox(" 武器零后坐力", &recoil);
            if (aim) {
                ImGui::Indent(25);
                static float fov = 120.0f;
                ImGui::SliderFloat("瞄准范围", &fov, 30, 500, "%.0f px");
                ImGui::Unindent(25);
            }
        }
        ImGui::EndChild();

        ImGui::NextColumn();

        ImGui::BeginChild("VisualSection", ImVec2(0, 320), true);
        {
            ImGui::TextColored(ImColor(100, 255, 150), "👁 视觉渲染");
            ImGui::Separator();
            static bool esp = false, line = false;
            ImGui::Checkbox(" 显示玩家方框", &esp);
            ImGui::Checkbox(" 显示骨骼射线", &line);
            
            ImGui::Spacing();
            ImGui::TextDisabled("系统偏好");
            if (ImGui::SliderFloat("菜单缩放", &g_ui_scale, 1.0f, 2.5f)) {
                io.FontGlobalScale = g_ui_scale;
            }
        }
        ImGui::EndChild();
        ImGui::Columns(1);
    }
    ImGui::End();
}

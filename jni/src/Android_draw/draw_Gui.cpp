#include "imgui.h"
#include "imgui_internal.h"
#include <math.h>
#include <unistd.h>

static float g_ui_scale = 1.7f;
static bool g_collapsed = false;  
static ImVec2 g_ball_pos = ImVec2(200, 400);

void init_My_drawdata() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    
    // Bug 修复：增加更多备选路径，并提高字号防止锯齿
    const char* fonts[] = {
        "/system/fonts/NotoSansSC-Regular.otf", 
        "/system/fonts/DroidSansFallback.ttf",
        "/system/fonts/NotoSansCJK-Regular.ttc",
        "/system/fonts/Roboto-Regular.ttf" // 最后的保底
    };
    for (auto p : fonts) {
        if (access(p, R_OK) == 0) {
            // 使用更大字号加载，由 FontGlobalScale 缩小，画质更清晰
            io.Fonts->AddFontFromFileTTF(p, 40.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
            break;
        }
    }
    io.Fonts->Build();
    io.FontGlobalScale = g_ui_scale;
}

// 高端科技渐变条 (修复了错位问题)
void RenderGlowBar() {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float t = (float)ImGui::GetTime();
    
    ImU32 c1 = ImColor::HSV(fmodf(t * 0.15f, 1.0f), 0.7f, 1.0f);
    ImU32 c2 = ImColor::HSV(fmodf(t * 0.15f + 0.3f, 1.0f), 0.7f, 1.0f);
    
    dl->AddRectFilledMultiColor(ImVec2(p.x, p.y), ImVec2(p.x + w, p.y + 3), c1, c2, c2, c1);
    ImGui::Dummy(ImVec2(0, 15)); 
}

void Layout_tick_UI(bool *p_open) {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* bg_dl = ImGui::GetBackgroundDrawList();

    if (g_collapsed) {
        // --- 修复版悬浮圆标：完美居中 ---
        ImGui::SetNextWindowPos(g_ball_pos);
        ImGui::SetNextWindowSize(ImVec2(100, 100));
        ImGui::Begin("Ball", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
        
        ImVec2 center = ImVec2(g_ball_pos.x + 50, g_ball_pos.y + 50);
        float pulse = sinf((float)ImGui::GetTime() * 4.0f) * 4.0f;
        
        bg_dl->AddCircleFilled(center, 35 + pulse, IM_COL32(255, 40, 80, 160), 64);
        bg_dl->AddCircle(center, 38 + pulse, IM_COL32(255, 255, 255, 180), 64, 2.0f);
        
        // 强制居中绘制字母 G
        char icon[] = "G";
        ImVec2 txt_sz = ImGui::CalcTextSize(icon);
        bg_dl->AddText(ImVec2(center.x - txt_sz.x/2, center.y - txt_sz.y/2), IM_COL32(255, 255, 255, 255), icon);
        
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) g_collapsed = false;
        if (ImGui::IsMouseDragging(0) && ImGui::IsWindowHovered()) {
            g_ball_pos.x += io.MouseDelta.x; g_ball_pos.y += io.MouseDelta.y;
        }
        ImGui::End();
        return;
    }

    // --- 修复版主菜单：解决窗口过矮、按钮截断问题 ---
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f;
    style.FramePadding = ImVec2(15, 12); // 解决点击范围小和文字一半的问题
    style.ItemSpacing = ImVec2(10, 15);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.08f, 0.98f);
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.40f);

    ImGui::SetNextWindowSize(ImVec2(800, 520), ImGuiCond_Always);
    if (ImGui::Begin("GALAXY_FIXED", p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)) {
        RenderGlowBar();

        // 顶部 Header：增加按钮宽度防止文字消失
        ImGui::TextColored(ImColor(255, 45, 85), "GALAXY PREMIUM | %0.1f FPS", io.Framerate);
        ImGui::SameLine(ImGui::GetWindowWidth() - 140); 
        if (ImGui::Button("收起菜单", ImVec2(120, 45))) g_collapsed = true; 
        
        ImGui::Separator();
        ImGui::Spacing();

        // 解决错位：使用两列等高 Child
        float child_h = 360.0f; 
        ImGui::Columns(2, "Content", false);
        
        // 左卡片
        ImGui::BeginChild("AimCard", ImVec2(0, child_h), true);
        ImGui::TextColored(ImColor(0, 180, 255), "⚔ 战斗修改");
        ImGui::Separator();
        static bool b1, b2;
        ImGui::Checkbox(" 自动锁定目标", &b1);
        ImGui::Checkbox(" 无后坐力模式", &b2);
        ImGui::EndChild();

        ImGui::NextColumn();

        // 右卡片
        ImGui::BeginChild("SysCard", ImVec2(0, child_h), true);
        ImGui::TextColored(ImColor(0, 255, 150), "⚙ 系统设置");
        ImGui::Separator();
        ImGui::Text("全局缩放");
        if (ImGui::SliderFloat("##ScaleSlider", &g_ui_scale, 1.0f, 2.5f)) {
            io.FontGlobalScale = g_ui_scale;
        }
        if (ImGui::Button("清理残留缓存", ImVec2(-1, 50))) { /* 逻辑 */ }
        ImGui::EndChild();
        
        ImGui::Columns(1);
    }
    ImGui::End();
}

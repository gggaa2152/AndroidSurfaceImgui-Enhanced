#include "draw.h"
#include "My_font/zh_Font.h"
#include "My_font/fontawesome-solid.h"
#include "imgui_internal.h"
#include <math.h>

// FontAwesome 图标宏
#ifndef ICON_MIN_FA
#define ICON_MIN_FA 0xf000
#define ICON_MAX_FA 0xf8ff
#endif
#define ICON_FA_BOLT "\xef\x83\xa7"
#define ICON_FA_SHIELD "\xef\x84\xb2"
#define ICON_FA_SKULL "\xef\x95\x8c"
#define ICON_FA_CROSSHAIRS "\xef\x81\x9b"
#define ICON_FA_EYE "\xef\x81\xae"
#define ICON_FA_CUBE "\xef\x9b\x91"

// 状态控制
static bool g_collapsed = false;         // 是否收缩成圆
static ImVec2 g_float_pos = ImVec2(100, 200); 
static float g_menu_scale = 1.6f;

// 引用外部变量
extern std::unique_ptr<AndroidImgui> graphics;
extern ANativeWindow *window;
extern android::ANativeWindowCreator::DisplayInfo displayInfo;
extern int native_window_screen_x, native_window_screen_y;

// 字体指针
ImFont* zh_font = NULL;
ImFont* icon_font_2 = NULL;

// 修复后的加载函数：确保中文字体优先，图标合并其后
bool M_Android_LoadFont(float SizePixels) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    
    // 1. 先加载系统中文
    ImGui::My_Android_LoadSystemFont(SizePixels);
    zh_font = io.Fonts->Fonts[0];
    
    // 2. 合并 FontAwesome 图标
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true; // 关键：合并到中文
    icons_config.PixelSnapH = true;
    icons_config.GlyphOffset.y = 2.0f;
    
    icon_font_2 = io.Fonts->AddFontFromMemoryCompressedTTF(
        (const void*)&font_awesome_solid_compressed_data,
        sizeof(font_awesome_solid_compressed_data),
        SizePixels, &icons_config, icons_ranges);
    
    io.Fonts->Build();
    io.FontDefault = zh_font;
    io.FontGlobalScale = g_menu_scale;
    return true;
}

void init_My_drawdata() {
    ImGui::StyleColorsDark();
    M_Android_LoadFont(32.0f); // 初始字号
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 12.0f;
    style.FrameRounding = 6.0f;
    style.WindowBorderSize = 0.0f;
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.09f, 0.94f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.15f, 0.35f, 1.00f);
}

// 顶部渐变流光线渲染
void DrawTechGradient() {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x;
    float t = (float)ImGui::GetTime();
    
    ImU32 col1 = ImColor::HSV(fmodf(t * 0.15f, 1.0f), 0.7f, 1.0f);
    ImU32 col2 = ImColor::HSV(fmodf(t * 0.15f + 0.3f, 1.0f), 0.7f, 1.0f);
    
    dl->AddRectFilledMultiColor(p, ImVec2(p.x + w, p.y + 2), col1, col2, col2, col1);
    ImGui::Dummy(ImVec2(0, 10));
}

void Layout_tick_UI(bool *main_thread_flag) {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* bg_dl = ImGui::GetBackgroundDrawList();

    if (g_collapsed) {
        // ===================== 模式 A：悬浮圆标模式 =====================
        ImGui::SetNextWindowPos(g_float_pos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(100, 100));
        ImGui::Begin("Ball", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
        
        ImVec2 center = ImVec2(g_float_pos.x + 50, g_float_pos.y + 50);
        float pulse = sinf((float)ImGui::GetTime() * 4.0f) * 4.0f;
        
        bg_dl->AddCircleFilled(center, 35 + pulse, IM_COL32(255, 30, 80, 160), 64);
        bg_dl->AddCircle(center, 38 + pulse, IM_COL32(255, 255, 255, 200), 64, 2.0f);
        bg_dl->AddText(NULL, 35.0f, ImVec2(center.x - 14, center.y - 18), IM_COL32(255, 255, 255, 255), "G");
        
        if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0)) g_collapsed = false;
        if (ImGui::IsMouseDragging(0) && ImGui::IsWindowHovered()) {
            g_float_pos.x += io.MouseDelta.x; g_float_pos.y += io.MouseDelta.y;
        }
        ImGui::End();
        return;
    }

    // ===================== 模式 B：展开的高端菜单 =====================
    io.FontGlobalScale = g_menu_scale;
    ImGui::SetNextWindowSize(ImVec2(750, 500), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("GALAXY_INTERNAL", main_thread_flag, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse)) {
        DrawTechGradient();
        
        // 顶部 Header
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.4f, 1.0f), ICON_FA_BOLT " GALAXY INTERNAL");
        ImGui::SameLine(ImGui::GetWindowWidth() - 150);
        ImGui::TextDisabled("FPS: %.0f", io.Framerate);
        ImGui::SameLine(ImGui::GetWindowWidth() - 50);
        if (ImGui::Button("-", ImVec2(35, 35))) g_collapsed = true;
        
        ImGui::Separator();

        // 左右布局
        ImGui::Columns(2, "MainCols", false);
        ImGui::SetColumnWidth(0, 320 * g_menu_scale);

        // 左栏：功能卡片
        ImGui::BeginChild("CombatCard", ImVec2(0, 350 * g_menu_scale), true);
        {
            ImGui::TextColored(ImVec4(0.6f, 0.8f, 1.0f, 1.0f), ICON_FA_CROSSHAIRS " 核心战斗");
            ImGui::Separator();
            static bool aim = false, recoil = false;
            ImGui::Checkbox(" 开启智能自瞄", &aim);
            ImGui::Checkbox(" 武器零后坐力", &recoil);
            
            if (aim) {
                ImGui::Indent(20);
                static int fov = 90;
                ImGui::SliderInt("自瞄范围", &fov, 30, 180, "%d°");
                ImGui::Unindent(20);
            }
        }
        ImGui::EndChild();

        ImGui::NextColumn();

        // 右栏：设置区域
        ImGui::BeginChild("SettingCard", ImVec2(0, 350 * g_menu_scale), true);
        {
            ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), ICON_FA_EYE " 视觉渲染");
            ImGui::Separator();
            static bool esp = false;
            ImGui::Checkbox(" 玩家方框绘制", &esp);
            
            ImGui::Spacing(); ImGui::Spacing();
            ImGui::TextDisabled("系统偏好");
            ImGui::SliderFloat("菜单缩放", &g_menu_scale, 1.0f, 2.5f);
            if (ImGui::Button("深度清理内存", ImVec2(-1, 50))) { /* 逻辑 */ }
        }
        ImGui::EndChild();
        ImGui::Columns(1);
    }
    ImGui::End();
}

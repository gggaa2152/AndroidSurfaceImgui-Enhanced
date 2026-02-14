#include "draw.h"
#include "My_font/zh_Font.h"
#include "My_font/fontawesome-solid.h"

#ifndef ICON_MIN_FA
#define ICON_MIN_FA 0xf000
#define ICON_MAX_FA 0xf8ff
#endif

// ===== 这些在 main.cpp 中有定义 =====
extern bool permeate_record;
extern std::unique_ptr<AndroidImgui> graphics;
extern ANativeWindow *window;
extern android::ANativeWindowCreator::DisplayInfo displayInfo;
extern int abs_ScreenX, abs_ScreenY;
extern int native_window_screen_x, native_window_screen_y;

// ===== 这些在 main.cpp 中没有，在 draw_Gui.cpp 中定义 =====
bool permeate_record_ini = false;
struct Last_ImRect LastCoordinate = {0, 0, 0, 0};
ImGuiWindow *g_window = NULL;

ImFont* zh_font = NULL;
ImFont* icon_font_2 = NULL;

bool M_Android_LoadFont(float SizePixels) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    
    ImGui::My_Android_LoadSystemFont(SizePixels);
    zh_font = io.Fonts->Fonts[0];
    
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.SizePixels = SizePixels;
    icons_config.GlyphOffset.y = 4.0f;
    
    icon_font_2 = io.Fonts->AddFontFromMemoryCompressedTTF(
        (const void*)&font_awesome_solid_compressed_data,
        sizeof(font_awesome_solid_compressed_data),
        0.0f, &icons_config, icons_ranges);
    
    io.Fonts->Build();
    io.FontDefault = zh_font;
    return true;
}

void init_My_drawdata() {
    ImGui::StyleColorsDark();
    ImGui::My_Android_LoadSystemFont(32.0f);
    M_Android_LoadFont(32.0f);
}

// screen_config() 和 drawBegin() 在 main.cpp 中

void Layout_tick_UI(bool *main_thread_flag) {
    // 窗口位置记忆（使用本文件定义的变量）
    if (permeate_record_ini) {
        ImGui::SetWindowPos({LastCoordinate.Pos_x, LastCoordinate.Pos_y});
        ImGui::SetWindowSize({LastCoordinate.Size_x, LastCoordinate.Size_y});
        permeate_record_ini = false;   
    }
    
    // ===== 唯一的滑动开关 =====
    ImGui::Begin("自定义开关", main_thread_flag, 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    
    static bool bSwitch = false;
    
    ImGui::Text("触摸自瞄");
    ImGui::SameLine(120);
    
    ImVec2 p = ImGui::GetCursorScreenPos();
    float height = 24.0f;
    float width = 48.0f;
    float radius = height * 0.5f;
    
    ImU32 bgColor = bSwitch ? IM_COL32(100, 200, 100, 255) : IM_COL32(80, 80, 80, 255);
    ImGui::GetWindowDrawList()->AddRectFilled(
        p, ImVec2(p.x + width, p.y + height), bgColor, radius
    );
    
    float thumbPos = bSwitch ? p.x + width - height : p.x;
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(thumbPos + radius, p.y + radius), radius - 3, IM_COL32_WHITE
    );
    
    ImGui::InvisibleButton("##toggle", ImVec2(width, height));
    if (ImGui::IsItemClicked()) {
        bSwitch = !bSwitch;
    }
    
    ImGui::SameLine(180);
    ImGui::Text(bSwitch ? "开启" : "关闭");
    
    ImGui::End();
    
    // 记录窗口位置
    g_window = ImGui::GetCurrentWindow();
}

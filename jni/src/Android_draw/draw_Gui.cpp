#include "draw.h"
#include "My_font/zh_Font.h"
#include "My_font/fontawesome-solid.h"

#ifndef ICON_MIN_FA
#define ICON_MIN_FA 0xf000
#define ICON_MAX_FA 0xf8ff
#endif

// ===== 在 main.cpp 中定义的变量 =====
extern bool permeate_record;
extern std::unique_ptr<AndroidImgui> graphics;
extern ANativeWindow *window;
extern android::ANativeWindowCreator::DisplayInfo displayInfo;
extern int abs_ScreenX, abs_ScreenY;
extern int native_window_screen_x, native_window_screen_y;

// ===== 在 draw_Gui.cpp 中定义的变量 =====
bool permeate_record_ini = false;
struct Last_ImRect LastCoordinate = {0, 0, 0, 0};
ImGuiWindow *g_window = NULL;

// ===== 缩放相关变量 =====
static float g_global_scale = 1.0f;
static bool g_show_settings = false;

ImFont* zh_font = NULL;
ImFont* icon_font_2 = NULL;

bool M_Android_LoadFont(float SizePixels) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    
    ImGui::My_Android_LoadSystemFont(SizePixels * g_global_scale);
    zh_font = io.Fonts->Fonts[0];
    
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.SizePixels = SizePixels * g_global_scale;
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
    M_Android_LoadFont(32.0f);
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.FramePadding = ImVec2(10 * g_global_scale, 8 * g_global_scale);
    style.ItemSpacing = ImVec2(12 * g_global_scale, 14 * g_global_scale);
    style.WindowPadding = ImVec2(16 * g_global_scale, 16 * g_global_scale);
    style.FrameRounding = 6.0f * g_global_scale;
}

void Layout_tick_UI(bool *main_thread_flag) {
    // 窗口位置记忆
    if (permeate_record_ini) {
        ImGui::SetWindowPos({LastCoordinate.Pos_x, LastCoordinate.Pos_y});
        ImGui::SetWindowSize({LastCoordinate.Size_x, LastCoordinate.Size_y});
        permeate_record_ini = false;   
    }
    
    // ===== 唯一的窗口 =====
    ImGui::Begin("自定义开关", main_thread_flag, 
        ImGuiWindowFlags_NoCollapse);
    
    // ----- 标题栏 + 设置按钮 -----
    ImGui::Text("控制中心");
    ImGui::SameLine(ImGui::GetWindowWidth() - 60);
    if (ImGui::Button("⚙")) {
        g_show_settings = !g_show_settings;
    }
    ImGui::Separator();
    ImGui::Spacing();
    
    // ----- 设置面板（缩放控制）-----
    if (g_show_settings) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.00f), "显示设置");
        ImGui::Separator();
        
        static int scale_percent = 100;
        ImGui::Text("界面缩放");
        ImGui::SameLine(100 * g_global_scale);
        if (ImGui::SliderInt("##scale", &scale_percent, 80, 150, "%d%%")) {
            g_global_scale = scale_percent / 100.0f;
        }
        if (ImGui::IsItemDeactivated()) {
            init_My_drawdata();
        }
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }
    
    // ----- 滑动开关 -----
    static bool bSwitch = false;
    
    ImGui::Text("触摸自瞄");
    ImGui::SameLine(120 * g_global_scale);
    
    ImVec2 p = ImGui::GetCursorScreenPos();
    float height = 24.0f * g_global_scale;
    float width = 48.0f * g_global_scale;
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
    
    ImGui::SameLine(180 * g_global_scale);
    ImGui::Text(bSwitch ? "开启" : "关闭");
    
    // ----- FPS 显示 -----
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
    
    // ----- 右下角缩放提示 -----
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "右下角可拖动缩放");
    
    ImGui::End();
    
    // 记录窗口位置
    g_window = ImGui::GetCurrentWindow();
    
    // ⚠️ 绝对没有 ShowDemoWindow 或 ShowMetricsWindow
}

#include "draw.h"
#include "My_font/zh_Font.h"
#include "My_font/fontawesome-solid.h"

// FontAwesome 图标宏定义
#ifndef ICON_MIN_FA
#define ICON_MIN_FA 0xf000
#define ICON_MAX_FA 0xf8ff
#endif
#define ICON_FA_BOLT "\xef\x83\xa7"
#define ICON_FA_SHIELD "\xef\x84\xb2"
#define ICON_FA_SKULL "\xef\x95\x8c"
#define ICON_FA_CROSSHAIRS "\xef\x81\x9b"
#define ICON_FA_EYE "\xef\x81\xae"

// ===== 只留 extern 声明，不要重复定义！=====
extern bool permeate_record;
extern bool permeate_record_ini;
extern struct Last_ImRect LastCoordinate;
extern std::unique_ptr<AndroidImgui> graphics;
extern ANativeWindow *window;
extern android::ANativeWindowCreator::DisplayInfo displayInfo;
extern ImGuiWindow *g_window;
extern int abs_ScreenX, abs_ScreenY;
extern int native_window_screen_x, native_window_screen_y;

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
    
    ImGuiStyle& style = ImGui::GetStyle();
    style.FramePadding = ImVec2(10, 8);
    style.ItemSpacing = ImVec2(12, 14);
    style.WindowPadding = ImVec2(16, 16);
    style.FrameRounding = 6.0f;
}

// ⚠️ 注意：这里不再定义 screen_config() 和 drawBegin()
// 它们已经在 main.cpp 中定义了

void Layout_tick_UI(bool *main_thread_flag) {
    static int style_idx = 0;
    
    // 窗口位置记忆
    if (::permeate_record_ini) {
        ImGui::SetWindowPos({LastCoordinate.Pos_x, LastCoordinate.Pos_y});
        ImGui::SetWindowSize({LastCoordinate.Size_x, LastCoordinate.Size_y});
        permeate_record_ini = false;   
    }
    
    // ===== 主菜单窗口 =====
    ImGui::Begin("✨ 银河外挂", main_thread_flag, 
        ImGuiWindowFlags_NoCollapse);
    
    // ----- 标题栏 -----
    ImGui::PushFont(icon_font_2);
    ImGui::TextColored(ImVec4(0.65f, 0.85f, 1.00f, 1.00f), "%s 控制中心", ICON_FA_BOLT);
    ImGui::PopFont();
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Spacing();
    
    // ----- 基础设置 -----
    ImGui::Text("基础设置");
    ImGui::Separator();
    
    ImGui::Combo("##主题", &style_idx, "白色主题\0深色主题\0经典主题\0");
    if (ImGui::IsItemDeactivated()) {
        switch (style_idx) {
            case 0: ImGui::StyleColorsLight(); break;
            case 1: ImGui::StyleColorsDark(); break;
            case 2: ImGui::StyleColorsClassic(); break;
        }
    }
    
    ImGui::Checkbox("过录制", &::permeate_record);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // ===== 战斗辅助 =====
    ImGui::PushFont(icon_font_2);
    ImGui::TextColored(ImVec4(0.65f, 0.85f, 1.00f, 1.00f), "%s 战斗辅助", ICON_FA_SHIELD);
    ImGui::PopFont();
    ImGui::Separator();
    
    static bool god_mode = false;
    static bool aimbot = false;
    static bool esp = false;
    
    // 第一行：秒杀 + 无敌
    if (ImGui::Button(ICON_FA_SKULL " 秒杀", ImVec2(130, 42))) {
        // 秒杀代码
    }
    ImGui::SameLine(160);
    ImGui::Checkbox(ICON_FA_SHIELD " 无敌", &god_mode);
    
    // ===== 第二行：触摸自瞄（带自定义滑动开关）=====
    ImGui::Text("触摸自瞄");
    ImGui::SameLine(120);
    
    ImVec2 p = ImGui::GetCursorScreenPos();
    float height = 24.0f;
    float width = 48.0f;
    float radius = height * 0.5f;
    
    // 画背景
    ImU32 bgColor = aimbot ? IM_COL32(100, 200, 100, 255) : IM_COL32(80, 80, 80, 255);
    ImGui::GetWindowDrawList()->AddRectFilled(
        p, ImVec2(p.x + width, p.y + height), bgColor, radius
    );
    
    // 画滑块
    float thumbPos = aimbot ? p.x + width - height : p.x;
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(thumbPos + radius, p.y + radius), radius - 3, IM_COL32_WHITE
    );
    
    ImGui::InvisibleButton("##aimbot_toggle", ImVec2(width, height));
    if (ImGui::IsItemClicked()) {
        aimbot = !aimbot;
        if (aimbot) {
            __android_log_print(ANDROID_LOG_INFO, "PureElf", "触摸自瞄开启");
        } else {
            __android_log_print(ANDROID_LOG_INFO, "PureElf", "触摸自瞄关闭");
        }
    }
    
    ImGui::SameLine(200);
    ImGui::Checkbox(ICON_FA_EYE " 透视", &esp);
    
    // 自瞄二级菜单
    if (aimbot) {
        ImGui::Indent(24);
        ImGui::Separator();
        ImGui::Text(ICON_FA_CROSSHAIRS " 自瞄参数");
        
        static float smooth = 1.2f;
        static int fov = 90;
        ImGui::SliderFloat("平滑度", &smooth, 0.5f, 3.0f, "%.1f倍");
        ImGui::SliderInt("范围", &fov, 30, 180, "%d°");
        
        ImGui::Unindent(24);
        ImGui::Spacing();
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::End();
    
    // 记录窗口位置
    g_window = ImGui::GetCurrentWindow();
}

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
#define ICON_FA_GUN "\xef\x86\x9b"
#define ICON_FA_GAUGE "\xef\x98\xa4"
#define ICON_FA_CHART_LINE "\xef\x88\x81"
#define ICON_FA_HEART "\xef\x80\x84"
#define ICON_FA_CLOSE "\xef\x80\x8d"

bool permeate_record = false;
bool permeate_record_ini = false;
struct Last_ImRect LastCoordinate = {0, 0, 0, 0};

std::unique_ptr<AndroidImgui> graphics;
ANativeWindow *window = NULL; 
android::ANativeWindowCreator::DisplayInfo displayInfo;
ImGuiWindow *g_window = NULL;
int abs_ScreenX = 0, abs_ScreenY = 0;
int native_window_screen_x = 0, native_window_screen_y = 0;

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
    icons_config.GlyphOffset.y = 3.0f;
    
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
    M_Android_LoadFont(28.0f);
}

void screen_config() {
    ::displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
}

void drawBegin() {
    // 原逻辑保留
}

void Layout_tick_UI(bool *main_thread_flag) {
    // 现代暗黑主题
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(16, 16);
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing = ImVec2(12, 10);
    
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.96f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.45f, 0.25f, 0.65f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.45f, 0.25f, 0.65f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.55f, 0.35f, 0.75f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.15f, 0.55f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 1.00f, 1.00f);
    
    ImGui::Begin("✨ 银河外挂", main_thread_flag, 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    
    // 标题
    ImGui::PushFont(icon_font_2);
    ImGui::TextColored(ImVec4(0.65f, 0.85f, 1.00f, 1.00f), "%s 控制中心", ICON_FA_BOLT);
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Spacing();
    
    // 战斗辅助
    if (ImGui::CollapsingHeader(ICON_FA_SHIELD " 战斗辅助", ImGuiTreeNodeFlags_DefaultOpen)) {
        static bool god_mode = false;
        static bool aimbot = false;
        static bool esp = false;
        
        if (ImGui::Button(ICON_FA_SKULL " 秒杀", ImVec2(120, 40))) {
            // 秒杀代码
        }
        ImGui::SameLine();
        ImGui::Checkbox(ICON_FA_SHIELD " 无敌", &god_mode);
        
        ImGui::Checkbox(ICON_FA_CROSSHAIRS " 自瞄", &aimbot);
        ImGui::SameLine(100);
        ImGui::Checkbox(ICON_FA_EYE " 透视", &esp);
        
        if (aimbot) {
            ImGui::Indent(20);
            ImGui::Separator();
            ImGui::Text(ICON_FA_CROSSHAIRS " 自瞄参数");
            
            static float smooth = 1.2f;
            static int fov = 90;
            ImGui::SliderFloat("平滑度", &smooth, 0.5f, 3.0f, "%.1f");
            ImGui::SliderInt("范围", &fov, 30, 180, "%d°");
            
            ImGui::Unindent(20);
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // 系统状态
    float fps = ImGui::GetIO().Framerate;
    ImGui::Text("FPS: %.1f", fps);
    ImGui::ProgressBar(fps / 120.0f, ImVec2(250, 0), "");
    
    ImGui::End();
}

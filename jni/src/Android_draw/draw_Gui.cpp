#include "draw.h"
#include "My_font/zh_Font.h"
#include "My_font/fontawesome-solid.h"

bool permeate_record = false;
bool permeate_record_ini = false;
struct Last_ImRect LastCoordinate = {0, 0, 0, 0};

std::unique_ptr<AndroidImgui> graphics;
ANativeWindow *window = NULL; 
android::ANativeWindowCreator::DisplayInfo displayInfo;
ImGuiWindow *g_window = NULL;
int abs_ScreenX = 0, abs_ScreenY = 0;
int native_window_screen_x = 0, native_window_screen_y = 0;

TextureInfo Aekun_image{};

ImFont* zh_font = NULL;
ImFont* icon_font_2 = NULL;  // 只用 solid 图标

// ===================== 字体加载（只保留图标）=====================
bool M_Android_LoadFont(float SizePixels) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();
    
    // 1. 加载系统字体（中文支持）
    ImGui::My_Android_LoadSystemFont(SizePixels);
    zh_font = io.Fonts->Fonts[0];
    
    // 2. 合并图标字体
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
    ::Aekun_image = graphics->LoadTextureFromMemory(
        (void*)picture_ZhenAiKun_PNG_H, 
        sizeof(picture_ZhenAiKun_PNG_H));
}

void screen_config() {
    ::displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
}

void drawBegin() {
    // 原逻辑保留
}

// ===================== 高级菜单 =====================
void Layout_tick_UI(bool *main_thread_flag) {
    // ----- 现代暗黑主题（黑紫渐变）-----
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 10.0f;
    style.FrameRounding = 6.0f;
    style.GrabRounding = 6.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.WindowPadding = ImVec2(16, 16);
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing = ImVec2(12, 10);
    
    // 黑紫主题色
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.08f, 0.12f, 0.96f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.16f, 1.00f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.45f, 0.25f, 0.65f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.45f, 0.25f, 0.65f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.55f, 0.35f, 0.75f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.35f, 0.15f, 0.55f, 1.00f);
    style.Colors[ImGuiCol_Text] = ImVec4(0.95f, 0.95f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.55f, 0.75f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.55f, 0.75f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.65f, 0.85f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    
    // ----- 主窗口（固定位置，可拖动）-----
    ImGui::Begin("✨ 银河外挂", main_thread_flag, 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    
    // ===== 标题栏（带图标）=----
    ImGui::PushFont(icon_font_2);
    ImGui::TextColored(ImVec4(0.65f, 0.85f, 1.00f, 1.00f), "%s 控制中心", ICON_FA_BOLT);
    ImGui::PopFont();
    ImGui::Separator();
    ImGui::Spacing();
    
    // ===== 战斗辅助（带二级菜单）=====
    static bool combat_open = true;
    if (ImGui::CollapsingHeader(ICON_FA_SHIELD " 战斗辅助", ImGuiTreeNodeFlags_DefaultOpen)) {
        static bool god_mode = false;
        static bool aimbot = false;
        static bool esp = false;
        static bool no_recoil = false;
        
        // 第一行：秒杀按钮 + 无敌开关
        if (ImGui::Button(ICON_FA_SKULL " 秒杀", ImVec2(120, 40))) {
            // 秒杀代码
        }
        ImGui::SameLine();
        ImGui::Checkbox(ICON_FA_SHIELD " 无敌", &god_mode);
        
        // 第二行：自瞄 + 透视
        ImGui::Checkbox(ICON_FA_CROSSHAIRS " 自瞄", &aimbot);
        ImGui::SameLine(100);
        ImGui::Checkbox(ICON_FA_EYE " 透视", &esp);
        ImGui::SameLine(200);
        ImGui::Checkbox(ICON_FA_GUN " 无后座", &no_recoil);
        
        // 自瞄二级菜单
        if (aimbot) {
            ImGui::Indent(20);
            ImGui::Separator();
            ImGui::Text(ICON_FA_CROSSHAIRS " 自瞄参数");
            
            static float smooth = 1.2f;
            static int fov = 90;
            static bool visible_only = true;
            
            ImGui::SliderFloat("平滑度", &smooth, 0.5f, 3.0f, "%.1f");
            ImGui::SliderInt("范围", &fov, 30, 180, "%d°");
            ImGui::Checkbox("仅可见敌人", &visible_only);
            
            ImGui::Unindent(20);
            ImGui::Spacing();
        }
        
        // 透视二级菜单
        if (esp) {
            ImGui::Indent(20);
            ImGui::Separator();
            ImGui::Text(ICON_FA_EYE " 透视参数");
            
            static int range = 300;
            static bool box = true;
            static bool line = false;
            static bool hp = true;
            
            ImGui::SliderInt("透视范围", &range, 100, 500, "%d米");
            ImGui::Checkbox("方框", &box);
            ImGui::SameLine(80);
            ImGui::Checkbox("射线", &line);
            ImGui::Checkbox("血量", &hp);
            
            ImGui::Unindent(20);
            ImGui::Spacing();
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // ===== 辅助工具 =====
    if (ImGui::CollapsingHeader(ICON_FA_BOLT " 辅助工具", ImGuiTreeNodeFlags_DefaultOpen)) {
        static bool crosshair = false;
        static bool speed = false;
        static float speed_val = 1.5f;
        
        ImGui::Checkbox(ICON_FA_CROSSHAIRS " 屏幕准星", &crosshair);
        ImGui::SameLine(120);
        ImGui::Checkbox(ICON_FA_GAUGE " 加速", &speed);
        
        if (speed) {
            ImGui::Indent(20);
            ImGui::SliderFloat("速度倍率", &speed_val, 1.0f, 3.0f, "%.1fx");
            ImGui::Unindent(20);
        }
        
        if (crosshair) {
            screen_config();
            ImDrawList* draw = ImGui::GetBackgroundDrawList();
            ImVec2 center = ImVec2(displayInfo.width / 2, displayInfo.height / 2);
            float size = 20.0f;
            draw->AddLine(ImVec2(center.x - size, center.y), ImVec2(center.x + size, center.y),
                         IM_COL32(255, 0, 0, 200), 2.0f);
            draw->AddLine(ImVec2(center.x, center.y - size), ImVec2(center.x, center.y + size),
                         IM_COL32(255, 0, 0, 200), 2.0f);
        }
    }
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // ===== 系统状态 =====
    if (ImGui::CollapsingHeader(ICON_FA_CHART_LINE " 系统状态", ImGuiTreeNodeFlags_DefaultOpen)) {
        float fps = ImGui::GetIO().Framerate;
        ImGui::Text("FPS: %.1f", fps);
        ImGui::ProgressBar(fps / 120.0f, ImVec2(250, 0), "");
        
        ImGui::TextColored(ImVec4(0.45f, 0.75f, 0.45f, 1.00f), "● 外挂已激活");
        ImGui::SameLine(150);
        ImGui::Text("版本: 2.1.0");
        
        // 渲染接口
        ImGui::Text("渲染: %s", graphics->RenderName);
    }
    
    ImGui::End();
    
    // ===== 坤坤窗口（保留）=====
    static bool show_another_window = false;
    if (show_another_window) {
        ImGui::Begin("🐔 爱坤专区", &show_another_window, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::PushFont(icon_font_2);
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.8f, 1.00f), "%s 真爱坤", ICON_FA_HEART);
        ImGui::PopFont();
        ImGui::Separator();
        ImGui::Image(Aekun_image.DS, ImVec2(200, 200));
        if (ImGui::Button(ICON_FA_CLOSE " 关闭窗口", ImVec2(150, 36))) {
            show_another_window = false;
        }
        ImGui::End();
    }
    
    // 坤坤开关（放在菜单底部）
    ImGui::PushFont(icon_font_2);
    if (ImGui::Button(ICON_FA_HEART " 真爱坤", ImVec2(100, 36))) {
        show_another_window = true;
    }
    ImGui::PopFont();
    
    // ===== 保留演示窗口开关（默认关）=====
    static bool show_demo_window = false;
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }
    
    // ===== 保留射线绘制（默认关）=====
    static bool show_draw_Line = false;
    if (show_draw_Line) {
        ImGui::GetForegroundDrawList()->AddLine(
            ImVec2(0,0),
            ImVec2(displayInfo.width, displayInfo.height),
            IM_COL32(255,0,0,255), 4);
    }
}

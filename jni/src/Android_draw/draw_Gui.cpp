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
    ImGui::My_Android_LoadSystemFont(32.0f);  // 字体加大
    M_Android_LoadFont(32.0f);
    ImGui::GetStyle().ScaleAllSizes(1.0f);    // 取消全局缩放
}

void screen_config() {
    ::displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
}

void drawBegin() {
    if (::permeate_record_ini) {
        LastCoordinate.Pos_x = ::g_window->Pos.x;
        LastCoordinate.Pos_y = ::g_window->Pos.y;
        LastCoordinate.Size_x = ::g_window->Size.x;
        LastCoordinate.Size_y = ::g_window->Size.y;

        graphics->Shutdown();
        android::ANativeWindowCreator::Destroy(::window);
        ::window = android::ANativeWindowCreator::Create("AImGui", native_window_screen_x, native_window_screen_y, permeate_record);
        graphics->Init_Render(::window, native_window_screen_x, native_window_screen_y);
        ::init_My_drawdata();
    }

    static uint32_t orientation = -1;
    screen_config();
    if (orientation != displayInfo.orientation) {
        orientation = displayInfo.orientation;
        Touch::setOrientation((int)displayInfo.orientation);
        if (g_window != NULL) {
            g_window->Pos.x = 100;
            g_window->Pos.y = 125;        
        }
    }
}

void Layout_tick_UI(bool *main_thread_flag) {
    static bool show_draw_Line = false;
    static bool show_demo_window = false;
    
    // 调整样式：加大间距，解决勾选框遮挡
    ImGuiStyle& style = ImGui::GetStyle();
    style.FramePadding = ImVec2(8, 6);      // 按钮内边距
    style.ItemSpacing = ImVec2(10, 12);     // 项间距（垂直加大）
    style.ItemInnerSpacing = ImVec2(6, 6);  // 内间距
    style.IndentSpacing = 20;                // 缩进宽度
    
    // 主窗口（合并所有功能）
    ImGui::Begin("✨ 银河外挂", main_thread_flag, 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    
    // ===== 1. 系统信息栏（始终显示）=====
    ImGui::PushFont(icon_font_2);
    ImGui::TextColored(ImVec4(0.65f, 0.85f, 1.00f, 1.00f), "%s 控制中心", ICON_FA_BOLT);
    ImGui::PopFont();
    ImGui::SameLine(ImGui::GetWindowWidth() - 120);
    ImGui::Text("FPS: %.0f", ImGui::GetIO().Framerate);
    ImGui::Separator();
    ImGui::Spacing();
    
    // ===== 2. 基础设置（折叠）=====
    if (ImGui::CollapsingHeader("⚙ 基础设置", ImGuiTreeNodeFlags_DefaultOpen)) {
        static int style_idx = 0;
        ImGui::Combo("主题", &style_idx, "白色\0深色\0经典\0");
        if (ImGui::IsItemDeactivated()) {
            switch (style_idx) {
                case 0: ImGui::StyleColorsLight(); break;
                case 1: ImGui::StyleColorsDark(); break;
                case 2: ImGui::StyleColorsClassic(); break;
            }
        }
        
        ImGui::Checkbox("过录制", &::permeate_record);
        ImGui::SameLine(120);
        ImGui::Checkbox("演示窗口", &show_demo_window);
        ImGui::SameLine(240);
        ImGui::Checkbox("绘制射线", &show_draw_Line);
        
        ImGui::Text("渲染: %s", graphics->RenderName);
        ImGui::Text("GUI版本: %s", ImGui::GetVersion());
    }
    
    // ===== 3. 战斗辅助（折叠）=====
    if (ImGui::CollapsingHeader(ICON_FA_SHIELD " 战斗辅助", ImGuiTreeNodeFlags_DefaultOpen)) {
        static bool god_mode = false;
        static bool aimbot = false;
        static bool esp = false;
        
        // 第一行
        if (ImGui::Button(ICON_FA_SKULL " 秒杀", ImVec2(120, 40))) {
            // 秒杀代码
        }
        ImGui::SameLine();
        ImGui::Checkbox(ICON_FA_SHIELD " 无敌", &god_mode);
        
        // 第二行
        ImGui::Checkbox(ICON_FA_CROSSHAIRS " 自瞄", &aimbot);
        ImGui::SameLine(120);
        ImGui::Checkbox(ICON_FA_EYE " 透视", &esp);
        
        // 自瞄二级菜单
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
        
        // 透视二级菜单
        if (esp) {
            ImGui::Indent(20);
            ImGui::Separator();
            ImGui::Text(ICON_FA_EYE " 透视参数");
            
            static int range = 300;
            ImGui::SliderInt("透视范围", &range, 100, 500, "%d米");
            
            ImGui::Unindent(20);
        }
    }
    
    // ===== 4. 调试工具（折叠）=====
    if (ImGui::CollapsingHeader("🔧 调试工具")) {
        static float f = 0.5f;
        static int counter = 0;
        static ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
        
        ImGui::SliderFloat("测试滑块", &f, 0.0f, 1.0f);
        ImGui::ColorEdit4("取色器", (float*)&clear_color);
        
        if (ImGui::Button("计数器")) {
            counter++;
        }
        ImGui::SameLine();
        ImGui::Text("次数: %d", counter);
        
        ImGui::Text("窗口焦点: %d", ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow));
    }
    
    ImGui::End();
    
    // ===== 演示窗口 =====
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
    }
    
    // ===== 射线绘制 =====
    if (show_draw_Line) {
        ImGui::GetForegroundDrawList()->AddLine(
            ImVec2(0,0),
            ImVec2(displayInfo.width, displayInfo.height),
            IM_COL32(255,0,0,255), 4);
    }
}

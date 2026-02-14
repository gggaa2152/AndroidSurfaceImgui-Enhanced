#include "draw.h"
#include "My_font/zh_Font.h"
#include "My_font/fontawesome-solid.h"

// FontAwesome 图标宏定义
#ifndef ICON_MIN_FA
#define ICON_MIN_FA 0xf000
#define ICON_MAX_FA 0xf8ff
#endif

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
    // 窗口位置记忆
    if (::permeate_record_ini) {
        ImGui::SetWindowPos({LastCoordinate.Pos_x, LastCoordinate.Pos_y});
        ImGui::SetWindowSize({LastCoordinate.Size_x, LastCoordinate.Size_y});
        permeate_record_ini = false;   
    }
    
    // ===== 唯一的窗口：自定义滑动开关 =====
    ImGui::Begin("自定义开关", main_thread_flag, 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    
    // 开关状态
    static bool bSwitch = false;
    
    // 显示文字
    ImGui::Text("触摸自瞄");
    ImGui::SameLine(120);
    
    // 获取当前绘制位置
    ImVec2 p = ImGui::GetCursorScreenPos();
    float height = 24.0f;
    float width = 48.0f;
    float radius = height * 0.5f;
    
    // 画背景（圆角矩形）- 开启时绿色，关闭时灰色
    ImU32 bgColor = bSwitch ? IM_COL32(100, 200, 100, 255) : IM_COL32(80, 80, 80, 255);
    ImGui::GetWindowDrawList()->AddRectFilled(
        p, ImVec2(p.x + width, p.y + height), bgColor, radius
    );
    
    // 画滑块（白色圆）- 开启时在右边，关闭时在左边
    float thumbPos = bSwitch ? p.x + width - height : p.x;
    ImGui::GetWindowDrawList()->AddCircleFilled(
        ImVec2(thumbPos + radius, p.y + radius), radius - 3, IM_COL32_WHITE
    );
    
    // 不可见按钮处理点击事件
    ImGui::InvisibleButton("##toggle", ImVec2(width, height));
    if (ImGui::IsItemClicked()) {
        bSwitch = !bSwitch;
        // 点击时的日志（可选）
        __android_log_print(ANDROID_LOG_INFO, "PureElf", "开关状态: %s", bSwitch ? "开启" : "关闭");
    }
    
    // 可选：显示当前状态文字
    ImGui::SameLine(180);
    ImGui::Text(bSwitch ? "开启" : "关闭");
    
    ImGui::End();
    
    // 记录窗口位置
    g_window = ImGui::GetCurrentWindow();
}

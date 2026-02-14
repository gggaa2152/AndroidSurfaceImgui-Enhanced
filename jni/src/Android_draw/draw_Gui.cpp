#include "draw.h"
#include "My_font/zh_Font.h"
#include "My_font/fontawesome-solid.h"
#include <map> // 动画状态需要用到这个

#ifndef ICON_MIN_FA
#define ICON_MIN_FA 0xf000
#define ICON_MAX_FA 0xf8ff
#endif

// ===== 在 main.cpp 中定义的变量 (外部声明) =====
// 这些变量必须在 main.cpp 或其他地方被定义 (extern 关键字表示它们在别处)
extern bool permeate_record;
// extern std::unique_ptr<AndroidImgui> graphics; // 不直接在这里使用，所以暂时注释
extern ANativeWindow *window; // 不直接在这里使用，所以暂时注释
extern android::ANativeWindowCreator::DisplayInfo displayInfo; // 不直接在这里使用，所以暂时注释
extern int abs_ScreenX, abs_ScreenY; // 不直接在这里使用，所以暂时注释
extern int native_window_screen_x, native_window_screen_y; // 不直接在这里使用，所以暂时注释

// ===== 在 draw_Gui.cpp 中定义的变量 (本文件定义) =====
bool permeate_record_ini = false; // 用于控制窗口初始位置/大小是否从记录加载
struct Last_ImRect LastCoordinate = {0, 0, 0, 0}; // 记录上次窗口位置和大小
ImGuiWindow *g_window = NULL; // 用于获取当前 ImGui 窗口指针

// ===== 缩放相关变量 =====
static float g_global_scale = 1.0f; // 全局 UI 缩放比例
static bool g_show_settings = false; // 是否显示设置面板

ImFont* zh_font = NULL; // 中文字体指针
ImFont* icon_font_2 = NULL; // 图标字体指针

// ====================================================================
// ImGui 字体加载函数 (用于加载中文和图标字体)
// ====================================================================
bool M_Android_LoadFont(float SizePixels) {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear(); // 清除现有字体

    // 加载中文字体
    // ImGui::My_Android_LoadSystemFont 是一个自定义函数，假设它存在于你的 My_imgui_impl_android.cpp
    // 或其他地方，用于加载 Android 系统字体。
    // 如果没有，你需要替换成 ImGui::GetIO().Fonts->AddFontDefault() 或者 AddFontFromMemoryCompressedTTF
    ImGui::My_Android_LoadSystemFont(SizePixels * g_global_scale);
    zh_font = io.Fonts->Fonts[0]; // 将加载的第一个字体设为中文字体

    // 加载 FontAwesome 图标字体 (合并到中文字体)
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true; // 合并模式，图标会叠加到中文字体上
    icons_config.SizePixels = SizePixels * g_global_scale;
    icons_config.GlyphOffset.y = 4.0f * g_global_scale; // 调整图标垂直偏移
    
    icon_font_2 = io.Fonts->AddFontFromMemoryCompressedTTF(
        (const void*)&font_awesome_solid_compressed_data,
        sizeof(font_awesome_solid_compressed_data),
        0.0f, &icons_config, icons_ranges);
    
    io.Fonts->Build(); // 构建字体图集
    io.FontDefault = zh_font; // 设置默认字体为中文字体
    return true;
}

// ====================================================================
// ImGui 初始化绘制数据 (样式、字体等)
// ====================================================================
void init_My_drawdata() {
    ImGui::StyleColorsDark(); // 设置为深色主题
    M_Android_LoadFont(32.0f); // 加载字体，基础字号 32 像素

    ImGuiStyle& style = ImGui::GetStyle();
    style.FramePadding = ImVec2(10 * g_global_scale, 8 * g_global_scale);
    style.ItemSpacing = ImVec2(12 * g_global_scale, 14 * g_global_scale);
    style.WindowPadding = ImVec2(16 * g_global_scale, 16 * g_global_scale);
    style.FrameRounding = 6.0f * g_global_scale; // 窗口圆角
    style.GrabRounding = 6.0f * g_global_scale; // 滑动条圆角
    style.ScrollbarRounding = 6.0f * g_global_scale; // 滚动条圆角
    style.WindowBorderSize = 0.0f; // 窗口无边框
    style.FrameBorderSize = 0.0f; // 框架无边框
    // style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.85f); // 窗口背景，可调整透明度
}

// ====================================================================
// 自定义开关绘制函数 (带平滑动画)
// ====================================================================
void DrawToggle(const char* label, bool* v) {
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    // 基础尺寸定义，支持你的全局缩放
    float height = 24.0f * g_global_scale; 
    float width = 46.0f * g_global_scale;
    float radius = height * 0.5f;

    // --- 布局 ---
    ImGui::Text("%s", label); // 显示功能名称
    // 自动靠右对齐开关
    ImGui::SameLine(ImGui::GetWindowWidth() - width - (16.0f * g_global_scale)); 

    ImVec2 p = ImGui::GetCursorScreenPos(); // 获取当前绘制位置

    // --- 交互逻辑（隐形按钮来处理点击） ---
    ImGui::InvisibleButton(label, ImVec2(width, height));
    if (ImGui::IsItemClicked()) {
        *v = !*v; // 切换布尔值
    }

    // --- 动画处理 (Lerp 线性插值) ---
    // 使用静态 Map 存储每个功能的动画进度 (0.0f -> 1.0f)
    // 这样每个开关都有独立的动画状态
    static std::map<ImGuiID, float> anim_stack;
    ImGuiID id = ImGui::GetID(label); // 获取当前控件的唯一ID
    // 如果是第一次绘制，初始化动画进度
    if (anim_stack.find(id) == anim_stack.end()) {
        anim_stack[id] = (*v ? 1.0f : 0.0f);
    }
    
    float& anim = anim_stack[id]; // 获取当前开关的动画进度引用
    float target = *v ? 1.0f : 0.0f; // 目标动画进度 (开启为1，关闭为0)
    
    // 使用 DeltaTime 让动画帧率无关，数值 15.0f 越大滑动越快
    anim += (target - anim) * io.DeltaTime * 15.0f; 

    // --- 颜色处理 ---
    // 背景颜色：从深灰 (关闭) 渐变到 绿色 (开启)
    ImVec4 col_off = ImVec4(0.3f, 0.3f, 0.3f, 1.0f); // 关闭时的背景色 (深灰)
    ImVec4 col_on  = ImVec4(0.4f, 0.8f, 0.4f, 1.0f); // 开启时的背景色 (绿色)
    ImU32 col_bg = ImGui::GetColorU32(ImVec4( // 根据动画进度计算当前背景色
        col_off.x + (col_on.x - col_off.x) * anim,
        col_off.y + (col_on.y - col_off.y) * anim,
        col_off.z + (col_on.z - col_off.z) * anim,
        1.0f
    ));
    
    // --- 绘制渲染 ---
    // 1. 绘制背景胶囊矩形
    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, radius);
    
    // 2. 绘制滑动实心圆 (圆点颜色固定为白色)
    float dot_x = p.x + radius + (anim * (width - radius * 2.0f)); // 根据动画进度计算圆心X坐标
    draw_list->AddCircleFilled(ImVec2(dot_x, p.y + radius), radius - (3.0f * g_global_scale), IM_COL32_WHITE);
}

// ====================================================================
// 你的主 UI 绘制函数 (Layout_tick_UI) - 所有 UI 逻辑的入口
// ====================================================================
void Layout_tick_UI(bool *main_thread_flag) {
    // 🚩 解决你之前遇到的 "Debug" 窗口问题：强制关闭底层可能残留的窗口
    // 某些 ImGui 封装在 NewFrame 后会默认打开一个窗口，这里确保它被关闭。
    ImGui::End(); 

    // 窗口位置/大小初始化 (permeate_record_ini 为 true 时，从 LastCoordinate 读取)
    // 这个逻辑保留，用于记住窗口位置
    if (permeate_record_ini) {
        ImGui::SetWindowPos({LastCoordinate.Pos_x, LastCoordinate.Pos_y});
        ImGui::SetWindowSize({LastCoordinate.Size_x, LastCoordinate.Size_y});
        permeate_record_ini = false;    
    }

    // ===== 你的主控制窗口 =====
    // ImGuiWindowFlags_NoCollapse: 不可折叠
    // ImGuiWindowFlags_AlwaysAutoResize: 窗口大小自动根据内容调整
    ImGui::Begin("控制中心", main_thread_flag, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
    
    // 假设你的功能开关变量
    static bool bAimbot = false; // "触摸自瞄" 开关状态
    static bool bEsp = false;    // "显示方框" 开关状态 (示例)
    
    // --- 调用自定义开关函数 ---
    // 每个开关只需要一行代码
    DrawToggle("触摸自瞄", &bAimbot);
    ImGui::Spacing(); // 增加一点间距
    DrawToggle("显示方框", &bEsp);

    // --- 逻辑执行区 ---
    // 根据开关状态，执行相应的功能代码
    if (bAimbot) {
        // 当 "触摸自瞄" 开启时，这里每帧都会执行
        // 放入你实际的自瞄功能函数调用，例如：
        // com_tencent_jkchess_RunAimbotLogic();
    }
    if (bEsp) {
        // 当 "显示方框" 开启时，这里每帧都会执行
        // 放入你实际的 ESP 绘制功能函数调用，例如：
        // com_tencent_jkchess_DrawESP();
    }

    // --- FPS 显示与底部提示 ---
    ImGui::Spacing(); // 增加间距
    ImGui::Separator(); // 分隔线
    ImGui::Spacing(); // 增加间距
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "FPS: %.0f", ImGui::GetIO().Framerate); // 显示 FPS
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "右下角可拖动缩放"); // 提示文本

    ImGui::End(); // 结束当前 ImGui 窗口

    // 记录窗口位置 (用于下次启动时恢复)
    g_window = ImGui::GetCurrentWindow();
}

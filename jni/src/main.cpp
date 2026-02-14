#include "draw.h"
#include "AndroidImgui.h"
#include "GraphicsManager.h"

// [!!!] 必须在这里定义实体，否则 GitHub 编译会报 undefined symbol
std::unique_ptr<AndroidImgui> graphics = nullptr; 
android::ANativeWindowCreator::DisplayInfo displayInfo;
ANativeWindow *window = nullptr;

int native_window_screen_x = 0;
int native_window_screen_y = 0;
int abs_ScreenX = 0;
int abs_ScreenY = 0;
bool permeate_record = false;

// 声明外部函数
void Layout_tick_UI(bool *main_thread_flag);
void init_My_drawdata();

// 实现缺失的函数实体
void screen_config() {
    ::displayInfo = android::ANativeWindowCreator::GetDisplayInfo();
}

void drawBegin() {
    // 处理屏幕旋转等逻辑
}

int main(int argc, char *argv[]) {
    // 1. 初始化渲染接口 (Vulkan)
    ::graphics = GraphicsManager::getGraphicsInterface(GraphicsManager::VULKAN);

    // 2. 获取并设置屏幕尺寸
    ::screen_config(); 
    int max_val = (::displayInfo.height > ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    int min_val = (::displayInfo.height < ::displayInfo.width ? ::displayInfo.height : ::displayInfo.width);
    
    ::native_window_screen_x = max_val;
    ::native_window_screen_y = max_val; 
    ::abs_ScreenX = max_val;
    ::abs_ScreenY = min_val;

    // 3. 创建窗口并初始化
    ::window = android::ANativeWindowCreator::Create("AImGui", native_window_screen_x, native_window_screen_y, permeate_record);
    graphics->Init_Render(::window, native_window_screen_x, native_window_screen_y);
    
    // 4. 初始化触摸与字体
    Touch::Init({(float)::abs_ScreenX, (float)::abs_ScreenY}, false); 
    Touch::setOrientation(displayInfo.orientation);
    ::init_My_drawdata(); 

    static bool flag = true;
    while (flag) {
        drawBegin();
        if (!permeate_record) {
            android::ANativeWindowCreator::ProcessMirrorDisplay();
        }
        
        graphics->NewFrame();
        
        // 执行高端 UI 逻辑
        Layout_tick_UI(&flag);

        graphics->EndFrame();        
    }
    
    graphics->Shutdown();
    android::ANativeWindowCreator::Destroy(::window);
    return 0;
}

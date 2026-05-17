#pragma once

// EntryPoint —— main() 函数定义
//
// 每个可执行文件最多只能有一个 .cpp include <AF.h>（因为 AF.h 包含此文件）。
// 其他 .cpp 各自按需 include 所需头文件。

#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Types.h"

extern AF::Application* CreateApplication();

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    // 最优先初始化日志
    AF::Log::Init();

    auto app = AF::Unique<AF::Application>(CreateApplication());
    app->Run();

    return 0;
}

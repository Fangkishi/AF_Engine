#pragma once

// Application —— 用户应用的薄壳
//
// 用户继承 Application，在 OnSetup 中注册 System，然后 Run 启动主循环。
// EntryPoint.h 提供 main()，保证每个可执行文件只有一个 include 路径。

#include "Core/Types.h"
#include "Core/Engine.h"

#include <string>

namespace AF {

class Application : public NonCopyable
{
public:
    explicit Application(std::string_view name);
    virtual ~Application();

    /// 用户在此注册 System
    virtual void OnSetup(Engine& engine) {}

    /// 启动引擎主循环
    void Run();

    Engine& GetEngine() { return m_Engine; }

private:
    Engine m_Engine;
};

} // namespace AF

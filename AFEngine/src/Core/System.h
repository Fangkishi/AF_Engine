#pragma once

// System 基类 —— 引擎功能单元的虚接口
//
// 所有功能模块（渲染、UI、物理等）继承此类，注册到 Engine。
// 生命周期：OnInitialize → 每帧 OnUpdate → OnEvent → OnShutdown。

namespace AF {

class Event;
class Engine;

class System
{
public:
    virtual ~System() = default;

    /// 系统初始化（Engine::AddSystem 时自动调用）
    virtual void OnInitialize(Engine&) {}

    /// 每帧更新（dt = 帧间秒数）
    virtual void OnUpdate(float dt) {}

    /// 事件分发（按注册顺序遍历，Handled 后终止）
    virtual void OnEvent(Event&) {}

    /// 系统析构前清理
    virtual void OnShutdown() {}

protected:
    Engine* GetEngine() { return m_Engine; }
    const Engine* GetEngine() const { return m_Engine; }

private:
    Engine* m_Engine = nullptr;
    friend class Engine;  // 仅 Engine 可设 m_Engine
};

} // namespace AF

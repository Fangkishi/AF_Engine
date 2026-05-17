#pragma once

// 事件系统 —— 引擎内部事件类型基类与分发器
//
// Event 是基类，各具体事件（键盘/鼠标/窗口）继承它。
// EventDispatcher 根据事件类型静态分发到对应的处理函数。
// 宏 EVENT_CLASS_TYPE / EVENT_CLASS_CATEGORY 为派生类简化样板代码。

#include <functional>
#include <string>
#include <sstream>

namespace AF {

/// 事件类型枚举
enum class EventType : uint8_t
{
    None = 0,
    WindowClose, WindowResize,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

/// 事件分类位掩码，用于快速筛选
enum EventCategory : uint8_t
{
    EventCategoryNone        = 0,
    EventCategoryApplication = 1 << 0,
    EventCategoryInput       = 1 << 1,
    EventCategoryKeyboard    = 1 << 2,
    EventCategoryMouse       = 1 << 3,
    EventCategoryMouseButton = 1 << 4,
};

#define EVENT_CLASS_TYPE(type) \
    static EventType GetStaticType() { return EventType::type; } \
    EventType GetEventType() const override { return GetStaticType(); } \
    const char* GetName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) \
    int GetCategoryFlags() const override { return category; }

class Event
{
public:
    virtual ~Event() = default;

    bool Handled = false;

    virtual EventType GetEventType() const = 0;
    virtual const char* GetName() const = 0;
    virtual int GetCategoryFlags() const = 0;
    virtual std::string ToString() const { return GetName(); }

    bool IsInCategory(EventCategory category) const
    {
        return GetCategoryFlags() & category;
    }
};

/// 事件分发器 —— 通过 EventType 匹配自动转型并调用处理函数
class EventDispatcher
{
public:
    explicit EventDispatcher(Event& event)
        : m_Event(event)
    {
    }

    /// 若事件类型匹配 T，则调用 func(T& event)，返回 true
    template <typename T, typename F>
    bool Dispatch(const F& func)
    {
        if (m_Event.GetEventType() == T::GetStaticType())
        {
            m_Event.Handled |= func(static_cast<T&>(m_Event));
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};

} // namespace AF

#pragma once

// 键盘事件 —— KeyPressed / KeyReleased / KeyTyped

#include "Events/Event.h"
#include "Events/KeyCodes.h"

namespace AF {

class KeyEvent : public Event
{
public:
    KeyCode GetKeyCode() const { return m_KeyCode; }

    EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)

protected:
    explicit KeyEvent(KeyCode keycode)
        : m_KeyCode(keycode)
    {
    }

    KeyCode m_KeyCode;
};

/// 按键按下事件（含 repeat 标志以区分首次按下与长按重复）
class KeyPressedEvent : public KeyEvent
{
public:
    KeyPressedEvent(KeyCode keycode, bool isRepeat = false)
        : KeyEvent(keycode), m_IsRepeat(isRepeat)
    {
    }

    bool IsRepeat() const { return m_IsRepeat; }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "KeyPressedEvent: " << m_KeyCode << " (repeat=" << m_IsRepeat << ")";
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyPressed)

private:
    bool m_IsRepeat;
};

/// 按键释放事件
class KeyReleasedEvent : public KeyEvent
{
public:
    explicit KeyReleasedEvent(KeyCode keycode)
        : KeyEvent(keycode)
    {
    }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "KeyReleasedEvent: " << m_KeyCode;
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyReleased)
};

/// 字符输入事件（已处理 Shift 等修饰符，适用于文本输入）
class KeyTypedEvent : public KeyEvent
{
public:
    explicit KeyTypedEvent(KeyCode keycode)
        : KeyEvent(keycode)
    {
    }

    std::string ToString() const override
    {
        std::stringstream ss;
        ss << "KeyTypedEvent: " << m_KeyCode;
        return ss.str();
    }

    EVENT_CLASS_TYPE(KeyTyped)
};

} // namespace AF

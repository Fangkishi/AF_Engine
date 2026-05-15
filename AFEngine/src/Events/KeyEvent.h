#pragma once

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

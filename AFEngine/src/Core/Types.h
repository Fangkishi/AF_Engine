#pragma once

// 类型别名和基类工具 —— 贯穿引擎各层的通用类型定义

#include "Core/Platform.h"

#include <memory>

namespace AF {

/// 独占所有权指针别名（语义同 unique_ptr）
template <typename T>
using Scope = std::unique_ptr<T>;

/// 共享所有权指针别名（语义同 shared_ptr）
template <typename T>
using Ref = std::shared_ptr<T>;

/// 独占所有权指针别名（与 Scope 等价，名称对称）
template <typename T>
using Unique = std::unique_ptr<T>;

/// 不可拷贝基类 —— 派生类禁止拷贝构造/赋值
class NonCopyable
{
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

/// 不可移动基类 —— 派生类禁止移动构造/赋值
class NonMovable
{
public:
    NonMovable() = default;
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
};

} // namespace AF

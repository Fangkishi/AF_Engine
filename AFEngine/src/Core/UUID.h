#pragma once

// UUID —— 64 位随机唯一标识符
//
// 默认构造生成随机值，也可用显式 uint64_t 构造（如反序列化时恢复 ID）。
// 已提供 std::hash 特化以支持 unordered_map 等容器。

#include <cstdint>
#include <functional>

namespace AF {

class UUID
{
public:
    UUID();
    explicit UUID(uint64_t id);

    /// 隐式转换为 uint64_t 方便比较和存储
    operator uint64_t() const { return m_ID; }

    bool operator==(const UUID& other) const { return m_ID == other.m_ID; }
    bool operator!=(const UUID& other) const { return m_ID != other.m_ID; }

private:
    uint64_t m_ID;
};

} // namespace AF

namespace std {

template <>
struct hash<AF::UUID>
{
    std::size_t operator()(const AF::UUID& uuid) const
    {
        return static_cast<uint64_t>(uuid);
    }
};

} // namespace std

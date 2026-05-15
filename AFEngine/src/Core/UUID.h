#pragma once

#include <cstdint>
#include <functional>

namespace AF {

class UUID
{
public:
    UUID();
    explicit UUID(uint64_t id);

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

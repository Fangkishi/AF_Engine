#include "Core/UUID.h"

#include <random>

namespace AF {

static std::random_device s_RandomDevice;
static std::mt19937_64 s_Engine(s_RandomDevice());
static std::uniform_int_distribution<uint64_t> s_Distribution;

UUID::UUID()
    : m_ID(s_Distribution(s_Engine))  // 随机 64 位
{
}

UUID::UUID(uint64_t id)
    : m_ID(id)
{
}

} // namespace AF

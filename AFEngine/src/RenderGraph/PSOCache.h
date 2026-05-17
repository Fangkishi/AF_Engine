#pragma once

// PSOCache —— 图形管线状态对象缓存
//
// 根据 PSODesc（着色器、深度/模板、光栅化、混合状态）生成哈希，
// 避免相同 PSO 在每一帧重复设置。GetOrCreate 返回哈希值，
// Apply 将缓存的状态应用到 RHICommandBuffer。

#include "RHI/PipelineState.h"
#include "RHI/RHIShader.h"
#include "Core/Types.h"
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace AF {

namespace RHI { class RHICommandBuffer; }

struct PSODesc
{
    Ref<RHI::RHIShader> VertexShader;
    Ref<RHI::RHIShader> FragmentShader;

    RHI::DepthStencilState DepthStencil;
    RHI::RasterizerState   Rasterizer;
    std::vector<RHI::BlendState> BlendStates;
    RHI::BufferLayout InputLayout;

    std::vector<RHI::TextureFormat> ColorFormats;
    RHI::TextureFormat DepthFormat = RHI::TextureFormat::Depth32;

    size_t Hash() const;
    bool operator==(const PSODesc& other) const;
};

class PSOCache
{
public:
    static PSOCache& Get();
    size_t GetOrCreate(const PSODesc& desc);
    void Apply(size_t hash, RHI::RHICommandBuffer& cmd);

private:
    std::unordered_map<size_t, PSODesc> m_Cache;
    size_t m_LastAppliedHash = 0;
};

} // namespace AF

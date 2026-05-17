#include "RenderGraph/PSOCache.h"
#include "RHI/RHICommandBuffer.h"

namespace AF {

// ── PSODesc 哈希与比较 ──

size_t PSODesc::Hash() const
{
    size_t h = 0;
    auto combine = [&h](size_t val) { h ^= val + 0x9e3779b9 + (h << 6) + (h >> 2); };

    combine(reinterpret_cast<size_t>(VertexShader.get()));
    combine(reinterpret_cast<size_t>(FragmentShader.get()));
    combine(DepthStencil.DepthTest ? 1 : 0);
    combine(DepthStencil.DepthWrite ? 1 : 0);
    combine(static_cast<size_t>(DepthStencil.DepthFunc));
    combine(DepthStencil.StencilTest ? 1 : 0);
    combine(static_cast<size_t>(Rasterizer.Cull));
    combine(static_cast<size_t>(Rasterizer.Winding));
    combine(static_cast<size_t>(Rasterizer.Fill));
    combine(BlendStates.size());
    for (auto& b : BlendStates)
    {
        combine(b.Enable ? 1 : 0);
        combine(static_cast<size_t>(b.SrcColor));
        combine(static_cast<size_t>(b.DstColor));
        combine(static_cast<size_t>(b.ColorOp));
        combine(static_cast<size_t>(b.SrcAlpha));
        combine(static_cast<size_t>(b.DstAlpha));
        combine(static_cast<size_t>(b.AlphaOp));
    }
    combine(static_cast<size_t>(DepthFormat));
    combine(ColorFormats.size());
    for (auto& f : ColorFormats)
        combine(static_cast<size_t>(f));
    return h;
}

bool PSODesc::operator==(const PSODesc& other) const
{
    if (VertexShader != other.VertexShader) return false;
    if (FragmentShader != other.FragmentShader) return false;
    if (DepthStencil.DepthTest != other.DepthStencil.DepthTest) return false;
    if (DepthStencil.DepthWrite != other.DepthStencil.DepthWrite) return false;
    if (DepthStencil.DepthFunc != other.DepthStencil.DepthFunc) return false;
    if (DepthStencil.StencilTest != other.DepthStencil.StencilTest) return false;
    if (Rasterizer.Cull != other.Rasterizer.Cull) return false;
    if (Rasterizer.Winding != other.Rasterizer.Winding) return false;
    if (Rasterizer.Fill != other.Rasterizer.Fill) return false;
    if (BlendStates.size() != other.BlendStates.size()) return false;
    for (size_t i = 0; i < BlendStates.size(); ++i)
    {
        auto& a = BlendStates[i];
        auto& b = other.BlendStates[i];
        if (a.Enable != b.Enable) return false;
        if (a.SrcColor != b.SrcColor) return false;
        if (a.DstColor != b.DstColor) return false;
        if (a.ColorOp != b.ColorOp) return false;
        if (a.SrcAlpha != b.SrcAlpha) return false;
        if (a.DstAlpha != b.DstAlpha) return false;
        if (a.AlphaOp != b.AlphaOp) return false;
    }
    if (DepthFormat != other.DepthFormat) return false;
    if (ColorFormats != other.ColorFormats) return false;
    return true;
}

// ── PSO 缓存单例 ──

PSOCache& PSOCache::Get()
{
    static PSOCache instance;
    return instance;
}

size_t PSOCache::GetOrCreate(const PSODesc& desc)
{
    size_t h = desc.Hash();
    if (m_Cache.find(h) == m_Cache.end())
        m_Cache[h] = desc;
    return h;
}

void PSOCache::Apply(size_t hash, RHI::RHICommandBuffer& cmd)
{
    if (hash == m_LastAppliedHash) return;

    auto it = m_Cache.find(hash);
    if (it == m_Cache.end()) return;

    const auto& desc = it->second;
    cmd.SetDepthStencilState(desc.DepthStencil.DepthTest, desc.DepthStencil.DepthWrite, desc.DepthStencil.DepthFunc);
    cmd.SetRasterizerState(desc.Rasterizer.Cull, desc.Rasterizer.Winding, desc.Rasterizer.Fill);

    if (!desc.BlendStates.empty())
    {
        const auto& b = desc.BlendStates[0];
        cmd.SetBlendState(0, b.Enable);
    }

    if (desc.FragmentShader)
        cmd.BindShader(desc.FragmentShader);

    m_LastAppliedHash = hash;
}

} // namespace AF

#pragma once

#include "RHI/RHITypes.h"
#include "RHI/RHIShader.h"

#include <cstdint>
#include <vector>

namespace AF {

struct RenderPassDesc
{
    // ── 渲染目标 ──
    uint32_t Width  = 1920;
    uint32_t Height = 1080;
    std::vector<RHI::TextureFormat> Attachments;
    bool Clear = true;

    // ── 着色器 ──
    Ref<RHI::RHIShader> Shader;

    // ── 深度/模板状态 ──
    struct DepthStencilState
    {
        bool               DepthTest   = false;
        bool               DepthWrite  = true;
        RHI::DepthCompareFunc DepthFunc = RHI::DepthCompareFunc::Less;
        bool               StencilTest = false;
        uint8_t            StencilReadMask  = 0xFF;
        uint8_t            StencilWriteMask = 0xFF;

        struct StencilFaceState
        {
            RHI::StencilOp FailOp      = RHI::StencilOp::Keep;
            RHI::StencilOp DepthFailOp = RHI::StencilOp::Keep;
            RHI::StencilOp PassOp      = RHI::StencilOp::Keep;
            RHI::DepthCompareFunc CompareFunc = RHI::DepthCompareFunc::Always;
            int              Reference   = 0;
        };

        StencilFaceState FrontFace;
        StencilFaceState BackFace;
    }
    DepthStencil;

    // ── 光栅化状态 ──
    struct RasterizerState
    {
        RHI::CullMode   Cull     = RHI::CullMode::Back;
        RHI::FrontFace  Winding  = RHI::FrontFace::CCW;
        RHI::FillMode   Fill     = RHI::FillMode::Solid;
        bool            DepthBias     = false;
        float           DepthBiasConstant = 0.0f;
        float           DepthBiasSlope    = 0.0f;
    }
    Rasterizer;

    // ── 混合状态（每个颜色附件一个）──
    struct BlendState
    {
        bool              Enable     = false;
        RHI::BlendFactor  SrcColor   = RHI::BlendFactor::One;
        RHI::BlendFactor  DstColor   = RHI::BlendFactor::Zero;
        RHI::BlendOp      ColorOp    = RHI::BlendOp::Add;
        RHI::BlendFactor  SrcAlpha   = RHI::BlendFactor::One;
        RHI::BlendFactor  DstAlpha   = RHI::BlendFactor::Zero;
        RHI::BlendOp      AlphaOp    = RHI::BlendOp::Add;
        RHI::ColorWriteMask WriteMask = RHI::ColorWriteMask::All;
    };
    std::vector<BlendState> BlendStates;

    // ── 顶点输入布局 ──
    RHI::BufferLayout InputLayout;
};

inline bool IsDepthFormat(RHI::TextureFormat fmt)
{
    return fmt == RHI::TextureFormat::Depth32 || fmt == RHI::TextureFormat::Depth24Stencil8;
}

} // namespace AF

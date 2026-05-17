#pragma once

// PipelineState —— 图形管线状态（PSO 描述）
//
// 集中定义 DepthStencil / Rasterizer / Blend 状态结构体，
// 供 RHICommandBuffer 录制 PSO 切换命令使用。

#include "RHI/RHITypes.h"
#include <cstdint>
#include <vector>

namespace AF {
namespace RHI {

// ── PSO Enums ──
enum class DepthCompareFunc : uint8_t { Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always };
enum class CullMode : uint8_t          { None, Front, Back, FrontAndBack };
enum class FrontFace : uint8_t         { CW, CCW };
enum class FillMode : uint8_t          { Solid, Wireframe, Point };
enum class BlendFactor : uint8_t       { Zero, One, SrcColor, OneMinusSrcColor, DstColor, OneMinusDstColor, SrcAlpha, OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha, ConstantColor, OneMinusConstantColor, ConstantAlpha, OneMinusConstantAlpha };
enum class BlendOp : uint8_t           { Add, Subtract, ReverseSubtract, Min, Max };
enum class ColorWriteMask : uint8_t    { None = 0, Red = 1, Green = 2, Blue = 4, Alpha = 8, All = 15 };
enum class StencilOp : uint8_t         { Keep, Zero, Replace, IncrClamp, DecrClamp, Invert, IncrWrap, DecrWrap };

// ── PSO State Structs ──
struct StencilFaceState
{
    StencilOp FailOp      = StencilOp::Keep;
    StencilOp DepthFailOp = StencilOp::Keep;
    StencilOp PassOp      = StencilOp::Keep;
    DepthCompareFunc CompareFunc = DepthCompareFunc::Always;
    int              Reference   = 0;
};

struct DepthStencilState
{
    bool               DepthTest   = false;
    bool               DepthWrite  = true;
    DepthCompareFunc   DepthFunc   = DepthCompareFunc::Less;
    bool               StencilTest = false;
    uint8_t            StencilReadMask  = 0xFF;
    uint8_t            StencilWriteMask = 0xFF;
    StencilFaceState   FrontFace;
    StencilFaceState   BackFace;
};

struct RasterizerState
{
    CullMode   Cull     = CullMode::Back;
    FrontFace  Winding  = FrontFace::CCW;
    FillMode   Fill     = FillMode::Solid;
    bool       DepthBias     = false;
    float      DepthBiasConstant = 0.0f;
    float      DepthBiasSlope    = 0.0f;
};

struct BlendState
{
    bool             Enable     = false;
    BlendFactor      SrcColor   = BlendFactor::One;
    BlendFactor      DstColor   = BlendFactor::Zero;
    BlendOp          ColorOp    = BlendOp::Add;
    BlendFactor      SrcAlpha   = BlendFactor::One;
    BlendFactor      DstAlpha   = BlendFactor::Zero;
    BlendOp          AlphaOp    = BlendOp::Add;
    ColorWriteMask   WriteMask  = ColorWriteMask::All;
};

} // namespace RHI
} // namespace AF

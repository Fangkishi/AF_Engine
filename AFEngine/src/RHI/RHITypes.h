#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace AF {
namespace RHI {

enum class TextureFormat : uint8_t
{
    Unknown = 0,
    RGBA8, RGB8, RGBA16F, R8,
    Depth32, Depth24Stencil8,
};

enum class ShaderDataType : uint8_t
{
    None = 0,
    Float, Float2, Float3, Float4,
    Mat3, Mat4,
    Int, Int2, Int3, Int4,
    Bool,
};

inline uint32_t ShaderDataTypeSize(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::Float:  return 4;
        case ShaderDataType::Float2: return 4 * 2;
        case ShaderDataType::Float3: return 4 * 3;
        case ShaderDataType::Float4: return 4 * 4;
        case ShaderDataType::Mat3:   return 4 * 3 * 3;
        case ShaderDataType::Mat4:   return 4 * 4 * 4;
        case ShaderDataType::Int:    return 4;
        case ShaderDataType::Int2:   return 4 * 2;
        case ShaderDataType::Int3:   return 4 * 3;
        case ShaderDataType::Int4:   return 4 * 4;
        case ShaderDataType::Bool:   return 1;
        case ShaderDataType::None:   break;
    }
    return 0;
}

struct BufferElement
{
    std::string Name;
    ShaderDataType Type = ShaderDataType::None;
    uint32_t Size = 0;
    uint32_t Offset = 0;
    bool Normalized = false;

    BufferElement() = default;
    BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
        : Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
    {
    }

    uint32_t GetComponentCount() const
    {
        switch (Type)
        {
            case ShaderDataType::Float:  return 1;
            case ShaderDataType::Float2: return 2;
            case ShaderDataType::Float3: return 3;
            case ShaderDataType::Float4: return 4;
            case ShaderDataType::Mat3:   return 3;
            case ShaderDataType::Mat4:   return 4;
            case ShaderDataType::Int:    return 1;
            case ShaderDataType::Int2:   return 2;
            case ShaderDataType::Int3:   return 3;
            case ShaderDataType::Int4:   return 4;
            case ShaderDataType::Bool:   return 1;
            case ShaderDataType::None:   break;
        }
        return 0;
    }
};

class BufferLayout
{
public:
    BufferLayout() = default;
    BufferLayout(std::initializer_list<BufferElement> elements)
        : m_Elements(elements)
    {
        CalculateOffsetsAndStride();
    }

    uint32_t GetStride() const { return m_Stride; }
    const std::vector<BufferElement>& GetElements() const { return m_Elements; }

    auto begin()       { return m_Elements.begin(); }
    auto end()         { return m_Elements.end(); }
    auto begin() const { return m_Elements.begin(); }
    auto end()   const { return m_Elements.end(); }

private:
    void CalculateOffsetsAndStride()
    {
        uint32_t offset = 0;
        m_Stride = 0;
        for (auto& element : m_Elements)
        {
            element.Offset = offset;
            offset += element.Size;
            m_Stride += element.Size;
        }
    }

    std::vector<BufferElement> m_Elements;
    uint32_t m_Stride = 0;
};

// ── PSO Enums ──

enum class DepthCompareFunc : uint8_t
{
    Never, Less, Equal, LessEqual, Greater, NotEqual, GreaterEqual, Always
};

enum class CullMode : uint8_t
{
    None, Front, Back, FrontAndBack
};

enum class FrontFace : uint8_t
{
    CW, CCW
};

enum class FillMode : uint8_t
{
    Solid, Wireframe, Point
};

enum class BlendFactor : uint8_t
{
    Zero, One, SrcColor, OneMinusSrcColor, DstColor, OneMinusDstColor,
    SrcAlpha, OneMinusSrcAlpha, DstAlpha, OneMinusDstAlpha,
    ConstantColor, OneMinusConstantColor, ConstantAlpha, OneMinusConstantAlpha
};

enum class BlendOp : uint8_t
{
    Add, Subtract, ReverseSubtract, Min, Max
};

enum class ColorWriteMask : uint8_t
{
    None = 0, Red = 1, Green = 2, Blue = 4, Alpha = 8, All = 15
};

enum class StencilOp : uint8_t
{
    Keep, Zero, Replace, IncrClamp, DecrClamp, Invert, IncrWrap, DecrWrap
};

} // namespace RHI
} // namespace AF

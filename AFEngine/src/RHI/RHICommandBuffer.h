#pragma once

#include "Core/Types.h"
#include "RHI/RHITypes.h"
#include "RHI/RHIShader.h"
#include "RHI/RHITexture.h"
#include "RHI/RHIVertexArray.h"
#include "RHI/RHIFramebuffer.h"
#include "RHI/RHIUniformBuffer.h"
#include "RHI/RHIStorageBuffer.h"

#include <glm/glm.hpp>
#include <variant>
#include <vector>
#include <string>
#include <cstdint>

namespace AF {
namespace RHI {

class RHIDevice;

struct CmdSetViewport     { uint32_t X = 0, Y = 0, Width = 0, Height = 0; };
struct CmdSetClearColor   { glm::vec4 Color{}; };
struct CmdClear           {};
struct CmdBindShader      { Ref<RHIShader> Shader; };
struct CmdSetMat4         { std::string Name; glm::mat4 Value{}; };
struct CmdSetFloat4       { std::string Name; glm::vec4 Value{}; };
struct CmdSetFloat3       { std::string Name; glm::vec3 Value{}; };
struct CmdSetFloat        { std::string Name; float Value = 0.0f; };
struct CmdSetInt          { std::string Name; int Value = 0; };
struct CmdBindTexture     { uint32_t Slot = 0; Ref<RHITexture2D> Texture; };
struct CmdDrawIndexed     { Ref<RHIVertexArray> VAO; uint32_t IndexCount = 0; };
struct CmdBindFramebuffer   { RHIFramebuffer* FBO = nullptr; };
struct CmdUnbindFramebuffer { RHIFramebuffer* FBO = nullptr; };

struct CmdBindUniformBuffer  { RHIUniformBuffer* Buffer = nullptr; uint32_t Binding = 0; };
struct CmdSetBufferData      { RHIUniformBuffer* Buffer = nullptr; uint32_t Offset = 0; std::vector<uint8_t> Data; };
struct CmdBindStorageBuffer  { RHIStorageBuffer* Buffer = nullptr; uint32_t Binding = 0; };
struct CmdSetStorageBufferData { RHIStorageBuffer* Buffer = nullptr; uint32_t Offset = 0; std::vector<uint8_t> Data; };

struct CmdSetDepthStencilState { DepthCompareFunc DepthFunc = DepthCompareFunc::Less; bool DepthTest = false; bool DepthWrite = true; };
struct CmdSetRasterizerState   { CullMode Cull = CullMode::Back; FrontFace Winding = FrontFace::CCW; FillMode Fill = FillMode::Solid; };
struct CmdSetBlendState        { uint32_t Attachment = 0; bool Enable = false;
    BlendFactor SrcColor = BlendFactor::One; BlendFactor DstColor = BlendFactor::Zero; BlendOp ColorOp = BlendOp::Add;
    BlendFactor SrcAlpha = BlendFactor::One; BlendFactor DstAlpha = BlendFactor::Zero; BlendOp AlphaOp = BlendOp::Add;
    ColorWriteMask WriteMask = ColorWriteMask::All; };

using RHICommand = std::variant<
    CmdSetViewport,
    CmdSetClearColor,
    CmdClear,
    CmdBindShader,
    CmdSetMat4,
    CmdSetFloat4,
    CmdSetFloat3,
    CmdSetFloat,
    CmdSetInt,
    CmdBindTexture,
    CmdDrawIndexed,
    CmdBindFramebuffer,
    CmdUnbindFramebuffer,
    CmdBindUniformBuffer,
    CmdSetBufferData,
    CmdBindStorageBuffer,
    CmdSetStorageBufferData,
    CmdSetDepthStencilState,
    CmdSetRasterizerState,
    CmdSetBlendState
>;

class RHICommandBuffer : public NonCopyable
{
public:
    RHICommandBuffer() = default;

    void Begin();
    void End();
    void Reset();

    void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void SetClearColor(const glm::vec4& color);
    void Clear();

    void BindShader(const Ref<RHIShader>& shader);
    void SetMat4(const std::string& name, const glm::mat4& value);
    void SetFloat4(const std::string& name, const glm::vec4& value);
    void SetFloat3(const std::string& name, const glm::vec3& value);
    void SetFloat(const std::string& name, float value);
    void SetInt(const std::string& name, int value);
    void BindTexture(uint32_t slot, const Ref<RHITexture2D>& texture);

    void DrawIndexed(const Ref<RHIVertexArray>& vao, uint32_t indexCount);

    void BindFramebuffer(RHIFramebuffer* fbo);
    void UnbindFramebuffer(RHIFramebuffer* fbo);

    void BindUniformBuffer(RHIUniformBuffer* buffer, uint32_t binding);
    void SetBufferData(RHIUniformBuffer* buffer, const void* data, uint32_t size, uint32_t offset = 0);
    void BindStorageBuffer(RHIStorageBuffer* buffer, uint32_t binding);
    void SetStorageBufferData(RHIStorageBuffer* buffer, const void* data, uint32_t size, uint32_t offset = 0);

    void SetDepthStencilState(bool depthTest, bool depthWrite, DepthCompareFunc depthFunc = DepthCompareFunc::Less);
    void SetRasterizerState(CullMode cull, FrontFace winding = FrontFace::CCW, FillMode fill = FillMode::Solid);
    void SetBlendState(uint32_t attachment, bool enable);

    void Execute(RHIDevice& device);

private:
    std::vector<RHICommand> m_Commands;
    bool m_Recording = false;
};

} // namespace RHI
} // namespace AF

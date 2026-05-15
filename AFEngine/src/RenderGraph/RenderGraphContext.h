#pragma once

#include "Core/Types.h"
#include "RHI/RHITexture.h"
#include "RHI/RHICommandBuffer.h"
#include "Renderer/RenderView.h"
#include "Renderer/RenderPacket.h"

#include <cstdint>
#include <string>

namespace AF {

class RenderGraph;

using RenderResource = uint32_t;
constexpr RenderResource NullResource = UINT32_MAX;

class RenderGraphContext
{
public:
    const RenderView& GetView() const;
    const RenderPacket& GetPacket() const;

    Ref<RHI::RHITexture2D> GetInput(RenderResource id) const;
    Ref<RHI::RHITexture2D> GetInput(const std::string& name) const;

    RHI::RHICommandBuffer* Cmd = nullptr;

private:
    friend class RenderGraph;

    const RenderView*   m_View   = nullptr;
    const RenderPacket* m_Packet = nullptr;
    const RenderGraph*  m_Graph  = nullptr;
};

} // namespace AF

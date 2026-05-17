#pragma once

// CameraGPU —— 发送到 GPU Uniform Buffer 的相机数据结构体
//
// 布局需满足 std140 对齐规则，保证 invec3 后按 16 字节对齐。
// 由 DeferredRenderPipeline 每帧填充并上传。

#include <glm/glm.hpp>

namespace AF {

struct CameraGPU
{
    glm::mat4 ViewProjection        = glm::mat4(1.0f);
    glm::mat4 InverseViewProjection = glm::mat4(1.0f);
    glm::mat4 Projection            = glm::mat4(1.0f);
    glm::vec3 Position              = {};
    float     _pad0                 = 0.0f;
    glm::vec2 ScreenSize            = {};
    float     _pad1[2]              = {};
    glm::vec3 ViewDirection         = {};
    float     _pad2                 = 0.0f;
    float     Time                  = 0.0f;
    float     DeltaTime             = 0.0f;
};

} // namespace AF

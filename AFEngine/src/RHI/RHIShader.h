#pragma once

// RHIShader —— 着色器抽象接口
//
// 支持从文件路径和内联源码两种方式创建（通过 ShaderSourceFile 结构体）。
// 提供 uniform 设置方法，自动反射获取纹理绑定和 uniform 位置。

#include "Core/Types.h"
#include "RHI/ShaderReflection.h"

#include <glm/glm.hpp>
#include <initializer_list>
#include <string>

namespace AF {
namespace RHI {

enum class ShaderStage
{
    Vertex,
    Fragment,
};

/// 着色器源描述（文件路径或内联源码二选一）
struct ShaderSourceFile
{
    ShaderStage Stage;
    std::string Filepath;   // 磁盘路径（与 SourceCode 互斥）
    std::string SourceCode; // 内联源码（与 Filepath 互斥）
};

class RHIShader
{
public:
    virtual ~RHIShader() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void SetInt(const std::string& name, int value) = 0;
    virtual void SetFloat(const std::string& name, float value) = 0;
    virtual void SetFloat2(const std::string& name, const glm::vec2& value) = 0;
    virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
    virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
    virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

    virtual const std::string& GetName() const = 0;

    /// 收集着色器反射信息（uniform 位置、纹理槽位等）
    virtual ShaderReflection CollectReflection() const { return {}; }

    /// 从文件列表创建（ShaderSourceFile 数组）
    static Ref<RHIShader> Create(const std::string& name,
                                  std::initializer_list<ShaderSourceFile> sources);

    /// 从源码字符串创建
    static Ref<RHIShader> Create(const std::string& name,
                                  const std::string& vertexSource,
                                  const std::string& fragmentSource);
};

} // namespace RHI
} // namespace AF

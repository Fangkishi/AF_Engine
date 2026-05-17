#include "RHI/OpenGL/GLShader.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "Core/Assert.h"
#include "Core/Log.h"

#include <fstream>
#include <sstream>

namespace AF {
namespace RHI {

// ── 工具函数 ──

unsigned int GLShader::StageToGL(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::Vertex:   return GL_VERTEX_SHADER;
        case ShaderStage::Fragment: return GL_FRAGMENT_SHADER;
    }
    AF_LOG_ERROR("Unknown shader stage");
    return 0;
}

// ── 构造/析构 ──

GLShader::GLShader(const std::string& name, const std::vector<ShaderSourceFile>& sources)
    : m_Name(name)
{
    // 收集各阶段源码（文件或内联）
    std::unordered_map<unsigned int, std::string> shaderSources;

    for (auto& src : sources)
    {
        std::string code;
        if (!src.SourceCode.empty())
            code = src.SourceCode;
        else
            code = ReadFile(src.Filepath);
        if (!code.empty())
            shaderSources[StageToGL(src.Stage)] = std::move(code);
    }

    Compile(shaderSources);
}

GLShader::~GLShader()
{
    glDeleteProgram(m_RendererID);
}

std::string GLShader::ReadFile(const std::string& filepath)
{
    std::ifstream in(filepath, std::ios::in | std::ios::binary);
    if (in)
    {
        std::ostringstream contents;
        contents << in.rdbuf();
        in.close();
        return contents.str();
    }
    AF_LOG_ERROR("Could not open shader file: {}", filepath);
    return {};
}

// ── 编译链接 ──

void GLShader::Compile(const std::unordered_map<unsigned int, std::string>& shaderSources)
{
    unsigned int program = glCreateProgram();
    std::vector<unsigned int> glShaderIDs;

    for (auto& [type, source] : shaderSources)
    {
        unsigned int shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);

        int isCompiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            int maxLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
            std::vector<char> infoLog(maxLength);
            glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);
            glDeleteShader(shader);
            AF_LOG_ERROR("Shader compilation failed:\n{}", infoLog.data());
            AF_CORE_ASSERT(false, "Shader compilation failed!");
        }

        glAttachShader(program, shader);
        glShaderIDs.push_back(shader);
    }

    glLinkProgram(program);
    int isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        int maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<char> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);
        glDeleteProgram(program);
        for (auto id : glShaderIDs) glDeleteShader(id);
        AF_LOG_ERROR("Shader link failed:\n{}", infoLog.data());
        AF_CORE_ASSERT(false, "Shader link failed!");
    }

    for (auto id : glShaderIDs) glDetachShader(program, id);

    m_RendererID = program;
}

// ── 绑定与 uniform 设置 ──

void GLShader::Bind() const
{
    glUseProgram(m_RendererID);
}

void GLShader::Unbind() const
{
    glUseProgram(0);
}

int GLShader::GetUniformLocation(const std::string& name) const
{
    auto it = m_UniformLocationCache.find(name);
    if (it != m_UniformLocationCache.end())
        return it->second;

    int location = glGetUniformLocation(m_RendererID, name.c_str());
    m_UniformLocationCache[name] = location;
    return location;
}

void GLShader::SetInt(const std::string& name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void GLShader::SetFloat(const std::string& name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void GLShader::SetFloat2(const std::string& name, const glm::vec2& value)
{
    glUniform2f(GetUniformLocation(name), value.x, value.y);
}

void GLShader::SetFloat3(const std::string& name, const glm::vec3& value)
{
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

void GLShader::SetFloat4(const std::string& name, const glm::vec4& value)
{
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

void GLShader::SetMat4(const std::string& name, const glm::mat4& value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

// ── 反射 ──

ShaderReflection GLShader::CollectReflection() const
{
    ShaderReflection reflection;

    // 收集所有 active uniform 的名字和位置
    GLint numUniforms = 0;
    glGetProgramiv(m_RendererID, GL_ACTIVE_UNIFORMS, &numUniforms);

    constexpr GLsizei bufSize = 256;
    char nameBuf[bufSize];

    for (GLint i = 0; i < numUniforms; ++i)
    {
        GLsizei length = 0;
        GLint size = 0;
        GLenum type = 0;
        glGetActiveUniform(m_RendererID, i, bufSize, &length, &size, &type, nameBuf);
        std::string name(nameBuf, length);

        GLint location = glGetUniformLocation(m_RendererID, name.c_str());
        reflection.UniformLocations[name] = location;

        // 检测 sampler 类型，读取 layout(binding=N) 值
        if (type == GL_SAMPLER_2D || type == GL_SAMPLER_CUBE ||
            type == GL_SAMPLER_3D || type == GL_SAMPLER_2D_SHADOW)
        {
            GLint binding = 0;
            glGetUniformiv(m_RendererID, location, &binding);
            reflection.TextureBindings[name] = static_cast<uint32_t>(binding);
        }
    }

    // 收集 uniform block binding
    GLint numBlocks = 0;
    glGetProgramiv(m_RendererID, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);
    for (GLint i = 0; i < numBlocks; ++i)
    {
        GLsizei length = 0;
        glGetActiveUniformBlockName(m_RendererID, i, bufSize, &length, nameBuf);
        std::string name(nameBuf, length);

        GLint binding = 0;
        glGetActiveUniformBlockiv(m_RendererID, i, GL_UNIFORM_BLOCK_BINDING, &binding);
        reflection.UniformBlockBindings[name] = static_cast<uint32_t>(binding);
    }

    return reflection;
}

// ── 工厂 ──

Ref<RHIShader> RHIShader::Create(const std::string& name,
                                  std::initializer_list<ShaderSourceFile> sources)
{
    return std::make_shared<GLShader>(name, std::vector<ShaderSourceFile>(sources));
}

Ref<RHIShader> RHIShader::Create(const std::string& name,
                                  const std::string& vertexSource,
                                  const std::string& fragmentSource)
{
    std::vector<ShaderSourceFile> sources;
    sources.push_back({ ShaderStage::Vertex, "", vertexSource });
    sources.push_back({ ShaderStage::Fragment, "", fragmentSource });
    return std::make_shared<GLShader>(name, sources);
}

} // namespace RHI
} // namespace AF

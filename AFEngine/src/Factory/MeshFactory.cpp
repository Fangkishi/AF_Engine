#include "Factory/MeshFactory.h"

#include "RHI/RHITypes.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <cmath>

namespace AF {

static RHI::BufferLayout MakePrimitiveLayout()
{
    return {
        { RHI::ShaderDataType::Float3, "a_Position" },
        { RHI::ShaderDataType::Float3, "a_Normal"   },
        { RHI::ShaderDataType::Float4, "a_Tangent"  },
        { RHI::ShaderDataType::Float2, "a_TexCoord" },
        { RHI::ShaderDataType::Float4, "a_Color"    },
    };
}

static glm::vec4 NormalToColor(const glm::vec3& n)
{
    glm::vec3 c = (n + 1.0f) * 0.5f;
    return glm::vec4(c, 1.0f);
}

// Push 11 floats: pos(3) + normal(3) + tangent(4) + uv(2) + color(4) = 16? 
// Wait: 3+3+4+2+4 = 16? No: 3+3+4+2+4 = 16 components, stride = 64 bytes
static void PushVertex(std::vector<float>& v, const glm::vec3& pos, const glm::vec3& normal,
                        const glm::vec4& tangent, const glm::vec2& uv, const glm::vec4& color)
{
    v.insert(v.end(), { pos.x, pos.y, pos.z, normal.x, normal.y, normal.z,
        tangent.x, tangent.y, tangent.z, tangent.w, uv.x, uv.y, color.x, color.y, color.z, color.w });
}

// ── Cube ──

Ref<Mesh> MeshFactory::CreateCube(float size)
{
    float h = size * 0.5f;

    struct Face { glm::vec3 n; glm::vec3 u; glm::vec3 v; };
    Face faces[6] = {
        {{ 1, 0, 0}, {0, 0,-1}, {0,-1, 0}},
        {{-1, 0, 0}, {0, 0, 1}, {0,-1, 0}},
        {{ 0, 1, 0}, {1, 0, 0}, {0, 0, 1}},
        {{ 0,-1, 0}, {1, 0, 0}, {0, 0,-1}},
        {{ 0, 0, 1}, {1, 0, 0}, {0,-1, 0}},
        {{ 0, 0,-1}, {-1,0, 0}, {0,-1, 0}},
    };

    std::vector<float> verts;
    std::vector<uint32_t> indices;

    for (int f = 0; f < 6; ++f)
    {
        auto& face = faces[f];
        glm::vec4 tangent(glm::normalize(face.u), 1.0f);
        glm::vec4 color = NormalToColor(face.n);

        glm::vec3 p0 = (face.n - face.u - face.v) * h;
        glm::vec3 p1 = (face.n - face.u + face.v) * h;
        glm::vec3 p2 = (face.n + face.u + face.v) * h;
        glm::vec3 p3 = (face.n + face.u - face.v) * h;

        uint32_t base = static_cast<uint32_t>(verts.size() / 16); // 16 = floats per vertex
        PushVertex(verts, p0, face.n, tangent, {0, 0}, color);
        PushVertex(verts, p1, face.n, tangent, {1, 0}, color);
        PushVertex(verts, p2, face.n, tangent, {1, 1}, color);
        PushVertex(verts, p3, face.n, tangent, {0, 1}, color);
        indices.insert(indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });
    }

    return std::make_shared<Mesh>(verts, indices, MakePrimitiveLayout());
}

// ── Plane ──

Ref<Mesh> MeshFactory::CreatePlane(float size)
{
    float h = size * 0.5f;
    glm::vec3 n(0.0f, 1.0f, 0.0f);
    glm::vec4 tangent(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 color = NormalToColor(n);

    std::vector<float> verts;
    PushVertex(verts, {-h, 0, -h}, n, tangent, {0, 0}, color);
    PushVertex(verts, { h, 0, -h}, n, tangent, {1, 0}, color);
    PushVertex(verts, { h, 0,  h}, n, tangent, {1, 1}, color);
    PushVertex(verts, {-h, 0,  h}, n, tangent, {0, 1}, color);

    std::vector<uint32_t> indices = { 0, 2, 1, 0, 3, 2 };
    return std::make_shared<Mesh>(verts, indices, MakePrimitiveLayout());
}

// ── Sphere ──

Ref<Mesh> MeshFactory::CreateSphere(float radius, uint32_t segments)
{
    uint32_t stacks = segments;
    uint32_t slices = segments * 2;

    std::vector<float> verts;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i <= stacks; ++i)
    {
        float theta = static_cast<float>(i) * glm::pi<float>() / static_cast<float>(stacks);
        float sinT = std::sin(theta);
        float cosT = std::cos(theta);

        for (uint32_t j = 0; j <= slices; ++j)
        {
            float phi = static_cast<float>(j) * 2.0f * glm::pi<float>() / static_cast<float>(slices);
            float sinP = std::sin(phi);
            float cosP = std::cos(phi);

            glm::vec3 pos(sinT * cosP * radius, cosT * radius, sinT * sinP * radius);
            glm::vec3 normal = glm::normalize(pos);

            glm::vec3 dPdu(-sinT * sinP, 0.0f, sinT * cosP);
            glm::vec3 tangentVec;
            float w = 1.0f;
            if (glm::length(dPdu) > 0.0001f)
            {
                tangentVec = glm::normalize(dPdu);
                glm::vec3 bitangent(cosT * cosP, -sinT, cosT * sinP);
                w = (glm::dot(glm::cross(normal, tangentVec), bitangent) >= 0.0f) ? 1.0f : -1.0f;
            }
            else
            {
                tangentVec = glm::vec3(1.0f, 0.0f, 0.0f);
            }

            glm::vec4 tangent(tangentVec, w);
            glm::vec2 uv(static_cast<float>(j) / static_cast<float>(slices),
                          static_cast<float>(i) / static_cast<float>(stacks));
            glm::vec4 color = NormalToColor(normal);

            PushVertex(verts, pos, normal, tangent, uv, color);
        }
    }

    for (uint32_t i = 0; i < stacks; ++i)
    {
        for (uint32_t j = 0; j < slices; ++j)
        {
            uint32_t first = i * (slices + 1) + j;
            uint32_t second = first + slices + 1;
            indices.insert(indices.end(), { first, first + 1, second, first + 1, second + 1, second });
        }
    }

    return std::make_shared<Mesh>(verts, indices, MakePrimitiveLayout());
}

// ── Cylinder ──

Ref<Mesh> MeshFactory::CreateCylinder(float radius, float height, uint32_t segments)
{
    std::vector<float> verts;
    std::vector<uint32_t> indices;

    float halfH = height * 0.5f;

    // Side wall
    for (uint32_t j = 0; j <= segments; ++j)
    {
        float phi = static_cast<float>(j) * 2.0f * glm::pi<float>() / static_cast<float>(segments);
        float x = std::cos(phi);
        float z = std::sin(phi);
        glm::vec3 normal(x, 0.0f, z);
        float w = (j < segments) ? 1.0f : (glm::dot(glm::cross(normal, glm::vec3(-x, 0.0f, -z)), glm::vec3(0.0f, 0.0f, 0.0f)) >= 0 ? 1.0f : -1.0f);
        glm::vec4 tangent(-z, 0.0f, x, 1.0f);
        glm::vec4 color = NormalToColor(normal);
        glm::vec2 uv(static_cast<float>(j) / static_cast<float>(segments), 1.0f);

        PushVertex(verts, {x * radius, halfH, z * radius}, normal, tangent, uv, color);
        PushVertex(verts, {x * radius, -halfH, z * radius}, normal, tangent, {uv.x, 0.0f}, color);
    }

    for (uint32_t j = 0; j < segments; ++j)
    {
        uint32_t t0 = j * 2, b0 = j * 2 + 1, t1 = (j + 1) * 2, b1 = (j + 1) * 2 + 1;
        indices.insert(indices.end(), { t0, t1, b0, t1, b1, b0 });
    }

    // Top cap
    glm::vec3 topN(0.0f, 1.0f, 0.0f);
    glm::vec4 topT(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 topColor = NormalToColor(topN);
    uint32_t topBase = static_cast<uint32_t>(verts.size() / 16);
    PushVertex(verts, {0.0f, halfH, 0.0f}, topN, topT, {0.5f, 0.5f}, topColor);
    for (uint32_t j = 0; j <= segments; ++j)
    {
        float phi = static_cast<float>(j) * 2.0f * glm::pi<float>() / static_cast<float>(segments);
        float x = std::cos(phi) * radius;
        float z = std::sin(phi) * radius;
        float u = x / (radius * 2.0f) + 0.5f;
        float v = z / (radius * 2.0f) + 0.5f;
        PushVertex(verts, {x, halfH, z}, topN, topT, {u, v}, topColor);
    }
    for (uint32_t j = 0; j < segments; ++j)
        indices.insert(indices.end(), { topBase, topBase + 2 + j, topBase + 1 + j });

    // Bottom cap
    glm::vec3 botN(0.0f, -1.0f, 0.0f);
    glm::vec4 botT(1.0f, 0.0f, 0.0f, -1.0f);
    glm::vec4 botColor = NormalToColor(botN);
    uint32_t botBase = static_cast<uint32_t>(verts.size() / 16);
    PushVertex(verts, {0.0f, -halfH, 0.0f}, botN, botT, {0.5f, 0.5f}, botColor);
    for (uint32_t j = 0; j <= segments; ++j)
    {
        float phi = static_cast<float>(j) * 2.0f * glm::pi<float>() / static_cast<float>(segments);
        float x = std::cos(phi) * radius;
        float z = std::sin(phi) * radius;
        float u = x / (radius * 2.0f) + 0.5f;
        float v = z / (radius * 2.0f) + 0.5f;
        PushVertex(verts, {x, -halfH, z}, botN, botT, {u, v}, botColor);
    }
    for (uint32_t j = 0; j < segments; ++j)
        indices.insert(indices.end(), { botBase, botBase + 1 + j, botBase + 2 + j });

    return std::make_shared<Mesh>(verts, indices, MakePrimitiveLayout());
}

// ── Capsule ──

Ref<Mesh> MeshFactory::CreateCapsule(float radius, float height, uint32_t segments)
{
    uint32_t stacks = segments;
    uint32_t slices = segments * 2;
    float halfH = height * 0.5f;

    std::vector<float> verts;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i <= stacks; ++i)
    {
        float theta = static_cast<float>(i) * glm::pi<float>() / static_cast<float>(stacks);
        float sinT = std::sin(theta);
        float cosT = std::cos(theta);

        for (uint32_t j = 0; j <= slices; ++j)
        {
            float phi = static_cast<float>(j) * 2.0f * glm::pi<float>() / static_cast<float>(slices);
            float sinP = std::sin(phi);
            float cosP = std::cos(phi);

            glm::vec3 pos(sinT * cosP * radius, cosT * radius, sinT * sinP * radius);
            glm::vec3 normal = glm::normalize(pos);

            if (pos.y >= 0.0f) pos.y += halfH;
            else               pos.y -= halfH;

            glm::vec3 dPdu(-sinT * sinP, 0.0f, sinT * cosP);
            glm::vec3 tangentVec;
            float w = 1.0f;
            if (glm::length(dPdu) > 0.0001f)
            {
                tangentVec = glm::normalize(dPdu);
                glm::vec3 bitangent(cosT * cosP, -sinT, cosT * sinP);
                w = (glm::dot(glm::cross(normal, tangentVec), bitangent) >= 0.0f) ? 1.0f : -1.0f;
            }
            else
            {
                tangentVec = glm::vec3(1.0f, 0.0f, 0.0f);
            }

            glm::vec4 tangent(tangentVec, w);
            glm::vec2 uv(static_cast<float>(j) / static_cast<float>(slices),
                          static_cast<float>(i) / static_cast<float>(stacks));
            glm::vec4 color = NormalToColor(normal);

            PushVertex(verts, pos, normal, tangent, uv, color);
        }
    }

    for (uint32_t i = 0; i < stacks; ++i)
    {
        for (uint32_t j = 0; j < slices; ++j)
        {
            uint32_t first = i * (slices + 1) + j;
            uint32_t second = first + slices + 1;
            indices.insert(indices.end(), { first, first + 1, second, first + 1, second + 1, second });
        }
    }

    return std::make_shared<Mesh>(verts, indices, MakePrimitiveLayout());
}

// ── Triangle (full attributes) ──

Ref<Mesh> MeshFactory::CreateTriangle()
{
    glm::vec3 n(0.0f, 0.0f, 1.0f);
    glm::vec4 tangent(1.0f, 0.0f, 0.0f, 1.0f);

    std::vector<float> verts;
    PushVertex(verts, {-0.5f, -0.5f, 0.0f}, n, tangent, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f});
    PushVertex(verts, { 0.5f, -0.5f, 0.0f}, n, tangent, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f});
    PushVertex(verts, { 0.0f,  0.5f, 0.0f}, n, tangent, {0.5f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f});

    std::vector<uint32_t> indices = { 0, 1, 2 };
    return std::make_shared<Mesh>(verts, indices, MakePrimitiveLayout());
}

// ── Quad (full attributes, fullscreen) ──

Ref<Mesh> MeshFactory::CreateQuad(float size)
{
    float h = size * 0.5f;
    glm::vec3 n(0.0f, 0.0f, 1.0f);
    glm::vec4 tangent(1.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 white(1.0f);

    std::vector<float> verts;
    PushVertex(verts, {-h, -h, 0.0f}, n, tangent, {0.0f, 0.0f}, white);
    PushVertex(verts, { h, -h, 0.0f}, n, tangent, {1.0f, 0.0f}, white);
    PushVertex(verts, { h,  h, 0.0f}, n, tangent, {1.0f, 1.0f}, white);
    PushVertex(verts, {-h,  h, 0.0f}, n, tangent, {0.0f, 1.0f}, white);

    std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };
    return std::make_shared<Mesh>(verts, indices, MakePrimitiveLayout());
}

} // namespace AF

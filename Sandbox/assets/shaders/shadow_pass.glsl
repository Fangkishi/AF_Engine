#type vertex
#version 450 core

// --- 阴影渲染阶段：顶点着色器 ---
// 负责将顶点变换到世界空间。
// 几何着色器（Geometry Shader）将接管后续的灯光空间变换和层级选择。

layout(location = 0) in vec3 a_Position;

uniform mat4 u_Transform; // 模型变换矩阵

void main() {
    // 仅变换到世界空间，后续由几何着色器处理
    gl_Position = u_Transform * vec4(a_Position, 1.0);
}

#type geometry
#version 450 core

// --- 阴影渲染阶段：几何着色器 ---
// 核心职责：
// 1. 实现分层渲染（Layered Rendering）：一次绘制将三角形发送到多个阴影图层（gl_Layer）。
// 2. 批量处理灯光：根据 u_LightCount 循环，减少 CPU 端的 Draw Call 压力。

layout(triangles) in;
// 顶点输出限制计算：
// 点光源：6 个面 * 3 顶点 = 18 顶点/灯光。96 / 18 ≈ 5 个点光源/批次。
// 方向光：1 个面 * 3 顶点 = 3 顶点/灯光。96 / 3 = 32 个方向光/批次。
layout(triangle_strip, max_vertices = 96) out;

// --- 数据结构 ---
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    mat4 LightSpaceMatrix; // 灯光空间变换矩阵
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    mat4 LightSpaceMatrix[6]; // 点光源 6 个方向的变换矩阵
};

// --- 资源绑定 ---
layout(std430, binding = 0) buffer DirLights {
    DirLight dirLights[];
};

layout(std430, binding = 1) buffer PointLights {
    PointLight pointLights[];
};

// --- Uniforms ---
uniform int u_LightType;  // 0: 方向光, 1: 点光源
uniform int u_LightStart; // 当前批次在 SSBO 中的起始索引
uniform int u_LightCount; // 当前批次需要处理的灯光数量

void main() {
    for (int i = 0; i < u_LightCount; ++i) {
        int lightIndex = u_LightStart + i;

        if (u_LightType == 0) {
            // --- 方向光阴影处理 ---
            // 每一层（gl_Layer）对应一个方向光源的阴影图
            gl_Layer = lightIndex; 
            for(int j = 0; j < 3; ++j) {
                gl_Position = dirLights[lightIndex].LightSpaceMatrix * gl_in[j].gl_Position;
                EmitVertex();
            }
            EndPrimitive();
        } 
        else if (u_LightType == 1) {
            // --- 点光源阴影处理 ---
            // 每一个点光源需要渲染到 6 层（对应立方体贴图的 6 个面）
            // 布局：Layer = 光源索引 * 6 + 面索引
            for(int face = 0; face < 6; ++face) {
                gl_Layer = lightIndex * 6 + face;
                for(int j = 0; j < 3; ++j) {
                    gl_Position = pointLights[lightIndex].LightSpaceMatrix[face] * gl_in[j].gl_Position;
                    EmitVertex();
                }
                EndPrimitive();
            }
        }
    }
}

#type fragment
#version 450 core

// --- 阴影渲染阶段：片元着色器 ---
// 仅用于深度写入，硬件会自动处理 gl_FragDepth。
// 此处无需输出颜色。

void main() {
    // 深度值自动写入深度缓冲
}

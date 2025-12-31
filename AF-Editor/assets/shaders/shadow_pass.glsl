#type vertex
#version 450 core

// 阴影渲染管线 - 顶点阶段
// 负责将场景物体变换到灯光空间，以生成深度图 (Shadow Map)

layout(location = 0) in vec3 a_Position;   // 顶点位置

uniform mat4 u_Transform;                  // 模型变换矩阵

// --- 光源结构定义 ---
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    mat4 LightSpaceMatrix;                 // 方向光空间变换矩阵
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    mat4 LightSpaceMatrix[6];              // 点光源 6 个方向的空间变换矩阵
};

// 光源 SSBO
layout(std430, binding = 0) buffer DirLights {
    DirLight dirLights[];
};

layout(std430, binding = 1) buffer PointLights {
    PointLight pointLights[];
};

uniform int u_LightType;                   // 灯光类型 (0: 方向光, 1: 点光源)
uniform int u_LightIndex;                  // 灯光在数组中的索引
uniform int u_TexIndex;                    // 纹理面索引 (仅对点光源有效，0-5 对应立方体贴图的 6 个面)

void main() {
    mat4 lightSpaceMatrix;

    // 根据灯光类型选择合适的变换矩阵
    if (u_LightType == 0) {
        // 方向光阴影
        lightSpaceMatrix = dirLights[u_LightIndex].LightSpaceMatrix;
    } else if (u_LightType == 1) {
        // 点光源阴影 (渲染到立方体贴图的某一个面)
        lightSpaceMatrix = pointLights[u_LightIndex].LightSpaceMatrix[u_TexIndex];
    }

    // 将顶点变换到灯光空间的裁剪坐标系
    gl_Position = lightSpaceMatrix * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

// 阴影渲染管线 - 片段阶段
// 此阶段通常为空，因为我们只需要深度信息，硬件会自动处理深度缓冲的写入

void main() {
    // 不需要颜色输出
}
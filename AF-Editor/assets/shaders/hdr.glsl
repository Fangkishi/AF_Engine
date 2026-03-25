// 后处理 - HDR 曝光处理着色器
// 负责将 HDR 渲染结果应用曝光参数

#type vertex
#version 450 core

// 顶点属性输入 (通常是一个铺满屏幕的 Quad)
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    gl_Position = vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

// 输入的渲染结果 (HDR 直接光照)
uniform sampler2D LightingResult;
// 输入的降噪结果 (SSAO, SSGI 等间接光/遮蔽信息)
uniform sampler2D DenoisedResult;

// 曝光参数
uniform float u_Exposure = 1.0;
// 是否启用 SSGI/SSAO
uniform bool u_EnableSSGI = true;

void main()
{
    // 采样直接光照阶段的结果
    vec3 directLight = texture(LightingResult, v_TexCoord).rgb;
    
    vec3 combinedColor = directLight;

    if (u_EnableSSGI) {
        // 采样间接光照与遮蔽信息 (假设 DenoisedResult 格式为 RGBA: RGB 是 SSGI 间接光，A 是 SSAO 遮蔽因子)
        // 根据实际设计，您也可以分开采样，这里假设合并在了一张纹理中
        vec4 denoisedData = texture(DenoisedResult, v_TexCoord);
        vec3 indirectLight = denoisedData.rgb;
        float aoFactor = denoisedData.a;

        // 组合直接光照和间接光照，并应用环境光遮蔽
        // 注意：AO 应该同时遮蔽间接光(SSGI)和基础环境光(如果 directLight 中包含的话)
        // 为了让 AO 更明显，我们可以让它甚至轻微影响一些直接光照的暗部，或者加深遮蔽乘数
        combinedColor = (directLight + indirectLight) * aoFactor;
    }

    // 应用曝光处理
    vec3 exposedColor = combinedColor * u_Exposure;

    // 色调映射 (Reinhard)
    vec3 mapped = exposedColor / (exposedColor + vec3(1.0));
    
    // Gamma 校正 (在线性空间完成所有光照和混合后，最后一步转为 sRGB)
    mapped = pow(mapped, vec3(1.0/2.2));

    // 输出最终颜色
    o_Color = vec4(mapped, 1.0);
}

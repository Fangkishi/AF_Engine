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

// 输入的渲染结果 (HDR)
uniform sampler2D LightingResult;
// 曝光参数
uniform float u_Exposure = 1.0;

void main()
{
    // 采样光照阶段的结果
    vec4 texColor = texture(LightingResult, v_TexCoord);

    // 应用曝光处理
    vec3 exposedColor = texColor.rgb * u_Exposure;

    // 输出最终颜色 (此处仅处理曝光，Tone Mapping 可能在其他阶段或后续添加)
    o_Color = vec4(exposedColor, 1.0);
}

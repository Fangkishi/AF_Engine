#type vertex
#version 450 core

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

in vec2 v_TexCoord;
layout(location = 0) out vec4 o_SSAOResult;

uniform sampler2D GBufferAlbedo;
uniform sampler2D GBufferNormal;
uniform sampler2D GBufferMP;
uniform sampler2D GBufferDepth;
uniform sampler2D u_EnvMap;

layout(std140, binding = 0) uniform Camera
{
    vec3 u_ViewPos;
    mat4 u_View;
    mat4 u_ViewInverse;
    mat4 u_Projection;
    mat4 u_ProjectionInverse;
};

// Reconstruct view space position
vec3 ReconstructViewPos(vec2 texCoord, float depth) {
    vec4 clipPos = vec4(texCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = u_ProjectionInverse * clipPos;
    return viewPos.xyz / viewPos.w;
}

// ---------------------------------------------------------
// 随机数与采样生成
// ---------------------------------------------------------
float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// 均匀半球采样
vec3 GetCosineWeightedSample(vec2 u, vec3 n) {
    float r = sqrt(u.x);
    float theta = 2.0 * 3.14159265359 * u.y;
    vec3 p = vec3(r * cos(theta), r * sin(theta), sqrt(max(0.0, 1.0 - u.x)));
    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 t = normalize(cross(up, n));
    vec3 b = cross(n, t);
    return p.x * t + p.y * b + p.z * n;
}

void main()
{
    float depth = texture(GBufferDepth, v_TexCoord).r;
    if (depth >= 1.0) {
        o_SSAOResult = vec4(1.0);
        return;
    }

    vec3 viewPos = ReconstructViewPos(v_TexCoord, depth);
    vec3 normal = texture(GBufferNormal, v_TexCoord).rgb * 2.0 - 1.0;
    
    // Transform normal to view space
    mat3 normalMatrix = transpose(inverse(mat3(u_View)));
    vec3 viewNormal = normalize(normalMatrix * normal);

    // 核心参数
    int samples = 32;       // 增加采样数，减少高频噪点
    float radius = 1.0;     
    float bias = 0.05;      
    float occlusion = 0.0;
    
    float seed = rand(v_TexCoord);

    for (int i = 0; i < samples; i++) {
        // 使用均匀分布的随机序列 (Hammersley 类似思想)
        vec2 u = vec2(
            fract(seed + float(i) * 0.618), 
            fract(seed + float(i) * 0.382)
        );
        
        // 生成偏向法线方向的半球采样向量
        vec3 sampleVec = GetCosineWeightedSample(u, viewNormal);
        
        // 加入一个随机长度缩放，避免所有采样点都在球壳边缘
        float len = fract(seed + float(i) * 0.123);
        // 让采样点更倾向于靠近中心
        len = mix(0.1, 1.0, len * len);
        sampleVec *= len * radius;

        vec3 samplePos = viewPos + sampleVec;

        // Project sample position to screen space
        vec4 offset = vec4(samplePos, 1.0);
        offset = u_Projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        // Sample depth
        float sampleDepth = texture(GBufferDepth, offset.xy).r;
        vec3 sampleViewPos = ReconstructViewPos(offset.xy, sampleDepth);

        // Range check
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(viewPos.z - sampleViewPos.z));
        // Note: in OpenGL right-handed view space, z is negative, closer to camera means LARGER z (e.g. -1 > -5)
        if (sampleViewPos.z >= samplePos.z + bias) {
            occlusion += 1.0 * rangeCheck;
        }
    }

    // Since occlusion accumulates when a sample is blocked, we subtract it from 1 to get visibility
    // Scale the occlusion to make it more visible for debugging/art direction
    // 由于我们增大了半径，可能需要稍微调整强度
    float intensity = 2.0; 
    float occlusionFactor = (occlusion / float(samples)) * intensity;
    
    // 限制在 0-1 之间
    occlusionFactor = clamp(occlusionFactor, 0.0, 1.0);
    float visibility = 1.0 - occlusionFactor;
    
    // Power function to increase contrast of AO
    visibility = pow(visibility, 1.5); // 稍微降低一点对比度指数，因为基础强度已经提高了

    vec4 envHack = texture(u_EnvMap, vec2(0.0)) * 0.000001;
    vec4 albedoHack = texture(GBufferAlbedo, vec2(0.0)) * 0.000001;
    vec4 mpHack = texture(GBufferMP, vec2(0.0)) * 0.000001;
    o_SSAOResult = vec4(vec3(visibility), 1.0) + envHack + albedoHack + mpHack;
}

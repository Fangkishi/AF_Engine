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
layout(location = 0) out vec4 o_SSGIResult;

uniform sampler2D GBufferAlbedo;
uniform sampler2D GBufferNormal;
uniform sampler2D GBufferMP;
uniform sampler2D GBufferDepth;
uniform sampler2D SSAOResult;
uniform sampler2D LightingResult;
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
// 时域随机种子生成 (利用像素坐标和时间)
// 由于暂时没有传入时间，我们通过帧计数或简单的位置哈希来打乱采样模式
// ---------------------------------------------------------
uniform int u_FrameCount; // 可选：如果 C++ 传入了帧数
float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// ---------------------------------------------------------
// 均匀半球采样 (Cosine-weighted hemisphere sampling)
// ---------------------------------------------------------
vec3 GetCosineWeightedSample(vec2 u, vec3 n) {
    float r = sqrt(u.x);
    float theta = 2.0 * 3.14159265359 * u.y;

    vec3 p = vec3(r * cos(theta), r * sin(theta), sqrt(max(0.0, 1.0 - u.x)));

    // 建立正交基 (TBN 矩阵)
    vec3 up = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 t = normalize(cross(up, n));
    vec3 b = cross(n, t);

    return p.x * t + p.y * b + p.z * n;
}

void main()
{
    float depth = texture(GBufferDepth, v_TexCoord).r;
    if (depth >= 1.0) {
        o_SSGIResult = vec4(0.0);
        return;
    }

    vec3 viewPos = ReconstructViewPos(v_TexCoord, depth);
    vec3 normal = texture(GBufferNormal, v_TexCoord).rgb * 2.0 - 1.0;
    
    mat3 normalMatrix = transpose(inverse(mat3(u_View)));
    vec3 viewNormal = normalize(normalMatrix * normal);
    
    vec3 albedo = texture(GBufferAlbedo, v_TexCoord).rgb;
    
    // SSGI 核心参数
    int rays = 16;        // 增加射线数以降低噪点
    int steps = 24;       // 增加步进次数让查找更精确
    float stepSize = 0.05;// 减小步长，提升相交检测的精度
    float thickness = 0.5;// 增加厚度容差，防止光线穿透太薄的物体
    
    vec3 indirectLight = vec3(0.0);
    int validHits = 0; // 记录有效命中次数，用于计算置信度
    
    // 基础随机数种子
    float seed = rand(v_TexCoord);

    for (int i = 0; i < rays; i++) {
        // 生成均匀分布的 2D 随机数用于采样
        vec2 u = vec2(
            fract(seed + float(i) * 0.618), 
            fract(seed + float(i) * 0.382)
        );
        
        // 余弦权重半球采样，直接获得符合 BRDF 的射线方向
        vec3 rayDir = GetCosineWeightedSample(u, viewNormal);
        
        vec3 rayPos = viewPos;
        bool hit = false;
        vec2 hitUV = vec2(0.0);
        
        // 加入抖动 (Jitter) 以打乱步进起点，减少带状瑕疵
        rayPos += rayDir * (stepSize * fract(seed * 1.333));
        
        for (int j = 0; j < steps; j++) {
            rayPos += rayDir * stepSize;
            
            // 投影到屏幕空间
            vec4 offset = vec4(rayPos, 1.0);
            offset = u_Projection * offset;
            offset.xyz /= offset.w;
            offset.xyz = offset.xyz * 0.5 + 0.5;
            
            // 越界检查
            if (offset.x < 0.0 || offset.x > 1.0 || offset.y < 0.0 || offset.y > 1.0)
                break;
                
            float sampleDepth = texture(GBufferDepth, offset.xy).r;
            vec3 sampleViewPos = ReconstructViewPos(offset.xy, sampleDepth);
            
            float depthDiff = rayPos.z - sampleViewPos.z;
            
            // 深度相交检测
            if (depthDiff > 0.0 && depthDiff < thickness) {
                // 检查法线方向，避免背面透出 (Backface check)
                vec3 sampleNormal = texture(GBufferNormal, offset.xy).rgb * 2.0 - 1.0;
                vec3 sampleViewNormal = normalize(normalMatrix * sampleNormal);
                
                // 如果射线打到了物体的背面，则忽略这次相交
                if (dot(rayDir, sampleViewNormal) < 0.0) {
                    hit = true;
                    hitUV = offset.xy;
                    break;
                }
            }
        }
        
        if (hit) {
            validHits++;
            vec3 hitColor = texture(LightingResult, hitUV).rgb;
            // 降低颜色溢出强度乘数，避免色彩过于饱和突兀
            float colorBleedIntensity = 1.0; 
            
            // 距离衰减：反弹的光应该随着距离衰减，这样墙角的缝隙就不会突兀地亮起一大块
            float distance = length(rayPos - viewPos);
            float attenuation = 1.0 / (1.0 + distance * distance);
            
            // 由于使用了余弦权重采样，不需要再乘以 dot(N, L)，只需除以 PDF (这里 PDF = cos(theta)/PI，互相抵消了部分)
            indirectLight += hitColor * albedo * colorBleedIntensity * attenuation;
        }
    }
    
    // 屏幕边缘衰减因子计算
    // 越靠近屏幕边缘，SSGI越容易获取不到屏幕外信息，因此主动降低置信度
    // 扩大边缘衰减范围，让过渡更平滑、更早发生
    vec2 ndc = v_TexCoord * 2.0 - 1.0;
    float edgeDist = min(1.0 - abs(ndc.x), 1.0 - abs(ndc.y));
    float edgeFactor = smoothstep(0.0, 0.4, edgeDist); // 在边缘 40% 区域内平滑衰减
    
    // 如果有命中，计算平均反弹光照 (纯 SSGI 能量)
    if (validHits > 0) {
        indirectLight /= float(validHits);
    }
    
    // 计算综合置信度 (命中率 * 屏幕边缘衰减)
    // 深度值异常（飞出屏幕/击中背景）导致 validHits 降低，也会自动降低置信度
    // 强化命中率的影响：命中率低于一定阈值时置信度急剧下降
    float hitRatio = float(validHits) / float(rays);
    hitRatio = smoothstep(0.2, 0.8, hitRatio); 
    
    float confidence = hitRatio * edgeFactor;
    
    vec4 envHack = texture(u_EnvMap, vec2(0.0)) * 0.000001;
    vec4 mpHack = texture(GBufferMP, vec2(0.0)) * 0.000001;
    vec4 aoHack = texture(SSAOResult, vec2(0.0)) * 0.000001;
    // 将置信度存储在 Alpha 通道中，供后续的降噪和混合阶段使用
    o_SSGIResult = vec4(indirectLight, confidence) + envHack + mpHack + aoHack;
}

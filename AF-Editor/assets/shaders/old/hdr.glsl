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

// GBuffer 数据 (用于 Probe GI)
uniform sampler2D GBufferNormal;
uniform sampler2D GBufferDepth;
uniform sampler2D GBufferAlbedo;

// 探针数据 SSBO
struct GPUProbe {
    vec4 position_radius; // xyz: position, w: radius
    vec4 sh[9];           // 9 个球谐系数，xyz 存 RGB，w 补齐
};

layout(std430, binding = 2) buffer LightProbes {
    GPUProbe probes[];
};

// Camera UBO
layout(std140, binding = 0) uniform Camera {
    vec3 u_ViewPos;
    mat4 u_View;
    mat4 u_ViewInverse;
    mat4 u_Projection;
    mat4 u_ProjectionInverse;
};

// 曝光参数
uniform float u_Exposure = 1.0;
// 是否启用 SSGI/SSAO
uniform bool u_EnableSSGI = true;
// 是否启用 Probe GI
uniform bool u_EnableProbeGI = true;

// 常量
const float PI = 3.14159265359;

// ---------------------------------------------------------
// 辅助函数
// ---------------------------------------------------------
vec3 ReconstructWorldPos(vec2 texCoord, float depth) {
    vec4 clipPos = vec4(texCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewPos = u_ProjectionInverse * clipPos;
    viewPos /= viewPos.w;
    return (u_ViewInverse * viewPos).xyz;
}

// ---------------------------------------------------------
// 球谐函数 (SH) 求值
// 计算给定法线方向上的辐照度 (Irradiance)
// ---------------------------------------------------------
vec3 EvaluateSH(vec3 normal, GPUProbe probe) {
    // SH 基函数常量
    float c1 = 0.429043;
    float c2 = 0.511664;
    float c3 = 0.743125;
    float c4 = 0.886227;
    float c5 = 0.247708;

    vec3 result = vec3(0.0);
    
    // L0
    result += probe.sh[0].rgb * c4;
    
    // L1
    result += probe.sh[1].rgb * 2.0 * c1 * normal.y;
    result += probe.sh[2].rgb * 2.0 * c1 * normal.z;
    result += probe.sh[3].rgb * 2.0 * c1 * normal.x;
    
    // L2
    result += probe.sh[4].rgb * 2.0 * c2 * normal.x * normal.y;
    result += probe.sh[5].rgb * 2.0 * c2 * normal.y * normal.z;
    result += probe.sh[6].rgb * c3 * (3.0 * normal.z * normal.z - 1.0);
    result += probe.sh[7].rgb * 2.0 * c2 * normal.x * normal.z;
    result += probe.sh[8].rgb * c5 * (normal.x * normal.x - normal.y * normal.y);
    
    // 防止出现负光照
    return max(result, vec3(0.0));
}

// ---------------------------------------------------------
// 查找并插值探针 (模拟 4.2.3 的四面体/空间插值)
// 为了简化，这里使用基于距离的权重混合 (Inverse Distance Weighting)
// ---------------------------------------------------------
vec3 ComputeProbeGI(vec3 worldPos, vec3 normal) {
    if (probes.length() == 0) return vec3(0.0);

    vec3 totalIrradiance = vec3(0.0);
    float totalWeight = 0.0;

    // 遍历所有探针（在实际大型项目中，这必须用 3D 纹理或基于网格的索引来优化）
    for (int i = 0; i < probes.length(); i++) {
        vec3 probePos = probes[i].position_radius.xyz;
        float radius = probes[i].position_radius.w;
        
        float dist = length(worldPos - probePos);
        
        // 如果在影响范围内
        if (dist < radius) {
            // 计算权重 (距离越近权重越大，边缘平滑衰减)
            float weight = smoothstep(radius, 0.0, dist);
            
            // 累加
            totalIrradiance += EvaluateSH(normal, probes[i]) * weight;
            totalWeight += weight;
        }
    }

    if (totalWeight > 0.0) {
        return totalIrradiance / totalWeight;
    }
    
    return vec3(0.0);
}

void main()
{
    // 采样直接光照阶段的结果
    vec3 directLight = texture(LightingResult, v_TexCoord).rgb;
    float depth = texture(GBufferDepth, v_TexCoord).r;
    
    vec3 combinedColor = directLight;

    if (depth < 1.0) {
        // --- 4.2.2 (3) 探针 GI 补偿机制 ---
        
        // 1. 获取几何信息
        vec3 normal = texture(GBufferNormal, v_TexCoord).rgb * 2.0 - 1.0;
        vec3 albedo = texture(GBufferAlbedo, v_TexCoord).rgb;
        vec3 worldPos = ReconstructWorldPos(v_TexCoord, depth);
        
        vec3 finalIndirectLight = vec3(0.0);
        
        // 2. 计算光照探针提供的全局 GI (作为兜底)
        vec3 probeLight = vec3(0.0);
        if (u_EnableProbeGI) {
            vec3 probeIrradiance = ComputeProbeGI(worldPos, normalize(normal));
            // 探针颜色饱和度增强 (Color Saturation Boost)
            // 单纯的亮度放大(Intensity)会导致白色过曝，但如果我们提取出探针的颜色倾向（色相）
            // 并单独放大这部分色彩倾向，就能在不过曝的前提下，让红绿反光变得极其明显。
            float luma = dot(probeIrradiance, vec3(0.299, 0.587, 0.114));
            if (luma > 0.001) {
                // 提取纯色度，并进行一定程度的饱和度放大
                vec3 chroma = probeIrradiance / luma;
                // 用一个指数来拉大颜色差异，让偏红的更红，偏绿的更绿
                chroma = pow(chroma, vec3(1.5)); 
                // 重新乘回亮度，并给出一点点额外的基础放大
                probeIrradiance = chroma * luma * 1.5; 
            }
            
            float probeIntensity = 1.0; // 稍微提升基础亮度，弥补纯物理渲染的不足
            probeLight = probeIrradiance * albedo * probeIntensity / PI; // 漫反射 BRDF
        }
        
        // 3. 读取 SSGI 并进行混合
        if (u_EnableSSGI) {
            // 读取降噪后的 SSGI 和 置信度
            vec4 denoisedData = texture(DenoisedResult, v_TexCoord);
            vec3 ssgiLight = denoisedData.rgb;  // 注意：在降噪 Pass 中已经乘过 AO 了
            float ssgiConfidence = denoisedData.a; // A 通道是 SSGI 置信度

            // 增强过渡的非线性：当置信度较高时，完全使用 SSGI；当置信度下降时，加速向 Probe GI 过渡
            ssgiConfidence = smoothstep(0.1, 0.9, ssgiConfidence);

            // 考虑到 SSGI 可能会因为遮挡或者射线步进精度问题而计算出纯黑 (而此时可能它并没有飞出屏幕，置信度很高)
            // 如果我们强行使用高置信度的纯黑 SSGI，就会覆盖掉探针原本明亮的全局光。
            // 我们现在改为：将 SSGI 作为基于探针基础上的高频细节叠加，或者基于能量互补的平滑过渡
            
            // 使用更温和的能量补充：如果 SSGI 计算出来的能量不如探针，说明该区域在屏幕空间被遮挡但全局空间是亮的
            // 我们给予一定比例的探针光照作为基础环境光，避免纯黑死角。
            // 不再使用极端的 max 导致亮度突变。
            float ssgiLuma = dot(ssgiLight, vec3(0.299, 0.587, 0.114));
            float probeLuma = dot(probeLight, vec3(0.299, 0.587, 0.114));
            
            // 当 SSGI 远暗于 Probe 时，补偿一部分 Probe 的能量
            // 注意：补偿的能量是未经置信度衰减的纯 probeLight，这会导致混合逻辑复杂化。
            // 简单的做法是：我们计算一个最终的“补偿后”的 SSGI，再和探针进行 mix。
            // 或者直接让置信度在“SSGI远暗于探针”时下降，强行使用探针。
            
            // 更稳健的做法：如果探针比SSGI亮，说明SSGI可能没采样到关键光照，我们降低SSGI的权重
            float compensation = clamp((probeLuma - ssgiLuma) / max(probeLuma, 0.001), 0.0, 1.0);
            // 降低置信度：如果探针比它亮很多，confidence 会下降，从而更多地露出探针
            float finalConfidence = ssgiConfidence * (1.0 - compensation * 0.8);
            
            finalIndirectLight = mix(probeLight, ssgiLight, finalConfidence);
        } else {
            // 如果关闭了 SSGI，完全使用探针 GI
            finalIndirectLight = probeLight;
        }

        // 组合直接光照和间接光照
        combinedColor = directLight + finalIndirectLight;
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

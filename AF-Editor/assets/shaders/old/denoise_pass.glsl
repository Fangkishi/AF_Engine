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
layout(location = 0) out vec4 o_DenoisedResult;

uniform sampler2D LightingResult;
uniform sampler2D SSAOResult;
uniform sampler2D SSGIResult;
uniform sampler2D GBufferDepth;
uniform sampler2D GBufferNormal;
uniform sampler2D u_EnvMap;

void main()
{
    vec2 texelSize = 1.0 / textureSize(LightingResult, 0);
    
    float centerDepth = texture(GBufferDepth, v_TexCoord).r;
    if (centerDepth >= 1.0) {
        // 对于背景/天空盒：没有间接光照(RGB=0)，没有环境光遮蔽(A=1，即完全可见)
        o_DenoisedResult = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec3 centerNormal = texture(GBufferNormal, v_TexCoord).rgb * 2.0 - 1.0;
    
    // ---------------------------------------------------------
    // 空域双边滤波 (Bilateral Blur)
    // ---------------------------------------------------------
    float aoSum = 0.0;
    vec4 ssgiSum = vec4(0.0); // 修改为 vec4 以同时累加 rgb(颜色) 和 a(置信度)
    float weightSum = 0.0;
    
    // 扩大滤波核，增加采样跨度，使降噪更平滑
    int blurRadius = 4;       // 增大滤波半径以抹平更多噪点
    float sigmaSpatial = 5.0; // 增大空间方差，让远处样本的权重更大，模糊更柔和
    float sigmaDepth = 0.5;   // 稍微放宽深度容差，以防因为噪点被切割成一块块的
    
    for (int x = -blurRadius; x <= blurRadius; x++) {
        for (int y = -blurRadius; y <= blurRadius; y++) {
            // 使用泊松分布进行带间隔的采样 (跨步采样提升模糊范围)
            vec2 offset = vec2(float(x), float(y)) * texelSize * 2.0; // 增加跨度
            vec2 sampleUV = v_TexCoord + offset;
            
            // 越界保护
            if(sampleUV.x < 0.0 || sampleUV.x > 1.0 || sampleUV.y < 0.0 || sampleUV.y > 1.0) continue;
            
            float sampleDepth = texture(GBufferDepth, sampleUV).r;
            vec3 sampleNormal = texture(GBufferNormal, sampleUV).rgb * 2.0 - 1.0;
            
            // 1. 空间权重 (高斯分布)
            float spatialWeight = exp(-(float(x*x + y*y)) / (2.0 * sigmaSpatial * sigmaSpatial));
            
            // 2. 深度权重 (边缘保留)
            // 考虑到深度是非线性的，最好转换到线性空间比较，但这里为了性能使用了一个非常敏感的系数
            float depthDiff = abs(centerDepth - sampleDepth);
            float depthWeight = exp(-(depthDiff * depthDiff) / (2.0 * sigmaDepth * sigmaDepth));
            
            // 3. 法线权重 (平面保留)
            float normalWeight = pow(max(dot(centerNormal, sampleNormal), 0.0), 64.0);
            
            // 总权重
            float w = spatialWeight * depthWeight * normalWeight;
            
            // 累加
            aoSum += texture(SSAOResult, sampleUV).r * w;
            ssgiSum += texture(SSGIResult, sampleUV) * w; // 累加 vec4 (包含置信度)
            weightSum += w;
        }
    }
    
    float finalAO = weightSum > 0.0 ? aoSum / weightSum : texture(SSAOResult, v_TexCoord).r;
    vec4 finalSSGI = weightSum > 0.0 ? ssgiSum / weightSum : texture(SSGIResult, v_TexCoord); // 获取降噪后的 vec4
    
    // Pass denoised results as a single texture
    // RGB: SSGI indirect lighting
    // A: SSGI Confidence 
    // TODO: 目前只用了两个附件输出，这里 DenoisedResult 暂存 SSGI 的 RGB 和 置信度
    // 但我们需要传递 AO。假设目前合并到了两张图或者通过某些机制。
    // 在您原本代码中，DenoisedResult 是 vec4(finalSSGI, finalAO)。
    // 为了支持探针补偿，我们需要改变数据打包方式，或者在 hdr.glsl 里直接读未降噪的 AO。
    // 权衡之下：我们将 SSGI 的 RGB 和置信度 Confidence 存在 o_DenoisedResult 中。
    // 将 AO 存入一个新的输出，或者将置信度乘以 AO 后再传。
    // 这里为了最少修改拓扑：我们将 o_DenoisedResult 维持 vec4(finalSSGI.rgb, finalAO)，
    // 但我们用 SSGI 的亮度或通过混合逻辑。
    // 实际上，为了完美支持 4.2.3，我们需要在 Denoise 节点输出两个 Target，或者在 HDR pass 里计算 Probe GI。
    
    // 我们暂时这样打包：RGB 是 SSGI，A 是 AO * Confidence (一个 Hack 做法) 
    // 或者我们直接在 DenoisedOutput 里返回 vec4(finalSSGI.rgb, finalSSGI.a)
    // 并在 C++ 端给 HDR Pass 再绑定一个 DenoisedAO ?
    
    // 最佳实践：DenoisedResult (RGBA16F) 存 SSGI(RGB) + Confidence(A)
    vec4 denoisedOutput = vec4(finalSSGI.rgb, finalSSGI.a);
    
    // 并且我们需要把 AO 也传出去。由于目前的拓扑限制，我们暂时将 AO 乘以 SSGI 来应用，
    // 这在物理上也是合理的（AO 用于遮蔽间接光）。
    // 所以：RGB = SSGI * AO，A = Confidence
    denoisedOutput.rgb *= finalAO;

    vec4 envHack = texture(u_EnvMap, vec2(0.0)) * 0.000001;
    o_DenoisedResult = denoisedOutput + envHack;
}

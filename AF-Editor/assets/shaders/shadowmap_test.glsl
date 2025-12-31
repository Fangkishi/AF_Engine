#type vertex
#version 450 core

// 阴影贴图测试着色器 - 顶点阶段
// 用于在屏幕上预览阴影贴图内容 (调试用)

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

// 阴影贴图测试着色器 - 片段阶段
// 将方向光阴影数组或点光源阴影立方体数组可视化
// 在全屏四边形上以 2x3 网格形式展示 6 个面/层

in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

// --- 阴影贴图采样器 ---
uniform sampler2DArray DirShadowMap;      // 方向光阴影图数组
uniform samplerCubeArray PointShadowMap;  // 点光源阴影立方体贴图数组

void main()
{
    // 0: 展示方向光, 1: 展示点光源
    int LightType = 1;
    // 点光源模式下选择查看哪一个点光源的阴影图 (层索引)
    int SelectedPointLightLayer = 0;

    // 假设渲染目标分辨率 (用于计算网格)
    vec2 resolution = vec2(1280.0, 720.0);
    vec2 pixelCoord = v_TexCoord * resolution;
    
    // 计算网格布局: 2 列 x 3 行 (共 6 个单元格)
    int col = int(floor(pixelCoord.x / (resolution.x * 0.5)));
    int row = int(floor(pixelCoord.y / (resolution.y / 3.0)));
    int cellIndex = row * 2 + col;

    // 计算当前单元格内的局部 UV 坐标 [0, 1]
    vec2 cellCoord = vec2(
        fract(pixelCoord.x / (resolution.x * 0.5)),
        fract(pixelCoord.y / (resolution.y / 3.0))
    );

    float shadowValue = 0.0;

    if (LightType == 0) // 方向光模式
    {
        // 直接根据单元格索引采样数组中的不同层
        shadowValue = texture(DirShadowMap, vec3(cellCoord, cellIndex)).r;
    }
    else if (LightType == 1) // 点光源模式
    {
        // 将局部 UV 转换为立方体贴图的 3D 采样向量
        vec2 uv = cellCoord * 2.0 - 1.0; // 映射到 [-1, 1]
        vec3 sampleDir;

        // 根据单元格索引映射到立方体贴图的 6 个面
        // 映射逻辑参考 OpenGL 立方体贴图规范
        switch(cellIndex)
        {
            case 0: // +X (右)
                sampleDir = vec3(1.0, -uv.y, -uv.x);
                break;
            case 1: // -X (左)
                sampleDir = vec3(-1.0, -uv.y, uv.x);
                break;
            case 2: // +Y (上)
                sampleDir = vec3(uv.x, 1.0, uv.y);
                break;
            case 3: // -Y (下)
                sampleDir = vec3(uv.x, -1.0, -uv.y);
                break;
            case 4: // +Z (前)
                sampleDir = vec3(uv.x, -uv.y, 1.0);
                break;
            case 5: // -Z (后)
                sampleDir = vec3(-uv.x, -uv.y, -1.0);
                break;
        }

        // 采样指定的点光源阴影图层
        shadowValue = texture(PointShadowMap, vec4(sampleDir, SelectedPointLightLayer)).r;
    }

    // 可视化深度值 (通常深度值在 0-1 之间)
    o_Color = vec4(vec3(shadowValue), 1.0);
}
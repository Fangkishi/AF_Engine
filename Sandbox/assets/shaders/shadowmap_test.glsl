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
// 将最终颜色、方向光阴影图、点光源阴影立方体贴图可视化
// 在全屏四边形上以 4x2 网格形式展示

in vec2 v_TexCoord;

layout(location = 0) out vec4 o_Color;

// --- 采样器 ---
uniform sampler2D FinalColor;               // 最终颜色缓冲
uniform sampler2DArray DirShadowMap;      // 方向光阴影图数组
uniform samplerCubeArray PointShadowMap;  // 点光源阴影立方体贴图数组

void main()
{
    // 计算网格布局: 4 列 x 2 行
    int col = int(floor(v_TexCoord.x * 4.0));
    int row = int(floor(v_TexCoord.y * 2.0));
    
    // 我们希望 index 0 在左上角
    // row=1 是上方, row=0 是下方
    int cellIndex = (1 - row) * 4 + col;

    // 计算当前单元格内的局部 UV 坐标 [0, 1]
    vec2 cellCoord = vec2(fract(v_TexCoord.x * 4.0), fract(v_TexCoord.y * 2.0));

    vec3 color = vec3(0.0);

    if (cellIndex == 0) 
    {
        // 第一个显示 FinalColor
        color = texture(FinalColor, cellCoord).rgb;
    }
    else if (cellIndex == 1)
    {
        // 第二个显示第一个平行光的阴影图
        float shadow = texture(DirShadowMap, vec3(cellCoord, 0)).r;
        color = vec3(shadow);
    }
    else if (cellIndex >= 2 && cellIndex <= 7)
    {
        // 剩下六个显示第一个点光源的阴影图的前后左右上下
        // cellIndex: 2=前, 3=后, 4=左, 5=右, 6=上, 7=下
        
        vec2 uv = cellCoord * 2.0 - 1.0; // 映射到 [-1, 1]
        vec3 sampleDir;

        int faceIndex = cellIndex - 2;
        switch(faceIndex)
        {
            case 0: // 前 (+Z)
                sampleDir = vec3(uv.x, -uv.y, 1.0);
                break;
            case 1: // 后 (-Z)
                sampleDir = vec3(-uv.x, -uv.y, -1.0);
                break;
            case 2: // 左 (-X)
                sampleDir = vec3(-1.0, -uv.y, uv.x);
                break;
            case 3: // 右 (+X)
                sampleDir = vec3(1.0, -uv.y, -uv.x);
                break;
            case 4: // 上 (+Y)
                sampleDir = vec3(uv.x, 1.0, uv.y);
                break;
            case 5: // 下 (-Y)
                sampleDir = vec3(uv.x, -1.0, -uv.y);
                break;
        }
        
        // 采样第一个点光源 (层索引 0)
        float shadowValue = texture(PointShadowMap, vec4(sampleDir, 0)).r;
        color = vec3(shadowValue);
    }

    // 输出最终颜色
    o_Color = vec4(color, 1.0);
}

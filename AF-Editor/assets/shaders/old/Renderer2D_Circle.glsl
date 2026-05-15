// 2D 圆形渲染着色器
// 使用 SDF (符号距离场) 技术在矩形 Quad 上绘制完美的圆

#type vertex
#version 450 core

// 顶点属性输入
layout(location = 0) in vec3 a_WorldPosition; // 世界空间位置
layout(location = 1) in vec3 a_LocalPosition; // 局部空间位置 (用于计算到中心的距离)
layout(location = 2) in vec4 a_Color;         // 颜色
layout(location = 3) in float a_Thickness;    // 厚度 (0.0 到 1.0)
layout(location = 4) in float a_Fade;         // 边缘平滑/渐变系数
layout(location = 5) in int a_EntityID;       // 实体 ID

// 相机 Uniform Block
layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

// 传递给片段着色器的数据
struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
};

layout (location = 0) out VertexOutput Output;
layout (location = 4) out flat int v_EntityID;

void main()
{
	Output.LocalPosition = a_LocalPosition;
	Output.Color = a_Color;
	Output.Thickness = a_Thickness;
	Output.Fade = a_Fade;

	v_EntityID = a_EntityID;

	// 计算裁剪空间坐标
	gl_Position = u_ViewProjection * vec4(a_WorldPosition, 1.0);
}

#type fragment
#version 450 core

// 输出颜色和实体 ID
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec3 LocalPosition;
	vec4 Color;
	float Thickness;
	float Fade;
};

layout (location = 0) in VertexOutput Input;
layout (location = 4) in flat int v_EntityID;

void main()
{
    // 计算到中心的距离并计算圆形的 alpha 遮罩
    // LocalPosition 范围通常是 [-1, 1]
    float distance = 1.0 - length(Input.LocalPosition);
    
    // 使用 smoothstep 进行边缘平滑处理
    float circle = smoothstep(0.0, Input.Fade, distance);
    // 处理圆环厚度
    circle *= smoothstep(Input.Thickness + Input.Fade, Input.Thickness, distance);

    // 如果不在圆内则丢弃像素
	if (circle == 0.0)
		discard;

    // 设置最终输出颜色
    o_Color = Input.Color;
	o_Color.a *= circle;

	o_EntityID = v_EntityID;
}

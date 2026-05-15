// 2D 线条渲染着色器
// 用于绘制简单的 2D 直线

#type vertex
#version 450 core

// 顶点属性输入
layout(location = 0) in vec3 a_Position; // 线条顶点位置
layout(location = 1) in vec4 a_Color;    // 线条颜色
layout(location = 2) in int a_EntityID;  // 实体 ID

// 相机 Uniform Block
layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

// 传递给片段着色器的数据
struct VertexOutput
{
	vec4 Color;
};

layout (location = 0) out VertexOutput Output;
layout (location = 1) out flat int v_EntityID;

void main()
{
	Output.Color = a_Color;
	v_EntityID = a_EntityID;

	// 计算裁剪空间坐标
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

// 输出颜色和实体 ID
layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

struct VertexOutput
{
	vec4 Color;
};

layout (location = 0) in VertexOutput Input;
layout (location = 1) in flat int v_EntityID;

void main()
{
	// 直接输出线条颜色
	o_Color = Input.Color;
	o_EntityID = v_EntityID;
}

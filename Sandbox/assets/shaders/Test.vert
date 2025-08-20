#version 330 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_Uv;

uniform mat4 u_ViewProjection;

out vec2 v_Uv;

void main()
{
	v_Uv = a_Uv;
	gl_Position = u_ViewProjection * vec4(a_Position, 1.0);
}
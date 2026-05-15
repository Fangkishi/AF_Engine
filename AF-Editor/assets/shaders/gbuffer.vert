#version 450 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;
out vec3 v_Color;
out vec3 v_WorldPos;
layout(std140, binding = 0) uniform FrameData {
    mat4 ViewProjection;
    mat4 InverseViewProjection;
    mat4 Projection;
    vec3 CameraPosition;
    vec2 ScreenSize;
};
uniform mat4 u_Model;
void main()
{
    vec4 worldPos = u_Model * vec4(a_Position, 1.0);
    v_WorldPos = worldPos.xyz;
    v_Color = a_Color;
    gl_Position = ViewProjection * worldPos;
}

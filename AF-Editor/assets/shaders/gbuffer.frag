#version 450 core
in vec3 v_Color;
in vec3 v_WorldPos;
layout(location = 0) out vec4 o_Albedo;
layout(location = 1) out vec4 o_Normal;
layout(location = 2) out vec4 o_Material;
void main()
{
    vec3 normal = normalize(cross(dFdx(v_WorldPos), dFdy(v_WorldPos)));
    o_Albedo   = vec4(v_Color, 1.0);
    o_Normal   = vec4(normal * 0.5 + 0.5, 0.0);
    o_Material = vec4(0.5, 0.0, 1.0, 0.0);
}

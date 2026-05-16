#version 450 core
in vec4 v_FragColor;
in vec3 v_WorldPos;
in vec3 v_WorldNormal;

layout(location = 0) out vec4 o_Albedo;
layout(location = 1) out vec4 o_Normal;
layout(location = 2) out vec4 o_Material;

void main()
{
    o_Albedo   = v_FragColor;
    o_Normal   = vec4(normalize(v_WorldNormal) * 0.5 + 0.5, 0.0);
    o_Material = vec4(0.5, 0.0, 1.0, 0.0);
}

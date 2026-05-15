#version 450 core
in vec2 v_TexCoord;
out vec4 o_Color;
uniform sampler2D u_Input;
void main()
{
    vec3 hdr = texture(u_Input, v_TexCoord).rgb;
    vec3 mapped = hdr / (hdr + vec3(1.0));
    mapped = pow(mapped, vec3(1.0 / 2.2));
    o_Color = vec4(mapped, 1.0);
}

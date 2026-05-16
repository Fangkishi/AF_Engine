#version 450 core
in vec2 v_TexCoord;
out vec4 o_Color;
uniform sampler2D u_Input;
void main()
{
    o_Color = texture(u_Input, v_TexCoord);
}

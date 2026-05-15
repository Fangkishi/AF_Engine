#version 450 core
in vec2 v_TexCoord;
out vec4 o_Color;
uniform sampler2D u_Albedo;
uniform sampler2D u_Normal;
uniform sampler2D u_Material;
uniform sampler2D u_Depth;
uniform vec3 u_LightDir;
uniform vec3 u_LightColor;
uniform float u_LightIntensity;
layout(std140, binding = 0) uniform FrameData {
    mat4 ViewProjection;
    mat4 InverseViewProjection;
    mat4 Projection;
    vec3 CameraPosition;
    vec2 ScreenSize;
};
void main()
{
    vec3 albedo     = texture(u_Albedo, v_TexCoord).rgb;
    vec3 normal     = normalize(texture(u_Normal, v_TexCoord).rgb * 2.0 - 1.0);
    float roughness = texture(u_Material, v_TexCoord).r;
    float metallic  = texture(u_Material, v_TexCoord).g;
    float ao        = texture(u_Material, v_TexCoord).b;
    float depth     = texture(u_Depth, v_TexCoord).r;

    // Reconstruct world position from depth + UV + inverse VP
    vec4 clipPos = vec4(v_TexCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 wp = InverseViewProjection * clipPos;
    vec3 worldPos = wp.xyz / wp.w;

    vec3 N = normalize(normal);
    vec3 L = normalize(-u_LightDir);
    vec3 V = normalize(CameraPosition - worldPos);
    vec3 H = normalize(L + V);

    float NdotL = max(dot(N, L), 0.0);
    float NdotH = max(dot(N, H), 0.0);

    vec3 ambient  = albedo * 0.03 * ao;
    vec3 diffuse  = albedo * NdotL;
    float spec    = pow(NdotH, (1.0 - roughness) * 256.0 + 1.0) * (1.0 - metallic) * 0.5;

    vec3 lighting = ambient + (diffuse + spec) * u_LightColor * u_LightIntensity * ao;
    o_Color = vec4(lighting, 1.0);
}

#version 450
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec2 v_uv;
layout(location = 1) flat in uint texId;

layout(set = 0, binding = 3) uniform sampler2D textureSamplers[];

vec3 adjustContrastLuminance(vec3 color, float contrast) {
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    vec3 luminanceColor = vec3(luminance);
    return mix(luminanceColor, color, contrast);
}

void main()
{
    FragColor = texture(textureSamplers[texId], v_uv);
    vec3 color = adjustContrastLuminance(FragColor.xyz, 0.8);
    FragColor = vec4(color, FragColor.w);
}
#version 450

layout(location = 0) in vec4 colorOut;
layout(location = 1) in float contrast;

layout(location = 0) out vec4 outColor;

vec4 adjustContrast(vec4 color, float contrast) {
    // Convert to grayscale using luminance weights
    float luminance = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    
    // Interpolate between grayscale and original color based on contrast
    vec3 adjustedColor = mix(vec3(luminance), color.rgb, contrast);
    
    return vec4(adjustedColor, color.a);
}

void main() {
    outColor = adjustContrast(colorOut, contrast);
}
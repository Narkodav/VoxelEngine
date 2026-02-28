#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) in vec4 vertex;

//instanced
layout(location = 1) in vec4 transformLocalX;
layout(location = 2) in vec4 transformLocalY;
layout(location = 3) in vec4 transformLocalZ;
layout(location = 4) in vec4 transformLocalW;

layout(location = 2) in vec4 color;

layout(location = 0) out vec4 colorOut;
layout(location = 1) out float contrast;

layout(push_constant) uniform PushConstants {
    mat4 view;                    // 16-byte aligned, 64 bytes (4x4 matrix)
    mat4 proj;                    // 16-byte aligned, 64 bytes (4x4 matrix)
    mat4 transformGlobal;
    float contrast;
} pushConstants;

void main() {
    mat4 transformLocal = mat4(
        transformLocalX,
        transformLocalY,
        transformLocalZ,
        transformLocalW
    );

    gl_Position = pushConstants.proj * pushConstants.view * pushConstants.transformGlobal * transformLocal * vertex;
    colorOut = color;
    contrast = pushConstants.contrast;
}
#version 450
//!#extension GL_KHR_vulkan_glsl : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location = 0) in vec2 UV;
layout(location = 1) flat in uint textureId;

//in voxels
const uint chunkWidth = 16;	//along X axis
const uint chunkHeight = 16;	//along Y axis
const uint chunkDepth = 16;	//along Z axis
const uvec3 chunkSizeVector = { chunkWidth, chunkHeight, chunkDepth };

const float gridCellSize = 1.0f;
const float gridCellCenterOffset = 0.5f;

const uint chunkSize = chunkWidth * chunkHeight * chunkDepth;
const uint chunkLayerSize = chunkWidth * chunkDepth;

const uvec3 chunkMin = { 0, 0, 0 };
const uvec3 chunkMax = { chunkWidth - 1, chunkHeight - 1, chunkDepth - 1 };

layout(push_constant) uniform PushConstants {
    mat4 view;                    // 16-byte aligned, 64 bytes (4x4 matrix)
    mat4 proj;                    // 16-byte aligned, 64 bytes (4x4 matrix)
    uint chunkCount;
} pushConstants;

struct Polygon                    // Total: 12 bytes
{
    uint positions[3];            // 4-byte aligned, 12 bytes total
    uint normal;
};

struct Coloring                   // Total: 16 bytes
{
    uint uvs[3];                  // 4-byte aligned, 12 bytes
    uint textureId;               // 4-byte aligned, 4 bytes
};                                // Struct gets 16-byte alignment

struct RangeStart                 // Total: 8 bytes
{
    uint polygon;                 // 4-byte aligned, 4 bytes
    uint coloring;                // 4-byte aligned, 4 bytes
};

struct Chunk                       // Total: 48 bytes
{
    ivec4 coord;                   // 16 bytes
    uint start;                    // 4 bytes
    uint neighbourStarts[6];       // 24 bytes  
    uint padding;                  // 4 bytes
};

struct Indices {
    uint polygon;
    uint coloring;
    uint block;
};

struct DrawCommand {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;

    uint bufferId;
};

layout(set = 0, binding = 0, std430) readonly buffer Vertices {
    vec4 vertices[];              // 16-byte aligned, 16-byte stride per element
};

layout(set = 0, binding = 1, std430) readonly buffer Uvs {
    vec2 uvs[];                   // 8-byte aligned, 8-byte stride per element
};

layout(set = 0, binding = 2, std430) readonly buffer Normals {
    vec4 normals[];               // 16-byte aligned, 8-byte stride per element
};

layout(set = 0, binding = 3, std430) readonly buffer PolygonCache {
    Polygon polygons[];           // 16-byte stride per element (12 bytes + 4 padding)
};

layout(set = 0, binding = 4, std430) readonly buffer ColoringCache {
    Coloring colorings[];         // 16-byte stride per element
};

layout(set = 0, binding = 5, std430) readonly buffer PolygonIndexCache {
    uint polygonIndices[];        // 4-byte stride per element
};

layout(set = 0, binding = 6, std430) readonly buffer ColoringIndexCache {
    uint coloringIndices[];       // 4-byte stride per element
};

layout(set = 0, binding = 11) uniform sampler2D textures[];

//the pool
layout(set = 1, binding = 0, std430) readonly buffer ChunkData {
    uint stateIds[];           // 4-byte stride per element
};

layout(set = 1, binding = 1, std430) readonly buffer Chunks {
    Chunk chunks[];    // 48-byte stride per element
};

layout(set = 1, binding = 2, std430) readonly buffer DrawCommands {
    DrawCommand drawCommands[];
};

layout(set = 2, binding = 0, std430) readonly buffer VertexBuffers {
    Indices indices[];
} vertexBuffers[];

layout(set = 3, binding = 0) uniform Config {
    float contrast;         // 0.0 = grayscale, 1.0 = normal, >1.0 = increased contrast
} config;

layout(location = 0) out vec4 outColor;

vec4 adjustContrast(vec4 color, float contrast) {
    // Convert to grayscale using luminance weights
    float luminance = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    
    // Interpolate between grayscale and original color based on contrast
    vec3 adjustedColor = mix(vec3(luminance), color.rgb, contrast);
    
    return vec4(adjustedColor, color.a);
}

void main() {
    vec4 texColor = texture(textures[textureId], UV);
    outColor = adjustContrast(texColor, config.contrast);
}
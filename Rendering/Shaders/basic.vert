#version 460
#extension GL_KHR_vulkan_glsl : enable

layout(location = 0) out vec2 UV;
layout(location = 1) flat out uint textureId;

//in voxels
const uint chunkWidth = 32;	//along X axis
const uint chunkHeight = 32;	//along Y axis
const uint chunkDepth = 32;	//along Z axis
const uvec3 chunkSizeVector = { chunkWidth, chunkHeight, chunkDepth };

const float gridCellSize = 1.0f;
const float gridCellCenterOffset = 0.5f;

const uint chunkSize = chunkWidth * chunkHeight * chunkDepth;
const uint chunkLayerSize = chunkWidth * chunkDepth;

const uvec3 chunkMin = { 0, 0, 0 };
const uvec3 chunkMax = { chunkWidth - 1, chunkHeight - 1, chunkDepth - 1 };

//per frame data - std140 layout (uniform buffer)
layout(set = 0, binding = 0) uniform UniformPerFrame {
    mat4 view;                    // 16-byte aligned, 64 bytes (4x4 matrix)
    mat4 proj;                    // 16-byte aligned, 64 bytes (4x4 matrix)
} perFrameUniforms;               // Total: 128 bytes

struct Polygon                    // Total: 16 bytes
{
    uint positions[3];            // 4-byte aligned, 12 bytes total
    uint padding;                 // 4-byte aligned, 5 bytes total
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

// Storage buffers - std430 layout
layout(set = 1, binding = 0, std430) readonly buffer Vertices {
    vec4 vertices[];              // 16-byte aligned, 16-byte stride per element
};

layout(set = 1, binding = 1, std430) readonly buffer Uvs {
    vec2 uvs[];                   // 8-byte aligned, 8-byte stride per element
};

layout(set = 1, binding = 2, std430) readonly buffer PolygonCache {
    Polygon polygons[];           // 16-byte stride per element (12 bytes + 4 padding)
};

layout(set = 1, binding = 3, std430) readonly buffer ColoringCache {
    Coloring colorings[];         // 16-byte stride per element
};

layout(set = 1, binding = 4, std430) readonly buffer PolygonIndexCache {
    uint polygonIndices[];        // 4-byte stride per element
};

layout(set = 1, binding = 5, std430) readonly buffer ColoringIndexCache {
    uint coloringIndices[];       // 4-byte stride per element
};

layout(set = 2, binding = 1, std430) readonly buffer RangeStartBuffer {
    RangeStart rangeStarts[chunkSize]; // 8-byte stride per element
};

//per chunk data - std140 layout (uniform buffer)
layout(set = 2, binding = 3) uniform ChunkPos {
    vec4 chunkPos;                // 16-byte aligned, 16 bytes
};

vec3 getBlockPosition(uint index) {
    uint z = (index % chunkLayerSize) / chunkWidth;
    uint y = index / chunkLayerSize;
    uint x = index % chunkWidth;
    return vec3(x, y, z);
}

vec4 getBlockGlobalPosition(uint index, vec3 chunkPos)
{
    vec3 pos = getBlockPosition(index);
    return vec4(chunkPos * chunkSizeVector + pos, 0); 
}

vec4 getPosition(uint polygonId, uint positionId)
{
    vec4 vertexPos = vertices[polygons[polygonId].positions[positionId]];
    vec4 blockPos = getBlockGlobalPosition(gl_DrawID, chunkPos.xyz);
    return vec4(vertexPos.xyz + blockPos.xyz, 1.0);
}

void main() {
    uint polygonIndex = polygonIndices[rangeStarts[gl_DrawID].polygon + gl_InstanceIndex];
    uint coloringIndex = coloringIndices[rangeStarts[gl_DrawID].coloring + gl_InstanceIndex];

    vec4 finalPos = getPosition(polygonIndex, gl_VertexIndex);
    gl_Position = perFrameUniforms.proj * perFrameUniforms.view * finalPos;

    Coloring coloring = colorings[coloringIndex];
    UV = uvs[coloring.uvs[gl_VertexIndex]];
    textureId = coloring.textureId;

//    // Bounds checking
//    if (gl_DrawID >= 4096) {
//        gl_Position = vec4(0, 0, 0, 1);
//        UV = vec2(0, 0);
//        textureId = 0;
//        return;
//    }
//    
//    RangeStart range = rangeStarts[gl_DrawID];
//    uint polygonIdx = range.polygon + gl_InstanceIndex;
//    uint coloringIdx = range.coloring + gl_InstanceIndex;
//    
//    // More bounds checking
//    if (polygonIdx >= polygonIndices.length() || coloringIdx >= coloringIndices.length()) {
//        gl_Position = vec4(0, 0, 0, 1);
//        UV = vec2(0, 0);
//        textureId = 0;
//        return;
//    }
//    
//    uint polygonIndex = polygonIndices[polygonIdx];
//    uint coloringIndex = coloringIndices[coloringIdx];
//    
//    if (polygonIndex >= polygons.length() || coloringIndex >= colorings.length()) {
//        gl_Position = vec4(0, 0, 0, 1);
//        UV = vec2(0, 0);
//        textureId = 0;
//        return;
//    }
//    
//    vec4 finalPos = getPosition(polygonIndex, gl_VertexIndex);
//    gl_Position = perFrameUniforms.proj * perFrameUniforms.view * finalPos;
//    
//    Coloring coloring = colorings[coloringIndex];
//    if (coloring.uvs[gl_VertexIndex] >= uvs.length()) {
//        UV = vec2(0, 0);
//    } else {
//        UV = uvs[coloring.uvs[gl_VertexIndex]];
//    }
//    textureId = coloring.textureId;

//        if (gl_DrawID >= 10) {
//            gl_Position = vec4(0, 0, 0, 1);
//            UV = vec2(0, 0);
//            textureId = 0;
//            return;
//        }
//
//      if(gl_VertexIndex == 0)
//      {
//        gl_Position = perFrameUniforms.proj * perFrameUniforms.view * vec4(0, 0, gl_DrawID * 2, 1);
////        UV = vec2(0, 0);
//      }
//      else if (gl_VertexIndex == 1)
//      {
//        gl_Position = perFrameUniforms.proj * perFrameUniforms.view * vec4(1, 0, gl_DrawID * 2, 1);
////        UV = vec2(1, 0);
//      }
//      else
//      {
//        gl_Position = perFrameUniforms.proj * perFrameUniforms.view * vec4(0, 1, gl_DrawID * 2, 1);
////        UV = vec2(0, 1);
//      }      
//      textureId = 0;
//
//      uint coloringIndex = coloringIndices[gl_VertexIndex];
//      Coloring coloring = colorings[coloringIndex];
//      UV = uvs[coloring.uvs[gl_VertexIndex]];
//      textureId = coloring.textureId;
}
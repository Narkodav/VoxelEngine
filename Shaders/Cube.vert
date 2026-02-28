#version 460
#extension GL_KHR_vulkan_glsl : enable
#extension GL_ARB_shader_draw_parameters : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec2 UV;
layout(location = 1) flat out uint textureId;

// enum class Directions3D : size_t
// {
// 	FORWARD,
// 	RIGHT,
// 	UP,
// 	BACKWARD,
// 	LEFT,
// 	DOWN,
// 	NUM,
// };

// const std::array<Direction3D, enumCast(Directions3D::NUM)> directions3D = {
// 	Direction3D{0, 0, -1}, Direction3D{1, 0, 0}, Direction3D{0, 1, 0},
// 	Direction3D{0, 0, 1}, Direction3D{-1, 0, 0}, Direction3D{0, -1, 0},
// };

const vec3 directions3D[6] = vec3[6](
    vec3( 0, 0,-1),
    vec3( 1, 0, 0),
    vec3( 0, 1, 0),
    vec3( 0, 0, 1),
    vec3(-1, 0, 0),
    vec3( 0,-1, 0)
);

//for cube faces
const vec4 cubeVertices[6][6] = vec4[6][6](

    // 0 — FORWARD ( -Z )
    vec4[6](
        vec4(-0.5, -0.5, -0.5, 1.0),
        vec4( 0.5, -0.5, -0.5, 1.0),
        vec4( 0.5,  0.5, -0.5, 1.0),

        vec4(-0.5, -0.5, -0.5, 1.0),
        vec4( 0.5,  0.5, -0.5, 1.0),
        vec4(-0.5,  0.5, -0.5, 1.0)
    ),

    // 1 — RIGHT ( +X )
    vec4[6](
        vec4( 0.5, -0.5, -0.5, 1.0),
        vec4( 0.5, -0.5,  0.5, 1.0),
        vec4( 0.5,  0.5,  0.5, 1.0),

        vec4( 0.5, -0.5, -0.5, 1.0),
        vec4( 0.5,  0.5,  0.5, 1.0),
        vec4( 0.5,  0.5, -0.5, 1.0)
    ),

    // 2 — UP ( +Y )
    vec4[6](
        vec4(-0.5,  0.5, -0.5, 1.0),
        vec4( 0.5,  0.5, -0.5, 1.0),
        vec4( 0.5,  0.5,  0.5, 1.0),

        vec4(-0.5,  0.5, -0.5, 1.0),
        vec4( 0.5,  0.5,  0.5, 1.0),
        vec4(-0.5,  0.5,  0.5, 1.0)
    ),

    // 3 — BACKWARD ( +Z )
    vec4[6](
        vec4( 0.5, -0.5,  0.5, 1.0),
        vec4(-0.5, -0.5,  0.5, 1.0),
        vec4(-0.5,  0.5,  0.5, 1.0),

        vec4( 0.5, -0.5,  0.5, 1.0),
        vec4(-0.5,  0.5,  0.5, 1.0),
        vec4( 0.5,  0.5,  0.5, 1.0)
    ),

    // 4 — LEFT ( -X )
    vec4[6](
        vec4(-0.5, -0.5,  0.5, 1.0),
        vec4(-0.5, -0.5, -0.5, 1.0),
        vec4(-0.5,  0.5, -0.5, 1.0),

        vec4(-0.5, -0.5,  0.5, 1.0),
        vec4(-0.5,  0.5, -0.5, 1.0),
        vec4(-0.5,  0.5,  0.5, 1.0)
    ),

    // 5 — DOWN ( -Y )
    vec4[6](
        vec4(-0.5, -0.5,  0.5, 1.0),
        vec4( 0.5, -0.5,  0.5, 1.0),
        vec4( 0.5, -0.5, -0.5, 1.0),

        vec4(-0.5, -0.5,  0.5, 1.0),
        vec4( 0.5, -0.5, -0.5, 1.0),
        vec4(-0.5, -0.5, -0.5, 1.0)
    )
);

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
    mat4 viewProj;
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

struct Chunk                       // Total: 44 bytes
{
    ivec4 coord;                   // 16 bytes
    ivec4 coordCorner;             // 16 bytes
    uint start;                    // 4 bytes
    uint neighbourStarts[6];       // 24 bytes
    uint padding;
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

struct Indices {
    uint side;
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

vec3 getBlockPosition(uint index) {
    uint z = (index % chunkLayerSize) / chunkWidth;
    uint y = index / chunkLayerSize;
    uint x = index % chunkWidth;
    return vec3(x, y, z);
}

vec3 getBlockGlobalPosition(uint index, vec3 chunkCoordCorner)
{
    vec3 pos = getBlockPosition(index);
    return chunkCoordCorner + pos; 
}

vec4 getPosition(uint side, uint blockId)
{
    uint localBlockIndex = blockId % chunkSize;
    uint chunkIndex = blockId / chunkSize;
    vec4 vertexPos = cubeVertices[side][gl_VertexIndex + (gl_InstanceIndex & 1) * 3];
    vec3 blockPos = getBlockGlobalPosition(localBlockIndex, chunks[chunkIndex].coordCorner.xyz);
    return vec4(vertexPos.xyz + blockPos, 1.0);
}

void main() {
    DrawCommand command = drawCommands[gl_DrawID];
    Indices indices = vertexBuffers[command.bufferId].indices[gl_InstanceIndex];
    
    //Always even amount of instances for cubes since each face is 2 instances
    vec4 finalPos = getPosition(indices.side, indices.block);
    gl_Position = pushConstants.viewProj * finalPos;
    
    Coloring coloringData = colorings[indices.coloring];
    UV = uvs[coloringData.uvs[gl_VertexIndex]];
    textureId = coloringData.textureId;
}
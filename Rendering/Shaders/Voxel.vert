#version 450

layout(location = 0) in vec4 v_position;

//per instance
layout(location = 1) in vec3 v_gridPosition; //position relative to chunk
layout(location = 2) in uint parallelogramId;
layout(location = 3) in uint side;

layout(location = 0) out vec2 v_uv;
layout(location = 1) flat out uint texId;

struct BufferStruct
{
	vec4 dimensions;
	vec4 rotations;
	vec4 position;		//relative to the grid cell
	uint texturesIds[6];
	vec2 uvs[6][4];
};

layout(set = 0, binding = 0, std430) buffer ShapeData
{
    BufferStruct data[];
};

// Uniforms per frame
layout(set = 0, binding = 1) uniform FrameData {
    mat4 u_viewTransform;
    mat4 u_projectionTransform;
    uvec3 u_chunkSizes;
    mat4 u_sideRotations[6];
    vec4 u_cornerPositions[4];
};

// Uniforms per chunk
layout(set = 0, binding = 2) uniform ChunkData {
    vec3 u_chunkPosition;
};

vec2 getUV(vec4 position, uint parallelogramId, uint side)
{
    if(position == u_cornerPositions[0])
        return data[parallelogramId].uvs[side][0];
    else if(position == u_cornerPositions[1])
        return data[parallelogramId].uvs[side][1];
    else if(position == u_cornerPositions[2])
        return data[parallelogramId].uvs[side][2];
    else
        return data[parallelogramId].uvs[side][3];
}

void main()
{
    v_uv = getUV(v_position, parallelogramId, side);

    vec4 rotatedFace = u_sideRotations[side] * v_position;
    vec4 finalFace = vec4(rotatedFace.x * data[parallelogramId].dimensions.x + data[parallelogramId].position.x 
        + v_gridPosition.x + u_chunkPosition.x * u_chunkSizes.x,
    rotatedFace.y * data[parallelogramId].dimensions.y + data[parallelogramId].position.y 
        + v_gridPosition.y + u_chunkPosition.y * u_chunkSizes.y,
    rotatedFace.z * data[parallelogramId].dimensions.z + data[parallelogramId].position.z 
        + v_gridPosition.z + u_chunkPosition.z * u_chunkSizes.z, 1);

    gl_Position = u_projectionTransform * u_viewTransform * finalFace;
    texId = data[parallelogramId].texturesIds[side];
}
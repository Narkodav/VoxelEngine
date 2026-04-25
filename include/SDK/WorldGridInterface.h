#pragma once
#include "Common.h"
#include "SDK/ResourceCacheInterface.h"

#include "Modular/Interfaces.h"

extern "C" {
    typedef size_t ChunkIndex;

    struct WorldGridInterface {
        glm::ivec4 (*getChunkCoord)(void* self, ChunkIndex chunk);
        glm::ivec4 (*getChunkCornerCoord)(void* self, ChunkIndex chunk);
        VoxelStateId* (*getChunkBlocks)(void* self, ChunkIndex chunk);
        VoxelStateId* (*getAdjacentBlocks)(void* self, ChunkIndex chunk, size_t sideIndex);
        void* self;
    };
}
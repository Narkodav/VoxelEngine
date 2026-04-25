#pragma once
#include "Common.h"
#include "Modular/Interfaces.h"

extern "C" {
    typedef uint32_t VoxelId;
    typedef uint32_t VoxelStateId;

    struct ResourceCacheInterface {
        VoxelId (*getVoxelId)(void* self, const char* name);
        VoxelStateId (*getVoxelStateId)(void* self, const char* name);
        void* self;
    };
}
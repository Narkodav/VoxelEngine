#pragma once
#include "Common.h"
#include "SDK/WorldGridInterface.h"

#include "Modular/Interfaces.h"

extern "C" {
    typedef uint64_t GeneratorSeed;

    struct GeneratorInterface {
        void (*setChunkData)(void* self, WorldGridInterface* grid, ChunkIndex chunk);
        void (*init)(void* self, GeneratorSeed seed, ResourceCacheInterface* resources);
        void* self;
    };

    struct GeneratorApiInterface {
        GeneratorInterface* (*createGenerator)(void* self);
        void (*destroyGenerator)(void* self, GeneratorInterface* generator);
        void* self;
    };
}
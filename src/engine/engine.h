#pragma once

#include "../scene.h"
#include "../config.h"
#include "engine_interface.h" // IEngine
#include "metal/metal_engine.h"

namespace minity
{

enum backend {kSoftware, kMetal, kNull};

// construct a concrete engine of type engineBackend
IEngine *getEngine(backend engineBackend)
{

    IEngine *engine{nullptr};
    switch (engineBackend)
    {
    case kMetal:
        // engine = metalEngine();
        engine = new metalEngine();
        break;
    case kSoftware:
        engine = nullptr;
        break;
    case kNull:
        engine = nullptr;
        break;
    default:
        throw std::runtime_error("Unknown rendering backend");
        break;
    }
    return engine;
};

} // NS minity

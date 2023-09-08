#pragma once

#include "../scene.h"

namespace minity
{

/**
 * Main minity engine interface (will have software and metal implementations)
 * Will take a scene and run it
*/
class IEngine
{
public:
    IEngine() {};
    virtual ~IEngine() {};
    virtual void run(scene scene) = 0;
    virtual void shutdown() = 0;
};

} // NS minity

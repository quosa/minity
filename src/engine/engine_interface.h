#pragma once

#include "../scene.h"
#include "../freezer/old_scene.h" // TODO: refactor away

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
    virtual void run(minity::old_scene scene) { (void)scene; }; // TODO: refactor to new scene type
    virtual void shutdown() = 0;
};

} // NS minity

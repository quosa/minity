#pragma once

struct config
{
    bool drawNormals = false; // n key
    bool drawWireframe = false; // l key
    bool drawPointCloud = false; // p key
    bool drawAxes = false; // x key
    bool fillTriangles = true; // f key
    bool showStatsWindow = false; // F1 key
    bool autoRotate = false; // r key
};
config *g_config = new config();

#pragma once

#include <iostream>

struct renderStats
{
    unsigned long int vertices{0};
    unsigned long int faces{0};
    unsigned long int bfCulled{0};
    unsigned long int vfCulled{0};
    unsigned long int drawnFaces{0};
    friend std::ostream& operator<<(std::ostream& os, const renderStats &stats);
};

// for std::cout << myRenderStats;
std::ostream& operator<<( std::ostream &os, const renderStats &stats )
{
    os << "render statistics:" << std::endl
       << "  vertices     " << stats.vertices << std::endl
       << "  faces        " << stats.faces << std::endl
       << "  bf-culled    " << stats.bfCulled << std::endl
       << "  vf-culled    " << stats.vfCulled << std::endl
       << "  drawn faces  " << stats.drawnFaces << std::endl;
    return os;
}

struct rasterizerStats
{
    unsigned long int triangles{0};
    unsigned long int lines{0};
    unsigned long int points{0};
    unsigned long int xyClipped{0};
    unsigned long int outside{0};
    unsigned long int inside{0};
    unsigned long int depth{0};
    friend std::ostream& operator<<(std::ostream& os, const rasterizerStats &stats);
};

// for std::cout << myRasterizerStats;
std::ostream& operator<<( std::ostream &os, const rasterizerStats &stats )
{
    float pixelPercentage = 100.0f * stats.inside / (stats.inside + stats.outside);
    os << "rasterizer statistics:" << std::endl
       << "  triangles    " << stats.triangles << std::endl
       << "  lines        " << stats.lines << std::endl
       << "  points       " << stats.points << std::endl
       << "  xy-clipped   " << stats.xyClipped << std::endl
       << "  outside      " << stats.outside << std::endl
       << "  inside       " << stats.inside << std::endl
       << "  in : out     " << pixelPercentage << std::endl
       << "  drawn points " << stats.points << std::endl
       << "  depth        " << stats.depth << std::endl;
    return os;
}


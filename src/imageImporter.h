#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <string>

namespace minity
{

struct image
{
public:
    int width{0};
    int height{0};
    int components{0}; // typically rgba = 4

    bool load(const std::string path)
    {
        assert(_raw_data == nullptr); // only load once
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &components, 0); // don't force the number of components
        if(!data)
        {
            std::cerr << "Could not load " << path << " (no data)." << std::endl;
            return false;
        }
        std::cout << "loaded a " << width << "x" << height << " image with " << components << " components per pixel." << std::endl;
        _raw_data = data;
        return true;
    }
    ~image()
    {
        stbi_image_free(_raw_data);
    }
private:
    unsigned char *_raw_data{nullptr};
};

} // minity

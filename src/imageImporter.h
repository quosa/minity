#pragma once
#ifdef IMAGEIMPORTER_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
#endif // IMAGEIMPORTER_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <string>
#include <cassert>

namespace minity
{

struct image
{
public:
    int width{0};
    int height{0};
    int components{0}; // typically rgb = 3 or rgba = 4

    bool load(const std::string path, bool flipVertically = false)
    {
        assert(_raw_data == nullptr); // only load once
        if (flipVertically)
        {
            stbi_set_flip_vertically_on_load(1); // flag_true_if_should_flip
        }
        unsigned char *data = stbi_load(path.c_str(), &width, &height, &components, 0); // 0 = don't force the number of components
        if(!data)
        {
            std::cerr << "Could not load " << path << " (no data)." << std::endl;
            return false;
        }
        std::cout << "loaded a " << width << "x" << height << " image with " << components << " components per pixel." << std::endl;
        _raw_data = data;
        return true;
    }

    // stb_image: the first pixel pointed to is top-left-most in the image
    // 0,0 = top-left (or bottom-left if flipped vertically)
    // 1,0 = top-right (or bottom-right if flipped vertically)
    // 0,1 = bottom-left (or top-left if flipped vertically)
    // 1,1 = bottom-right (or top-right if flipped vertically)
    u_int32_t get(float u, float v) const
    {
        int x = std::min(static_cast<int>(u * width), width - 1); // [0,width - 1]
        int y = std::min(static_cast<int>(v * height), height-1); // [0, height - 1]
        assert(0 <= x && x < width);
        assert(0 <= y && y < height);
        assert(_raw_data != nullptr);

        auto data = _raw_data + (x + y * width) * components;
        // stb_image: comp 3 are red, green, blue
        // stb_image: comp 4 are red, green, blue, alpha
        // this sets the output to 0xrrggbbaa
        u_int32_t color = (data[3]<<0) | (data[2]<<8) | (data[1]<<16) | ((unsigned)data[0]<<24);
        if (components == 3)
        {
            color = color | 0xff;
        }

        return color;
    }

    ~image()
    {
        stbi_image_free(_raw_data);
    }
private:
    unsigned char *_raw_data{nullptr};
};

} // minity

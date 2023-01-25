#pragma once

#ifdef IMAGEIMPORTER_IMPLEMENTATION
    #define STB_IMAGE_IMPLEMENTATION
#endif // IMAGEIMPORTER_IMPLEMENTATION
#include <stb/stb_image.h>

#define MINITY_SCENE_TYPES_ONLY
// #include "scene.h"
#include "new_scene.h"

#include <iostream>
#include <string>
#include <cassert>
#include <memory>

namespace minity
{

struct imageImporter
{
public:
    imageImporter() : image_(std::make_shared<image>()) {}
    ~imageImporter() { image_.reset(); }
    std::shared_ptr<image> load(const std::string &path, bool flipVertically = false);
private:
    std::shared_ptr<image> image_{nullptr};
};


std::shared_ptr<image> imageImporter::load(const std::string &path, bool flipVertically)
{
    if (image_)
    {
        image_.reset();
        image_ = std::make_shared<image>();
    }
    if (flipVertically)
    {
        stbi_set_flip_vertically_on_load(1); // flag_true_if_should_flip
    }
    unsigned char *data = stbi_load(path.c_str(), &image_->width, &image_->height, &image_->components, 4); // 4 = force the number of components
    if(image_->components == 3) // only rgb for each pixel, no alpha
    {
        image_->components = 4; // we forced stbi to load as rgba 32bit for GPU/metal optimization
    }
    if(!data)
    {
        // std::cerr << "Could not load " << path << " (no data)." << std::endl;
        throw std::runtime_error("Could not load " + path + " (no data).");
    }
    std::cout << "loaded a " <<  image_->width << "x" <<  image_->height << " image with " <<  image_->components << " components per pixel." << std::endl;
    image_->set(data);
    // stbi_image_free(data);
    return image_;
}

} // minity

# minity
Minity is a minimal 3d game engine.

![Minity spinning a Utah teapot wireframe](./doc/img/minity-utah-teapot.png "Minity spinning a Utah teapot wireframe")

![Minity spinning a shaded Utah teapot](./doc/img/minity-utah-teapot-shaded.png "Minity spinning a shaded Utah teapot")

The main purpose is to do a fun project to re-learn [modern](https://docs.microsoft.com/en-us/cpp/cpp/welcome-back-to-cpp-modern-cpp?view=vs-2019) [C++](https://isocpp.org/) and try to follow the new [guidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines). [SDL2](https://www.libsdl.org/) is used to get a simple canvas to draw to (`setPixel(x, y)`) and input handling. Math is written all by hand to re-learn vector and matrix operations. Write all graphics pipeline operations by hand to understand what the GPU does for us and compare them with [metal API](https://developer.apple.com/metal/)](https://developer.apple.com/metal/) results. [Dear ImGui](https://github.com/ocornut/imgui) is used for adding a statistics window to the rendered area.

Design choices:
 * matrices are in [row-major order](https://en.wikipedia.org/wiki/Row-_and_column-major_order) to align with maths notation (unlike OpenGL!)
 * coordinate system and rotation follow the [right-hand rule](https://en.wikipedia.org/wiki/Cartesian_coordinate_system) like OpenGL
   * axis rotation follows the right-hand rule: right thumb to positive axis and fingers point to positive rotation
 * camera is watching towards the negative Z-axis
 * screen coordinates are top-left (0,0) to bottom-right (screen-width, screen-height)
 * texture coordinates are top-left (0,0) to bottom-right (1,1) when flipped vertically on load
 * model faces are defined in clockwise winding order
faces follow the LEFT-hand rule: e.g. center > up > right and LEFT-thumb points front

Working:
 * simple scene object with light, camera, and model (no entity hierarchy)
 * object loading from a simple obj file (vertices, faces, normals, and texture coordinates)
 * object scaling, rotation, and movement (translation)
 * rudimentary input handling for move ([arrow-keys](https://en.wikipedia.org/wiki/Arrow_keys), +, -) and rotate ([wasd](https://en.wikipedia.org/wiki/Arrow_keys#WASD_keys))
 * camera rotation and move (TODO: better key mapping for exploration)
 * [orthographic](https://en.wikipedia.org/wiki/Orthographic_projection) perspective correction with fixed [FoV, field-of-view](https://en.wikipedia.org/wiki/Angle_of_view)
 * rudimentary clipping (TODO: proper clipping)
 * face normal check to discard hidden triangles (model needs to be in left-hand/clockwise winding order!)
 * draw wireframe, normals, point cloud, and axes
 * global face color
 * [Phong-shading](https://en.wikipedia.org/wiki/Phong_shading) with normals for a smooth surface
textures with perspective-corrected model texture coordinates
 * basic math tests with [Catch2](https://github.com/catchorg/Catch2)
 * z-buffer check (instead of z-sorting the vertices before rendering)
 * // simple util for generating a sphere
 * rendering on [metal API](https://developer.apple.com/metal/)

TODO:
 * proper clipping (zoom in and the entire face is removed if clipped)
 * input handling:
   * camera movement is not good for exploration
   * cannot change the target between mesh/camera/light
 * wire-frame line color (done) > painting (done) > texturing (done) > anti-aliasing
 * vertex/face color from model
 * jump from classic make to CMake
 * consider `clang-format`
 * add address/thread/memory sanitizer
 * fix utils / sphere to work again with the new model structure
 * call model->update() or something from run loop

Main influences:
 * [OneLoneCoder/Javidx9](https://github.com/OneLoneCoder): console game engine and 3d videos
 * [Michael Kissner/Kayzaks](https://github.com/Kayzaks): StupidGL [gamasutra article](https://gamasutra.com/blogs/MichaelKissner/20160112/263097/Writing_a_Game_Engine_from_Scratch__Part_4_Graphics_Library.php)
 * [Gabriel Gambetta](https://gabrielgambetta.com/computer-graphics-from-scratch/): Computer Graphics from Scratch
 * [Dmitry Sokolov](https://github.com/ssloy/tinyrenderer): Tiny renderer

# Gallery

Tilted yellow sphere with 50 meridians and parallels in basic global illumination:
![Minity showing a yellow sphere](./doc/img/minity-50-50-yellow-sphere.png "Minity showing a yellow sphere")

Simple statistics window (turn on with [F1]) to get basic fps and library information:
![Minity showing statistics window](./doc/img/minity-stats-window.png "Minity showing statistics window")

Head model with texture (incomplete mesh split and no eye/lash textures)
![Minity showing a textured head](./doc/img/minity-head-with-texture.png "Minity showing a head with texture")

Texture with perspective-correct interpolation (no dent in face)

![Minity showing texture perspective correct interpolation](./doc/img/texture_with_perspective_correction.png "Minity showing texture perspective correct interpolation")

Rendering differences between software rasterizer and [metal API](https://developer.apple.com/metal/)

![Minity software rendered textured head](./doc/img/software_rasterizer_head.png "Minity software rendered textured head")

![Minity metal API rendered textured head](./doc/img/metal_head.png "Minity metal API rendered textured head")

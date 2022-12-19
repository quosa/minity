# minity
Minity is a minimal 3d game engine.

![Minity spinning a Utah teapot wireframe](./doc/img/minity-utah-teapot.png "Minity spinning a Utah teapot wireframe")

![Minity spinning a shaded Utah teapot](./doc/img/minity-utah-teapot-shaded.png "Minity spinning a shaded Utah teapot")

The main purpose is to do a fun project to re-learn [modern](https://docs.microsoft.com/en-us/cpp/cpp/welcome-back-to-cpp-modern-cpp?view=vs-2019) [C++](https://isocpp.org/) and try to follow the new [guidelines](http://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines). [SDL2](https://www.libsdl.org/) is used to get a simple canvas to draw to (`setPixel(x, y)`) and input handling. Math is written all by hand to re-learn vector and matrix operations. For now, no GPU acceleration, write all graphics pipeline operations by hand to understand what the GPU does for us. [Dear ImGui](https://github.com/ocornut/imgui) is used for adding a statistics window to the rendered area.

Design choices:
 * matrices are in [row-major order](https://en.wikipedia.org/wiki/Row-_and_column-major_order) to align with maths notation (unlike OpenGL!)
 * coordinate system and rotation follow the [right-hand rule](https://en.wikipedia.org/wiki/Cartesian_coordinate_system)
 * camera is watching towards the positive Z-axis (TODO: swap?)
 * screen coordinates are top-left (0,0) to bottom-right (screen-width, screen-height)

Working:
 * object loading from a simple obj file (vertices and faces only)
 * object scaling, rotation, and movement (translation)
 * rudimentary input handling for move ([arrow-keys](https://en.wikipedia.org/wiki/Arrow_keys), +, -) and rotate ([wasd](https://en.wikipedia.org/wiki/Arrow_keys#WASD_keys))
 * camera rotation and move (TODO: better key mapping for exploration)
 * [orthographic](https://en.wikipedia.org/wiki/Orthographic_projection) perspective correction with fixed [FoV, field-of-view](https://en.wikipedia.org/wiki/Angle_of_view)
 * rudimentary clipping (TODO: proper clipping)
 * face normal check to discard hidden triangles (model needs to be in clockwise winding order!)
 * wireframe and global face color
 * basic math tests with [Catch2](https://github.com/catchorg/Catch2)
 * z-buffer check (instead of z-sorting the vertices before rendering)
 * simple util for generating a sphere

TODO:
 * proper clipping (zoom in and the entire face is removed if clipped)
 * input handling:
   * camera movement is not good for exploration
   * cannot change the target between mesh/camera/light
 * scene concept
 * wire-frame line color (done) > painting (done) > texturing > anti-aliasing
 * vertex/face color from model
 * jump from classic make to CMake
 * consider `clang-format`
 * add address/thread/memory sanitizer

Main influences:
 * [OneLoneCoder/Javidx9](https://github.com/OneLoneCoder): console game engine and 3d videos
 * [Michael Kissner/Kayzaks](https://github.com/Kayzaks): StupidGL [gamasutra article](https://gamasutra.com/blogs/MichaelKissner/20160112/263097/Writing_a_Game_Engine_from_Scratch__Part_4_Graphics_Library.php)

# Gallery

Tilted yellow sphere with 50 meridians and parallels in basic global illumination:
![Minity showing a yellow sphere](./doc/img/minity-50-50-yellow-sphere.png "Minity showing a yellow sphere")

Simple statistics window (turn on with [F1]) to get basic fps and library information:
![Minity showing statistics window](./doc/img/minity-stats-window.png "Minity showing statistics window")

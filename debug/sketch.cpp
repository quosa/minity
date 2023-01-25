/*
clang++ -std=c++17 debug/sketch.cpp -o bin/sketch && ./bin/sketch
update(1.23) yRot: -30 isKeyPressed(minity::KEY_LEFT) 0
update(0.01) yRot: -17.7 isKeyPressed(minity::KEY_LEFT) 0
update(0.01) yRot: -17.6 isKeyPressed(minity::KEY_LEFT) 0
update(0.01) yRot: -17.5 isKeyPressed(minity::KEY_LEFT) 0
update(0.01) yRot: -17.4 isKeyPressed(minity::KEY_LEFT) 0
update(0.01) yRot: -17.3 isKeyPressed(minity::KEY_LEFT) 0
update(0.01) yRot: -17.2 isKeyPressed(minity::KEY_LEFT) 0
update(0.01) yRot: -17.1 isKeyPressed(minity::KEY_LEFT) 0
update(0.01) yRot: -17 isKeyPressed(minity::KEY_LEFT) 0
update(0.01) yRot: -16.9 isKeyPressed(minity::KEY_LEFT) 0
update(0.01) yRot: -16.8 isKeyPressed(minity::KEY_LEFT) 0
*/

#include <iostream>

constexpr float deg2rad(float deg) {return deg;}; // TODO: proper implementation

typedef u_int32_t color;
const color yellow{0xffff00ff};

struct vec3
{
    float x = 0;
    float y = 0;
    float z = 0;
    float w = 1;
};

namespace minity
{
struct texture {};

class imageImporter
{
public:
    texture load(std::string path, bool flip) {(void)path; (void)flip; return texture{};}
};

struct mesh {};

class meshImporter
{
public:
    mesh load(std::string path, bool reverseWining) {(void)path; (void)reverseWining; return mesh{};}
};

struct material
{
    color color; // rgba - 0xffff00ff = yellow
    float reflectivity;
    // transparency and refraction later
    texture texture;
};

struct model; // fwd

typedef std::function< void(float) > updateMethod;
typedef std::function< updateMethod(minity::model *) > updateMethodFactory;
// default null-update method that does nothing
auto nullUpdater = [](float timeDelta) -> void { (void)timeDelta; };
struct model
{
    mesh mesh;
    material material;
    vec3 position{};
    vec3 rotation{};
    vec3 scale{};
    updateMethod update = nullUpdater;
    void setUpdate(updateMethodFactory updateMaker) { update = updateMaker(this); }
};

enum cameraType {lookAt, fps};
struct camera
{
    cameraType type{lookAt};
    float fovDegrees;
    vec3 rotation;
    vec3 position;
    // type, view matrix parameters etc.
};

enum lightType {ambient, directional, point, spot};

struct light
{
    // type, light parameters
    lightType type{directional};
    vec3 position;
    // some cone parameters...
};

// TODO: turn to vectors?
struct scene
{
    camera camera;
    light light;
    model model;
};

enum keyCode
{
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    KEY_SPACE,
    KEY_a, //wasd left
    KEY_d, //wasd right
    KEY_w, //wasd up
    KEY_s, //wasd down
};

class input
{
public:
    static bool isKeyPressed(keyCode key) {return false;}
    static bool isKeyDown(keyCode key) {return false;}
    static bool isKeyUp(keyCode key) {return false;} // needed?
    static input& instance() {
        static input instance;
        return instance;
    }
private:
    input() {};
    ~input() {};
};

enum backend {software, metal};
class minity
{
public:
    minity(backend renderingBackend) : m_input(input::instance()) {
        (void)renderingBackend;
    };
    // void run(scene scene) {(void)scene;};
    void run(scene scene) { for (int i = 0; i < 10; i++) scene.model.update(0.01f); };
    void shutdown() {};
//     static minity& instance() {
//         static minity m;
//         return m;
//     }

// private:
//     Singleton();
//     ~Singleton();};
private:
    input &m_input;
};

} // NS minity

int main()
{
    minity::minity minity(minity::backend::metal); // too many minity?

    minity::imageImporter imgImporter{};
    auto texture = imgImporter.load("path/to/texture.png", true); // flip

    minity::meshImporter meshImporter{};
    auto mesh = meshImporter.load("test/models/teapot.obj", true); // reverse winding

    minity::material material{yellow, 1.0f, texture};

    minity::model model{mesh, material};
    model.scale = vec3{1.0f, 1.0f, 1.0f};
    model.rotation = vec3{deg2rad(30), deg2rad(-30), deg2rad(0)};
    model.position = vec3{0.0f, -1.5f, 0.0f};
    // model.update = []()

    minity::camera camera{minity::cameraType::lookAt};
    camera.fovDegrees = 50.0f;
    camera.position = vec3{0.0f, 0.0f, 5.0f};
    camera.rotation = vec3{deg2rad(0), deg2rad(0), deg2rad(0)};

    minity::light light{minity::lightType::directional};
    light.position = vec3{-1.0f, 1.0f, 10.0f}; // top-left


    model.update(0.01f);
    // update method factory that binds self to this
    auto updateFactory = [](minity::model *self)
    {
        return [self](float timeDelta)
        {
            minity::input &input = minity::input::instance();

            float rotationSpeed = 10.0f;
            std::cout << "update(" << timeDelta << ") yRot: " << self->rotation.y << " isKeyPressed(minity::KEY_LEFT) " << input.isKeyPressed(minity::KEY_LEFT)<< std::endl;
            self->rotation.y += timeDelta * rotationSpeed;
        };
    };
    model.setUpdate(updateFactory);
    model.update(1.23f);

    minity::scene scene{camera, light, model}; // todo: pass by reference (or pointer)

    minity.run(scene);
    minity.shutdown();
}

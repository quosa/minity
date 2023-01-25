/*
clang++ -std=c++17 debug/input.cpp -o bin/input && ./bin/input
isKeyPressed(KEY_LEFT): 0
*/

#include <iostream>

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

int main()
{
    input& i = input::instance();
    std::cout << "isKeyPressed(KEY_LEFT): " << i.isKeyPressed(KEY_LEFT) << std::endl;
    return 0;
}

#pragma once

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_metal.h"
#include <SDL.h>
#include <iostream>
#include <set>

namespace minity
{

enum keyCode
{
    KEY_LEFT = SDLK_LEFT,
    KEY_RIGHT = SDLK_RIGHT,
    KEY_UP = SDLK_UP,
    KEY_DOWN = SDLK_DOWN,
    KEY_SPACE = SDLK_SPACE,
    KEY_PLUS = SDLK_PLUS, // zoom in, z+
    KEY_MINUS = SDLK_MINUS, // zoom out z-
    KEY_a = SDLK_a, // wasd left
    KEY_d = SDLK_d, // wasd right
    KEY_w = SDLK_w, // wasd up
    KEY_s = SDLK_s, // wasd down
    KEY_l = SDLK_l, // l for draw wireframe "(l)ines"
    KEY_F1 = SDLK_F1, // show stats window
};

class input
{
public:
    bool handleInput();
    bool isKeyPressed(keyCode key) {return pressedKeys.find(key) != pressedKeys.end();}
    bool isKeyDown(keyCode key) {return m_keyState[SDL_GetScancodeFromKey(key)];}
    bool isKeyUp(keyCode key) {return !m_keyState[SDL_GetScancodeFromKey(key)];} // needed?
    static input& instance() {
        static input instance;
        return instance;
    }
private:
    const Uint8 *m_keyState;
    std::set<keyCode> pressedKeys{};

    input() : m_keyState(SDL_GetKeyboardState(NULL)) {};
    ~input() {};
};

// Use events for single-shot events (toggle wireframe)
// Use key state map for continuous events (wasd/<>^./fire etc.)
// return true unless it's time to quit the application
bool input::handleInput()
{
    SDL_Event sEvent;
    pressedKeys.clear();

    while (SDL_PollEvent(&sEvent))
    {
        ImGui_ImplSDL2_ProcessEvent(&sEvent);

        switch (sEvent.type)
        {
        case SDL_QUIT:
            return false;
            break;
        case SDL_KEYDOWN:
            // pressedKeys.insert(sEvent.key.keysym.sym);
            switch (sEvent.key.keysym.sym)
            {
            case SDLK_LEFT:
                break;
            case SDLK_RIGHT:
                break;
            case SDLK_UP:
                break;
            case SDLK_DOWN:
                break;
            case SDLK_PLUS:
                break;
            case SDLK_MINUS:
                break;
            case SDLK_a:
                break;
            case SDLK_d:
                break;
            case SDLK_w:
                break;
            case SDLK_s:
                break;
            case SDLK_n:
                break;
            case SDLK_l:
                pressedKeys.insert(KEY_l);
                break;
            case SDLK_p:
                break;
            case SDLK_f:
                break;
            case SDLK_r:
                break;
            case SDLK_x:
                break;
            case SDLK_F1:
                pressedKeys.insert(KEY_F1);
                break;
            case SDLK_q:
                return false;
                break;
            default:
                break;
            }
            break;
        }
    }

    m_keyState = SDL_GetKeyboardState(NULL);

    return true;
}
} // NS minity


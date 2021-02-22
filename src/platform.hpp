#pragma once

#include <LLGL/LLGL.h>
#include <SDL2/SDL.h>
#include <glm/glm.hpp>
#include <chrono>

namespace rise {
    SDL_Window* createGameWindow(std::string const& title, unsigned width, unsigned height);

    class Surface : public LLGL::Surface {
    public:
        explicit Surface(SDL_Window* sdlWindow) : window(sdlWindow) {}

        bool GetNativeHandle(void *nativeHandle, std::size_t nativeHandleSize) const override;

        LLGL::Extent2D GetContentSize() const override;

        bool AdaptForVideoMode(LLGL::VideoModeDescriptor &videoModeDesc) override;

        void ResetPixelFormat() override;

        bool ProcessEvents() override;

        std::unique_ptr<LLGL::Display> FindResidentDisplay() const override;

    private:
        SDL_Window *window;
    };

    namespace events {
        bool isPressed(SDL_Keycode key);

        bool isDown(SDL_KeyCode key) ;

        bool isUp(SDL_KeyCode key);

        void relativeMode(bool enabled);

        glm::vec2 mouseOffset() ;

        std::chrono::milliseconds pullTime();

        bool pull(SDL_Window* window);
    }
}
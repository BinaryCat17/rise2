#include "platform.hpp"
#include <LLGL/Platform/NativeHandle.h>
#include <SDL2/SDL_syswm.h>

namespace rise {

    bool Surface::GetNativeHandle(void *nativeHandle, std::size_t nativeHandleSize) const {
        if (nativeHandleSize == sizeof(LLGL::NativeHandle)) {
            auto handle = reinterpret_cast<LLGL::NativeHandle *>(nativeHandle);
            SDL_SysWMinfo wmInfo;
            SDL_VERSION(&wmInfo.version)
            SDL_GetWindowWMInfo(window, &wmInfo);
#if defined(SDL_VIDEO_DRIVER_X11)
            handle->window = wmInfo.info.x11.window;
            handle->display = wmInfo.info.x11.display;
#endif
#if defined(SDL_VIDEO_DRIVER_WINDOWS)
            handle->window = wmInfo.info.win.window;
#endif
            return true;
        }
        return false;

    }

    LLGL::Extent2D Surface::GetContentSize() const {
        int width = 0, height = 0;
        SDL_GetWindowSize(window, &width, &height);
        return {static_cast<unsigned>(width), static_cast<unsigned>(height)};
    }

    bool Surface::AdaptForVideoMode(LLGL::VideoModeDescriptor &videoModeDesc) {
        auto resolution = videoModeDesc.resolution;
        SDL_SetWindowSize(window, static_cast<int>(resolution.width),
                static_cast<int>(resolution.height));
        return true;
    }

    void Surface::ResetPixelFormat() {
        throw std::runtime_error("not implemented");
    }

    bool Surface::ProcessEvents() {
        throw std::runtime_error("not implemented");
    }

    std::unique_ptr<LLGL::Display> Surface::FindResidentDisplay() const {
        throw std::runtime_error("not implemented");
    }

    class EventListener {
        friend EventListener &eventListener();

    public:
        EventListener() {
            SDL_Init(SDL_INIT_EVENTS);
        }

        bool isPressed(SDL_Keycode key) {
            return keys[key].isPressed;
        }

        bool isDown(SDL_KeyCode key) {
            return keys[key].isDown;
        }

        bool isUp(SDL_KeyCode key) {
            return keys[key].isUp;
        }

        glm::vec2 mouseOffset() {
            return offset;
        }

        std::chrono::milliseconds pullEventsTime() {
            return elapsedTime;
        }

        bool pullEvents() {
            SDL_Event event;

            offset.x = 0;
            offset.y = 0;
            for (auto &key : keys) {
                key.second.isUp = false;
                key.second.isDown = false;
            }

            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_KEYUP:
                        keys[event.key.keysym.sym].isUp = true;
                        keys[event.key.keysym.sym].isPressed = false;
                        break;
                    case SDL_KEYDOWN:
                        keys[event.key.keysym.sym].isDown = true;
                        keys[event.key.keysym.sym].isPressed = true;
                        break;
                    case SDL_MOUSEMOTION:
                        if (relative) {
                            offset.x = static_cast<float>(event.motion.xrel);
                            offset.y = static_cast<float>(event.motion.yrel);
                        }
                    case SDL_WINDOWEVENT:
                        if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                            return false;
                        }
                        break;
                    default:
                        break;
                }
            }

            if (!relative) {
                int x = 0, y = 0;
                SDL_GetGlobalMouseState(&x, &y);
                glm::vec2 position(x, y);

                position.x = static_cast<float>(x);
                position.y = static_cast<float>(y);
                offset = position - lastPosition;

                lastPosition = position;
            }

            auto now = std::chrono::high_resolution_clock::now();
            elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime);
            lastTime = now;

            return true;
        }

        void relativeMode(bool enabled) {
            SDL_SetRelativeMouseMode(static_cast<SDL_bool>(enabled));
            relative = enabled;
        }

    private:
        ~EventListener() {
            SDL_Quit();
        }

        struct KeyInfo {
            bool isUp = false;
            bool isDown = false;
            bool isPressed = false;
        };
        std::map<SDL_Keycode, KeyInfo> keys;

        bool relative = false;
        glm::vec2 lastPosition = {};
        glm::vec2 offset = {};

        std::chrono::high_resolution_clock::time_point lastTime;
        std::chrono::milliseconds elapsedTime = {};
    };

    EventListener &eventListener() {
        static EventListener listener;
        return listener;
    }

    SDL_Window *createGameWindow(std::string const &title, unsigned width, unsigned height) {
        eventListener();
        auto window = SDL_CreateWindow(title.c_str(), 400, 0,
                static_cast<int>(width), static_cast<int>(height),
                SDL_WINDOW_VULKAN);
        if (!window) {
            throw std::runtime_error("fail to create window");
        }
        return window;
    }

    namespace events {
        bool isPressed(SDL_Keycode key) {
            return eventListener().isPressed(key);
        }

        bool isDown(SDL_KeyCode key) {
            return eventListener().isDown(key);
        }

        bool isUp(SDL_KeyCode key) {
            return eventListener().isUp(key);
        }

        glm::vec2 mouseOffset() {
            return eventListener().mouseOffset();
        }

        std::chrono::milliseconds pullTime() {
            return eventListener().pullEventsTime();
        }

        bool pull() {
            return eventListener().pullEvents();
        }

        void relativeMode(bool enabled) {
            return eventListener().relativeMode(enabled);
        }
    }
}
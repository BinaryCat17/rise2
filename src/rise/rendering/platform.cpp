#include "platform.hpp"
#include <LLGL/Platform/NativeHandle.h>

#ifdef __LINUX__

#include <SDL/SDL_config.h>

#endif

#include <SDL_syswm.h>
#include <backends/imgui_impl_sdl.h>

namespace rise {
    class Surface : public LLGL::Surface {
    public:
        explicit Surface(SDL_Window *sdlWindow) : window(sdlWindow) {}

        bool GetNativeHandle(void *nativeHandle, std::size_t nativeHandleSize) const override {
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


        LLGL::Extent2D GetContentSize() const override {
            int width = 0, height = 0;
            SDL_GetWindowSize(window, &width, &height);
            return {static_cast<unsigned>(width), static_cast<unsigned>(height)};
        }

        bool AdaptForVideoMode(LLGL::VideoModeDescriptor &videoModeDesc) override {
            auto resolution = videoModeDesc.resolution;
            SDL_SetWindowSize(window, static_cast<int>(resolution.width),
                    static_cast<int>(resolution.height));
            return true;
        }

        void ResetPixelFormat() override {
            throw std::runtime_error("not implemented");
        }

        bool ProcessEvents() override {
            throw std::runtime_error("not implemented");
        }

        std::unique_ptr<LLGL::Display> FindResidentDisplay() const override {
            throw std::runtime_error("not implemented");
        }

    private:
        SDL_Window *window;
    };

    LLGL::RenderContext *createRenderingContext(LLGL::RenderSystem *renderer, SDL_Window *window) {
        LLGL::RenderContextDescriptor contextDesc;
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        contextDesc.videoMode.resolution = {static_cast<uint32_t>(w), static_cast<uint32_t>(h)};
        contextDesc.videoMode.fullscreen = false;
        contextDesc.vsync.enabled = true;
        contextDesc.samples = 8;
        LLGL::RenderContext *context = renderer->CreateRenderContext(contextDesc,
                std::make_shared<Surface>(window));

        const auto &info = renderer->GetRendererInfo();

        std::cout << "Renderer:         " << info.rendererName << std::endl;
        std::cout << "Device:           " << info.deviceName << std::endl;
        std::cout << "Vendor:           " << info.vendorName << std::endl;
        std::cout << "Shading Language: " << info.shadingLanguageName << std::endl;

        return context;
    }

    bool pullInputEvents(SDL_Window *window) {
        SDL_Event event;

        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            switch (event.type) {
                case SDL_WINDOWEVENT:
                    if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
                        return false;
                    }
                    break;
                default:
                    break;
            }
        }

        ImGui_ImplSDL2_NewFrame(window);

        return true;
    }

    SDL_Window *createGameWindow(std::string const &title, glm::vec2 extent) {
        SDL_Init(SDL_INIT_EVENTS);
        auto window = SDL_CreateWindow(title.c_str(),
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                static_cast<int>(extent.x), static_cast<int>(extent.y), SDL_WINDOW_VULKAN);
        if (!window) {
            throw std::runtime_error("fail to create window");
        }
        return window;
    }
}
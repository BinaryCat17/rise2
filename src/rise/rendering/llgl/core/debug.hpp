#pragma once
#include <LLGL/LLGL.h>

namespace rise {
    class Debugger : public LLGL::RenderingDebugger {
    public:
    protected:
        void OnError(LLGL::ErrorType type, Message &message) override {
            //std::cout << message.GetText() << std::endl;
        }

        void OnWarning(LLGL::WarningType type, Message &message) override {
            //std::cout << message.GetText() << std::endl;
        }
    };
}
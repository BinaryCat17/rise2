#include "module.hpp"
#include "platform.hpp"

namespace rise {
    InputModule::InputModule(flecs::world &ecs) {

    }

    struct InputState {
        bool relative = false;
        SDL_Window *window = nullptr;
    };

    void move(glm::vec3 &position, glm::vec3 direction, float speed) {
        auto time = static_cast<float>(events::pullTime().count());
        position += speed * direction * (time / 17.f);
    }

    void processKeyboard(InputState &camera, glm::vec3 &position, glm::vec3 rotation) {
        glm::vec3 origin = calcOrigin(position, rotation);
        glm::vec3 direction = glm::normalize(position - origin);

        glm::vec3 right = glm::normalize(glm::cross({0.0f, 1.0f, 0.0f}, direction));
        glm::vec3 up = glm::cross(direction, right);
        glm::vec3 front = glm::cross(up, right);

        float const speed = 0.05f;

        if (events::isDown(SDLK_f)) {
            camera.relative = !camera.relative;
            events::relativeMode(camera.relative);
        }

        if (events::isPressed(SDLK_d)) {
            move(position, right, speed);
        }

        if (events::isPressed(SDLK_a)) {
            move(position, -right, speed);
        }

        if (events::isPressed(SDLK_w)) {
            move(position, front, speed);
        }

        if (events::isPressed(SDLK_s)) {
            move(position, -front, speed);
        }

        if (events::isPressed(SDLK_e)) {
            move(position, up, speed);
        }

        if (events::isPressed(SDLK_q)) {
            move(position, -up, speed);
        }
    }

    void processMouse(InputState &state, glm::vec3 &rotation) {
        if (state.relative) {
            float const speed = 0.5f;
            auto offset = events::mouseOffset() * speed;
            rotation.x += offset.x;
            rotation.z -= offset.y;
        }
    }

    void InputSystem::init(entt::registry &r) {
        auto &parameters = r.ctx_or_set<Parameters>();
        auto &state = r.set<InputState>();
        state.window = createGameWindow(parameters.title,
                static_cast<unsigned>(parameters.windowSize.x),
                static_cast<unsigned>(parameters.windowSize.y),
                static_cast<unsigned>(parameters.windowPos.x),
                static_cast<unsigned>(parameters.windowPos.y));
    }

    bool InputSystem::update(entt::registry &r) {
        auto &state = r.ctx<InputState>();
        auto camera = RenderSystem::getActiveCamera(r);

        auto &position = r.get<Position>(camera);
        auto &rotation = r.get<Rotation>(camera);

        glm::vec3 origin = calcOrigin(position.val, rotation.val);

        processKeyboard(state, position.val, rotation.val);
        processMouse(state, rotation.val);

        r.patch<Position>(camera, [](auto...) {});
        r.patch<Rotation>(camera, [](auto...) {});

        return events::pull(state.window);
    }

    void InputSystem::destroy(entt::registry &r) {

    }

    void InputSystem::setRelativeMode(entt::registry &r, bool enable) {
        auto &state = r.ctx<InputState>();
        state.relative = enable;
        events::relativeMode(enable);
    }

    SDL_Window *InputSystem::getWindow(entt::registry &r) {
        auto &state = r.ctx<InputState>();
        return state.window;
    }


}
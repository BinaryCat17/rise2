cmake_minimum_required(VERSION 3.6)
project(rise)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(submodules/LLGL)
add_subdirectory(submodules/entt)
add_subdirectory(submodules/glm)
add_subdirectory(submodules/tinyobjloader)
add_subdirectory(submodules/SDL2)
add_subdirectory(submodules/reactphysics3d)
add_subdirectory(submodules/flecs)

add_library(imgui
        submodules/imgui/imgui.cpp
        submodules/imgui/imgui_draw.cpp
        submodules/imgui/imgui_widgets.cpp
        submodules/imgui/imgui_tables.cpp
        submodules/imgui/imgui_demo.cpp
        submodules/ImGuizmo/ImGuizmo.cpp
        submodules/ImGuizmo/ImCurveEdit.cpp
        submodules/ImGuizmo/ImGradient.cpp
        submodules/ImGuizmo/ImSequencer.cpp
        submodules/imgui/backends/imgui_impl_sdl.cpp)

target_link_libraries(imgui PRIVATE SDL2)

target_include_directories(imgui PUBLIC submodules/imgui)
target_include_directories(imgui PUBLIC submodules/ImGuizmo)

add_library(rise
        src/rendering.cpp
        src/resources.cpp
        src/renderGui.cpp
        src/gui.cpp
        src/physic.cpp
        src/input.cpp
        src/platform.cpp
        src/utils.cpp
        src/pipeline.cpp)
target_include_directories(rise INTERFACE src)

add_executable(riseapp app/main.cpp)
add_executable(entt_test app/entt_test.cpp)
add_executable(flecs_test app/flecs_test.cpp)

target_link_libraries(rise PUBLIC imgui LLGL flecs_static glm::glm tinyobjloader SDL2 reactphysics3d)
target_link_libraries(riseapp PRIVATE rise)

target_link_libraries(flecs_test PRIVATE flecs_static)

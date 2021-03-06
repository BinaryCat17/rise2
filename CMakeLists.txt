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

add_library(flecs_meta submodules/flecs-meta/flecs_meta.c)
target_link_libraries(flecs_meta PRIVATE flecs_static)
target_include_directories(flecs_meta PUBLIC submodules/flecs-meta)

target_link_libraries(imgui PRIVATE SDL2)

target_include_directories(imgui PUBLIC submodules/imgui)
target_include_directories(imgui PUBLIC submodules/ImGuizmo)

add_library(rise
        src/rise/rendering/module.cpp
        src/rise/rendering/pipeline.cpp
        src/rise/rendering/platform.cpp
        src/rise/rendering/debug.cpp
        src/rise/rendering/resources.cpp)

target_include_directories(rise PUBLIC src)
target_include_directories(rise PRIVATE src/rise)

add_executable(riseapp app/main.cpp)
add_executable(entt_test app/entt_test.cpp)
add_executable(flecs_test app/flecs_test.cpp)

target_link_libraries(rise PUBLIC
        imgui
        LLGL
        flecs_static
        flecs_meta
        glm::glm
        tinyobjloader
        SDL2
        reactphysics3d)
target_link_libraries(riseapp PRIVATE rise)

target_link_libraries(flecs_test PRIVATE flecs_static flecs_meta)

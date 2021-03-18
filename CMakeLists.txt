cmake_minimum_required(VERSION 3.6)
project(rise)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(submodules/LLGL)
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
        submodules/imgui/misc/cpp/imgui_stdlib.cpp
        submodules/ImGuizmo/ImGuizmo.cpp
        submodules/ImGuizmo/ImCurveEdit.cpp
        submodules/ImGuizmo/ImGradient.cpp
        submodules/ImGuizmo/ImSequencer.cpp
        submodules/imgui/backends/imgui_impl_sdl.cpp)

add_library(flecs_deps
    submodules/flecs-meta/flecs_meta.c
    submodules/flecs-components-http/flecs_components_http.c
    submodules/flecs-json/flecs_json.c
    submodules/flecs-monitor/flecs_monitor.c
    submodules/flecs-rest/flecs_rest.c
    submodules/flecs-player/flecs_player.c
    submodules/flecs-dash/flecs_dash.c
    submodules/flecs-systems-civetweb/flecs_systems_civetweb.c
    )
target_link_libraries(flecs_deps PRIVATE flecs_static)
target_include_directories(flecs_deps PUBLIC
    submodules/flecs-meta
    submodules/flecs-components-http
    submodules/flecs-json
    submodules/flecs-player
    submodules/flecs-monitor
    submodules/flecs-rest
    submodules/flecs-dash
    submodules/flecs-systems-civetweb
    )

target_link_libraries(imgui PRIVATE SDL2)

target_include_directories(imgui PUBLIC submodules/imgui)
target_include_directories(imgui PUBLIC submodules/ImGuizmo)

find_package(Threads REQUIRED)

add_library(rise
        src/rise/util/flecs_os.cpp

        src/rise/rendering/module.cpp
        src/rise/rendering/editor.cpp
        src/rise/rendering/llgl/module.cpp
        src/rise/rendering/llgl/core/systems.cpp
        src/rise/rendering/llgl/core/texture.cpp
        src/rise/rendering/llgl/core/platform.cpp
        src/rise/rendering/llgl/core/utils.cpp
        src/rise/rendering/llgl/gui/systems.cpp
        src/rise/rendering/llgl/gui/pipeline.cpp
        src/rise/rendering/llgl/scene/mesh.cpp
        src/rise/rendering/llgl/scene/systems.cpp
        src/rise/rendering/llgl/scene/pipeline.cpp

        src/rise/input/module.cpp

        src/rise/editor/gui.cpp
        )

target_include_directories(rise PUBLIC src)
target_include_directories(rise PRIVATE src/rise)

add_executable(riseapp app/main.cpp)
add_executable(flecs_test app/flecs_test.cpp)

target_link_libraries(rise PUBLIC
        ${CMAKE_DL_LIBS}
        imgui
        LLGL
        flecs_static
        flecs_deps
        glm::glm
        tinyobjloader
        SDL2
        reactphysics3d)
target_link_libraries(riseapp PRIVATE rise)

target_link_libraries(flecs_test PRIVATE flecs_static flecs_deps)

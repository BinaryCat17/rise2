cmake_minimum_required(VERSION 3.6)
project(rise)

set(CMAKE_CXX_STANDARD 17)


add_subdirectory(submodules/LLGL)
add_subdirectory(submodules/entt)
add_subdirectory(submodules/glm)
add_subdirectory(submodules/tinyobjloader)
add_subdirectory(submodules/SDL2)

add_library(imgui
        submodules/imgui/imgui.cpp
        submodules/imgui/imgui_draw.cpp
        submodules/imgui/imgui_widgets.cpp
        submodules/imgui/imgui_tables.cpp
        submodules/imgui/imgui_demo.cpp
        submodules/imgui/backends/imgui_impl_sdl.cpp)

target_link_libraries(imgui SDL2)

target_include_directories(imgui PUBLIC submodules/imgui)

add_library(rise
        src/rendering.cpp
        src/resources.cpp
        src/gui.cpp
        src/platform.cpp
        src/pipeline.cpp)
target_include_directories(rise INTERFACE src)

add_executable(riseapp app/main.cpp )
add_executable(entt_test app/entt_test.cpp )


target_link_libraries(rise PUBLIC imgui LLGL EnTT glm::glm tinyobjloader SDL2)
target_link_libraries(riseapp rise)

target_link_libraries(entt_test EnTT)

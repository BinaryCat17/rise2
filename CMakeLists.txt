cmake_minimum_required(VERSION 3.6)
project(rise)

set(CMAKE_CXX_STANDARD 17)

add_library(rise
        src/rendering.cpp
        src/resources.cpp
        src/pipeline.cpp)
target_include_directories(rise INTERFACE src)

add_executable(riseapp app/main.cpp )
add_executable(entt_test app/entt_test.cpp )

add_subdirectory(submodules/LLGL)
add_subdirectory(submodules/entt)
add_subdirectory(submodules/glm)
add_subdirectory(submodules/tinyobjloader)

find_package(SDL2 REQUIRED)

target_link_libraries(rise PUBLIC LLGL EnTT glm::glm tinyobjloader SDL2)
target_link_libraries(riseapp rise)

target_link_libraries(entt_test EnTT)

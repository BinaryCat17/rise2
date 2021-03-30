#version 450 core

// Vertex input
layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform Model {
    mat4 transform;
} model;

void main() {
    gl_Position = model.transform * vec4(inPosition, 1.0);
}

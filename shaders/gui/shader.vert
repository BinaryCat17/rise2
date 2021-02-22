#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;

layout (binding = 0) uniform Parameters {
    vec2 scale;
    vec2 translate;
} parameters;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    outUV = inUV;
    outColor = inColor;
    vec2 pos = vec2(inPos.x , inPos.y);
    gl_Position = vec4(pos * parameters.scale + parameters.translate, 0.0, 1.0);
}
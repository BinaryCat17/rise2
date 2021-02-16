#version 450 core

// Vertex attributes (these names must match our vertex format attributes)
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

// Vertex output to the fragment shader
layout(location = 0) out vec3 vertexColor;

// uniform
layout(binding = 0) uniform mat4 MVP;

out gl_PerVertex
{
	vec4 gl_Position;
};

// Vertex shader main function
void main()
{
	gl_Position = MVP * vec4(position, 1);
	vertexColor = color;

}

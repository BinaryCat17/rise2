#version 450 core

// Vertex attributes
layout(location = 0) in vec3 position;

// Vertex output to the fragment shader
layout(location = 0) out vec3 vertexColor;

// uniform
layout(binding = 0) uniform MVP {
	mat4 mvp;
} ubo;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = ubo.mvp * vec4(position, 1);
	vertexColor = vec3(1, 0, 0);
}

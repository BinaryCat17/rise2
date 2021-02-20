#version 450 core

// Vertex input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

// Vertex output
layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;

// uniform

layout(binding = 0) uniform Global {
	mat4 view;
	mat4 projection;
} global;

layout(binding = 1) uniform Model {
	mat4 transform;
} model;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = global.projection * global.view * model.transform * vec4(position, 1);
	outPosition = vec3(model.transform * vec4(position, 1.0));
	outNormal = normal;
}

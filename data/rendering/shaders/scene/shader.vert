#version 450 core

// Vertex input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 texCoord;

// Vertex output
layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outColor;
layout(location = 3) out vec2 outTexCoord;

// uniform

struct PointLight {
	vec3 position;
	vec3 diffuse;
	float distance;
	float intensity;
};

const uint maxLightCount = 8;

layout(binding = 0) uniform Viewport {
	mat4 view;
	mat4 projection;
    PointLight pointLights[maxLightCount];
	vec3 viewPos;
	float farPlane;
} viewport;

layout(binding = 1) uniform Material {
	vec4 diffuseColor;
} material;

layout(binding = 2) uniform Model {
	mat4 transform;
} model;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = viewport.projection * viewport.view * model.transform * vec4(position, 1);
	outPosition = vec3(model.transform * vec4(position, 1.0));
	outNormal = normal;
	outColor = color;
	outTexCoord = texCoord;
}

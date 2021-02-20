#version 450 core

// Fragment input from the vertex shader
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;


// Fragment output color
layout(location = 0) out vec4 fragColor;

// Fragment shader main function

const vec3 lightPos = vec3(3, 3, 4);
const vec3 lightColor = vec3(1, 1, 1);

void main()
{
	vec3 norm = normalize(inNormal);
	vec3 lightDir = normalize(lightPos - inPosition);

	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	fragColor = vec4(diffuse, 1);
}
#version 450 core

// Fragment input from the vertex shader
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 texCoord;

// Fragment output color
layout(location = 0) out vec4 fragColor;

// Fragment shader main function

struct PointLight {
    vec3 position;
    vec3 diffuse;
    float constant;
    float linear;
    float quadratic;
};

const uint maxLightCount = 32;

layout(binding = 0) uniform Global {
    mat4 view;
    mat4 projection;
    PointLight pointLights[maxLightCount];
} global;

layout(binding = 2) uniform sampler modelSampler;
layout(binding = 3) uniform texture2D modelTexture;

const vec3 lightPos = vec3(3, 3, 4);
const vec3 lightColor = vec3(1, 1, 1);

void main()
{
    vec3 diffuseTex = texture(sampler2D(modelTexture, modelSampler), texCoord).xyz;
    vec3 norm = normalize(inNormal);

    vec3 resultColor = diffuseTex;

    for (int i = 0; i != maxLightCount; ++i) {
        PointLight light = global.pointLights[i];
        vec3 lightDir = normalize(light.position - inPosition);

        float attenuation = 1.f;
        if (light.constant != 0) {
            float distance  = length(light.position - inPosition);
            attenuation /= (light.constant + light.linear * distance +
            light.quadratic * (distance * distance));
        }

        resultColor += max(dot(norm, lightDir), 0.0) * attenuation;
    }

    fragColor = vec4(resultColor, 1);
}
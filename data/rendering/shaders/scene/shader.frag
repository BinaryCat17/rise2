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

layout(binding = 3) uniform sampler modelSampler;
layout(binding = 4) uniform texture2D modelTexture;
layout(binding = 5) uniform textureCubeArray depthMap;
layout(binding = 6) uniform sampler shadowSampler;

const float constantFactor = 1.0f;
const float linearFactor = 4.5;
const float quadraticFactor = 80.0;


vec3 sampleOffsetDirections[20] = vec3[]
(
vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

float shadowCalculation(vec3 fragPos, int lightId) {
    vec3 fragToLight = inPosition - viewport.pointLights[lightId].position;
    float currentDepth = length(fragToLight);

    float shadow = 0.0;
    float bias   = 0.15;
    int samples  = 20;
    float farPlane = viewport.farPlane;
    float viewDistance = length(viewport.viewPos - fragPos);
    float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;

    for (int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(samplerCubeArray(depthMap, shadowSampler),
            vec4(fragToLight + sampleOffsetDirections[i] * diskRadius, lightId)).r;

        closestDepth *= farPlane;// undo mapping [0;1]
        if (currentDepth - bias > closestDepth) {
            shadow += 1.0;
        }
    }
    shadow /= float(samples);

    return shadow;
}

void main()
{
    vec3 diffuseTex = texture(sampler2D(modelTexture, modelSampler), texCoord).xyz;
    vec3 norm = normalize(inNormal);
    vec3 resultColor = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i != maxLightCount; ++i) {
        PointLight light = viewport.pointLights[i];
        vec3 lightDir = normalize(light.position - inPosition);

        float attenuation = 1.f;
        if (light.distance != -1 && light.intensity != 0) {
            float distance  = length(light.position - inPosition);
            float constant = constantFactor * light.intensity;
            float linear = linearFactor / light.distance;
            float quadratic = linearFactor / (light.distance * light.distance) * light.intensity;
            attenuation /= (constant + linear * distance + quadratic * (distance * distance));

            resultColor += max(dot(norm, lightDir), 0.0) * attenuation * light.diffuse
            * light.intensity;

            float shadow = shadowCalculation(inPosition, i);
            resultColor *= (1.0f - shadow);
        }
    }

    vec3 ambient = 0.1 * (diffuseTex + material.diffuseColor.xyz);
    resultColor += ambient;
    resultColor *= diffuseTex;
    resultColor *= material.diffuseColor.xyz;

    fragColor = vec4(resultColor, 1);
}
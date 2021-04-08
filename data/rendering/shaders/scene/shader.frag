#version 450 core

// Fragment input from the vertex shader
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 texCoord;

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
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;
} material;

layout(binding = 2) uniform Model {
    mat4 transform;
} model;

layout(binding = 3) uniform sampler modelSampler;
layout(binding = 4) uniform texture2D albedoTexture;
layout(binding = 5) uniform texture2D metallicTexture;
layout(binding = 6) uniform texture2D roughnessTexture;
layout(binding = 7) uniform texture2D aoTexture;
layout(binding = 8) uniform textureCubeArray depthMap;
layout(binding = 9) uniform sampler shadowSampler;

const float constantFactor = 1.0f;
const float linearFactor = 4.5;
const float quadraticFactor = 80.0;
const float PI = 3.14159265359;

vec3 sampleOffsetDirections[20] = vec3[]
(
vec3(1, 1, 1), vec3(1, -1, 1), vec3(-1, -1, 1), vec3(-1, 1, 1),
vec3(1, 1, -1), vec3(1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0),
vec3(1, 0, 1), vec3(-1, 0, 1), vec3(1, 0, -1), vec3(-1, 0, -1),
vec3(0, 1, 1), vec3(0, -1, 1), vec3(0, -1, -1), vec3(0, 1, -1)
);

// функция нормального распределения - относительная площадь микрограней, точно ориентированных
// в сторону медианного вектора - тут будет наибольший блик
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// часть микрограней, который перекрылись
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// относительная площадь поверхности, где микрограни перекрывают друг друга
// чем больше шероховатость - тем больше шанс затенения микрограней
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    // от наблюдателя
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    // от света
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// коэффициент, определяющий отражаемость поверхности, в зависимости от угла обзора
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

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
    vec3 albedoTex = texture(sampler2D(albedoTexture, modelSampler), texCoord).xyz;
    float metallicTex = texture(sampler2D(metallicTexture, modelSampler), texCoord).x;
    float roughnessTex = texture(sampler2D(roughnessTexture, modelSampler), texCoord).x;
    float aoTex = texture(sampler2D(aoTexture, modelSampler), texCoord).x;
    vec3 albedo = albedoTex * material.albedo * inColor;
    float metallic = metallicTex * material.metallic;
    float roughness = roughnessTex * material.roughness;
    float ao = aoTex * material.ao;

    vec3 norm = normalize(inNormal);
    vec3 resultColor = vec3(0.0, 0.0, 0.0);

    // направление к наблюдателю
    vec3 viewDir = normalize(viewport.viewPos - inPosition);

    // Базовая отражательная способность - коэффициет отраженного света при прямом угле
    vec3 F0 = vec3(0.04);
    // 'Подкрашиваем' в зависимости от металличности поверхности и коэффициента поглощения
    F0 = mix(F0, albedo, metallic);

    for (int i = 0; i != maxLightCount; ++i) {
        PointLight light = viewport.pointLights[i];
        // направление к свету
        vec3 lightDir = normalize(light.position - inPosition);
        // медианный вектор
        vec3 medianVector = normalize(viewDir + lightDir);

        if (light.distance != -1 && light.intensity != 0) {
            // расстояние от источника света до позиции фрагмента
            float distance  = length(light.position - inPosition);

            // факторы затухания света
            float constant = constantFactor * light.intensity;
            float linear = linearFactor / light.distance;
            float quadratic = linearFactor / (light.distance * light.distance) * light.intensity;

            // коэффециент затухания
            float attenuation = 1.f;
            attenuation /= (constant + linear * distance + quadratic * (distance * distance));

            // энергетическая яркость света после угасания
            vec3 radiance = light.diffuse * attenuation;

            float NDF = DistributionGGX(norm, medianVector, roughness);
            float G   = GeometrySmith(norm, viewDir, lightDir, roughness);
            vec3 F    = fresnelSchlick(max(dot(medianVector, viewDir), 0.0), F0);

            // колличество отраженной световой энергии
            vec3 kS = F;
            // остаточная энергия - по закону сохранения энергии
            vec3 kD = vec3(1.0) - kS;
            // металлы не приломляют свет, грубо учитываем это
            kD *= 1.0 - metallic;

            // Cook-Torrance BRDF - уравнение отражения - подставляем в него все значения
            // для вычисления вклада луча в конечный отраженный свет
            vec3 numerator    = NDF * G * F;
            float denominator = 4.0 * max(dot(norm, viewDir), 0.0) * max(dot(norm, lightDir), 0.0);
            vec3 specular     = numerator / max(denominator, 0.001);

            float NdotL = max(dot(norm, lightDir), 0.0);

            resultColor += (kD * albedo / PI + specular) * radiance * NdotL;

            float shadow = shadowCalculation(inPosition, i);
            resultColor *= (1.0f - shadow);
        }
    }
    vec3 ambient = vec3(0.03) * albedo * ao;
    resultColor += ambient;

    fragColor = vec4(resultColor, 1);
}
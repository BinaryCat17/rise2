#version 450 core

layout(location = 0) in vec4 FragPos;

layout(binding = 2) uniform LightParams {
    vec3 lightPos;
    float farPlane;
};

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);

    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / farPlane;

    // write this as modified depth
    gl_FragDepth = lightDistance;
}
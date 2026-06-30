#version 450

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

layout(std140, set = 0, binding = 2) uniform SceneBuffer {
    vec3 color;
    float pad0;
    vec3 ambient;
    float pad1;
    vec3 direction;
} sceneBuffer;

void main()
{
    vec3 normal = normalize(inNormal);

    vec3 lightAmount = vec3(dot(normal, sceneBuffer.direction));
    lightAmount = clamp(lightAmount, sceneBuffer.ambient.xyz, vec3(1.0)) * sceneBuffer.color;

    outColor = vec4(fragColor, 1.0) * vec4(lightAmount, 1.0);
}

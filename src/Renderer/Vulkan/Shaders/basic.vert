#version 450

struct PerObjectInfo {
    mat4 world;
};

layout(push_constant) uniform ObjectIndex
{
    uint objectIndex;
} PushConstant;

layout(std140, set = 0, binding = 0) readonly buffer ObjectBuffer {
    PerObjectInfo objects[];
} objectBuffer;

layout(std140, set = 0, binding = 1) uniform UniformBufferObject
{
    mat4 viewProj;
} uniformObject;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec3 fragColor;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

void main()
{
    PerObjectInfo ob = objectBuffer.objects[PushConstant.objectIndex];

    gl_Position = uniformObject.viewProj * ob.world * vec4(inPosition, 1.0);
    mat3 normalMatrix = transpose(inverse(mat3(ob.world)));
    outNormal = normalize(normalMatrix * inNormal);

    if (PushConstant.objectIndex == 0)
    {
        fragColor = vec3(1.0, 1.0, 1.0);
    }
    else
    {
        fragColor = vec3(0.0, 1.0, 0.0);
    }

    // materialIndex = ob.materialIndex;
}

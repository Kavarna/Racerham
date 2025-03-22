#version 450

layout(push_constant) uniform camera
{
    mat4 viewProj;
} PushConstant;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = PushConstant.viewProj * vec4(inPosition, 1.0);

    fragColor = vec3(inColor);
}

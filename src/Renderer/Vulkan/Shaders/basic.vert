#version 450

layout(location = 0) out vec3 fragColor;

layout(location = 0) in vec3 inPosition;

void main()
{
    gl_Position = vec4(inPosition, 1.0);
    fragColor = vec3(1.0, 1.0, 0.0);
}

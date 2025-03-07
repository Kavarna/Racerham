#pragma once

#include <glm/glm.hpp>

#include <array>
#include <vulkan/vulkan.h>

struct VertexPosition
{
    glm::vec3 position;

    VertexPosition() = default;
    VertexPosition(float x, float y, float z) : position(x, y, z) {};

    static std::array<VkVertexInputAttributeDescription, 1>
    GetInputAttributeDescription()
    {
        std::array<VkVertexInputAttributeDescription, 1>
            attributeDescriptions{};
        {
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].offset =
                offsetof(VertexPosition, position);
        }

        return attributeDescriptions;
    }

    static std::array<VkVertexInputBindingDescription, 1>
    GetInputBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        {
            bindingDescription.binding = 0;
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescription.stride = sizeof(VertexPosition);
        }
        return {bindingDescription};
    }
};
struct VertexPositionNormal
{
    glm::vec3 position;
    glm::vec3 normal;

    VertexPositionNormal() = default;
    VertexPositionNormal(float x, float y, float z, float nx, float ny,
                         float nz)
        : position(x, y, z), normal(nx, ny, nz) {};

    static std::array<VkVertexInputAttributeDescription, 2>
    GetInputAttributeDescription()
    {
        std::array<VkVertexInputAttributeDescription, 2>
            attributeDescriptions{};
        {
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].offset =
                offsetof(VertexPositionNormal, position);
        }
        {
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].offset =
                offsetof(VertexPositionNormal, normal);
        }
        return attributeDescriptions;
    }

    static std::array<VkVertexInputBindingDescription, 1>
    GetInputBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        {
            bindingDescription.binding = 0;
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescription.stride = sizeof(VertexPositionNormal);
        }
        return {bindingDescription};
    }
};

struct VertexPositionColor
{
    glm::vec3 position;
    glm::vec4 color;

    VertexPositionColor() = default;
    VertexPositionColor(float x, float y, float z, float r, float g, float b,
                        float a)
        : position(x, y, z), color(r, g, b, a) {};
    VertexPositionColor(glm::vec3 const &pos, glm::vec4 const &col)
        : position(pos), color(col) {};

    static std::array<VkVertexInputAttributeDescription, 2>
    GetInputAttributeDescription()
    {
        std::array<VkVertexInputAttributeDescription, 2>
            attributeDescriptions{};
        {
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].offset =
                offsetof(VertexPositionColor, position);
        }
        {
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].offset =
                offsetof(VertexPositionColor, color);
        }
        return attributeDescriptions;
    }

    static std::array<VkVertexInputBindingDescription, 1>
    GetInputBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        {
            bindingDescription.binding = 0;
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            bindingDescription.stride = sizeof(VertexPositionColor);
        }
        return {bindingDescription};
    }
};

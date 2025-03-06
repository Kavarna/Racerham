#pragma once

#include <vulkan/vulkan.h>

#include "Image.h"

namespace Vulkan
{
using Attachment = u32;
class RenderPass
{
    friend class Framebuffer;
    friend class CommandList;
    friend class Pipeline;

public:
    RenderPass();
    ~RenderPass();

    /* TODO: Should make this non-copyable */

public:
    void Bake();

    Attachment AddAttachment(Image *image, VkImageLayout initialLayout, VkImageLayout finalLayout,
                             VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);
    Attachment AddBackbufferAttachment(VkImageLayout initialLayout, VkImageLayout finalLayout,
                                       VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp);

    Attachment AddAttachment(Image *image, VkImageLayout finalLayout, VkAttachmentLoadOp loadOp,
                             VkAttachmentStoreOp storeOp);
    Attachment AddBackbufferAttachment(VkImageLayout finalLayout, VkAttachmentLoadOp loadOp,
                                       VkAttachmentStoreOp storeOp);

    void AddColorAttachment(Attachment attachment, VkImageLayout layout);
    void AddInputAttachment(Attachment attachment, VkImageLayout layout);
    void AddPreserveAttachment(Attachment attachment, VkImageLayout layout);

    void PreviousSubpass();
    void NextSubpass();

private:
    void ResizeReferences(u32 newSize);

private:
    std::vector<VkAttachmentDescription> mAttachments;

    std::vector<std::vector<VkAttachmentReference>> mColorAttachmentReferences;
    std::vector<std::vector<VkAttachmentReference>> mInputAttachmentReferences;
    std::vector<std::vector<VkAttachmentReference>> mPreserveAttachmentReferences;

    u32 mCurrentSubpass = 0;
    u32 mSubpassCount = 0;

    VkRenderPass mRenderpass;
};

class Framebuffer
{
    friend class CommandList;

public:
    Framebuffer();
    ~Framebuffer();

public:
    void Bake(u32 width, u32 height, RenderPass *);

    void AddAttachment(ImageView *);
    void AddBackbufferAsAttachment(u32 index);

private:
    std::vector<VkImageView> mAttachments;

    u32 mWidth;
    u32 mHeight;
    VkFramebuffer mFramebuffer;
};

} // namespace Vulkan
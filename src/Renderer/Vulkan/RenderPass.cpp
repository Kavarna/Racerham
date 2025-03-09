#include "RenderPass.h"

#include "Renderer.h"

using namespace Vulkan;

RenderPass::RenderPass()
{
    ResizeReferences(1);
}

RenderPass::~RenderPass()
{
    auto device = Renderer::Get()->GetDevice();
    if (mRenderpass != VK_NULL_HANDLE)
    {
        jnrDestroyRenderPass(device, mRenderpass, nullptr);
    }
}

void RenderPass::ResizeReferences(u32 newSize)
{
    if (newSize > mSubpassCount)
    {
        mColorAttachmentReferences.resize(newSize);
        mInputAttachmentReferences.resize(newSize);
        mPreserveAttachmentReferences.resize(newSize);
        mSubpassCount = newSize;
    }
}

Attachment RenderPass::AddBackbufferAttachment(VkImageLayout initialLayout,
                                               VkImageLayout finalLayout,
                                               VkAttachmentLoadOp loadOp,
                                               VkAttachmentStoreOp storeOp)
{
    auto renderer = Renderer::Get();
    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.initialLayout = initialLayout;
    attachmentDescription.finalLayout = finalLayout;
    attachmentDescription.format = renderer->GetBackbufferFormat();
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = loadOp;
    attachmentDescription.storeOp = storeOp;

    u32 index = (u32)mAttachments.size();
    mAttachments.push_back(attachmentDescription);
    return index;
}

Attachment RenderPass::AddAttachment(Image *image, VkImageLayout initialLayout,
                                     VkImageLayout finalLayout,
                                     VkAttachmentLoadOp loadOp,
                                     VkAttachmentStoreOp storeOp)
{
    ThrowIfFailed(image->GetSampleCount() == VK_SAMPLE_COUNT_1_BIT,
                  "Only 1 sample images are allowed");

    VkAttachmentDescription attachmentDescription = {};
    attachmentDescription.initialLayout = initialLayout;
    attachmentDescription.finalLayout = finalLayout;
    attachmentDescription.format = image->GetFormat();
    attachmentDescription.samples = image->GetSampleCount();
    attachmentDescription.loadOp = loadOp;
    attachmentDescription.storeOp = storeOp;

    u32 index = (u32)mAttachments.size();
    mAttachments.push_back(attachmentDescription);
    return index;
}

Attachment RenderPass::AddAttachment(Image *image, VkImageLayout finalLayout,
                                     VkAttachmentLoadOp loadOp,
                                     VkAttachmentStoreOp storeOp)
{
    return AddAttachment(image, image->GetLayout(), finalLayout, loadOp,
                         storeOp);
}

Attachment RenderPass::AddBackbufferAttachment(VkImageLayout finalLayout,
                                               VkAttachmentLoadOp loadOp,
                                               VkAttachmentStoreOp storeOp)
{
    auto renderer = Renderer::Get();
    return AddBackbufferAttachment(renderer->mSwapchainImageLayouts[0],
                                   finalLayout, loadOp, storeOp);
}

void RenderPass::AddColorAttachment(Attachment attachment, VkImageLayout layout)
{
    VkAttachmentReference reference = {};
    reference.attachment = attachment;
    reference.layout = layout;

    mColorAttachmentReferences[mCurrentSubpass].push_back(reference);
}

void RenderPass::AddInputAttachment(Attachment attachment, VkImageLayout layout)
{
    VkAttachmentReference reference = {};
    reference.attachment = attachment;
    reference.layout = layout;

    mInputAttachmentReferences[mCurrentSubpass].push_back(reference);
}

void RenderPass::AddPreserveAttachment(Attachment attachment,
                                       VkImageLayout layout)
{
    VkAttachmentReference reference = {};
    reference.attachment = attachment;
    reference.layout = layout;

    mPreserveAttachmentReferences[mCurrentSubpass].push_back(reference);
}

void RenderPass::PreviousSubpass()
{
    mCurrentSubpass--;
}

void RenderPass::NextSubpass()
{
    mCurrentSubpass++;
    ResizeReferences(mCurrentSubpass);
}

void RenderPass::Bake()
{
    auto renderer = Renderer::Get();
    auto device = renderer->GetDevice();
    std::vector<VkSubpassDescription> subpassDescriptions;

    for (u32 i = 0; i < mSubpassCount; ++i)
    {
        VkSubpassDescription subpassDescription = {};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount =
            mColorAttachmentReferences[i].size();
        subpassDescription.pColorAttachments =
            mColorAttachmentReferences[i].data();
        subpassDescription.inputAttachmentCount =
            mInputAttachmentReferences[i].size();
        subpassDescription.pInputAttachments =
            mInputAttachmentReferences[i].data();
        subpassDescription.pDepthStencilAttachment =
            nullptr;                                      // TODO: Support this
        subpassDescription.pResolveAttachments = nullptr; // TODO: Support this

        subpassDescriptions.push_back(subpassDescription);
    }

    VkRenderPassCreateInfo renderpassInfo = {};
    {
        renderpassInfo.attachmentCount = mAttachments.size();
        renderpassInfo.pAttachments = mAttachments.data();
        renderpassInfo.pSubpasses = subpassDescriptions.data();
        renderpassInfo.subpassCount = subpassDescriptions.size();
        renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpassInfo.flags = 0;
    }

    vkThrowIfFailed(
        jnrCreateRenderPass(device, &renderpassInfo, nullptr, &mRenderpass));
}

Framebuffer::Framebuffer()
{
}

Framebuffer::~Framebuffer()
{
    auto device = Renderer::Get()->GetDevice();
    if (mFramebuffer != VK_NULL_HANDLE)
    {
        jnrDestroyFramebuffer(device, mFramebuffer, nullptr);
    }
}

void Framebuffer::AddAttachment(ImageView *view)
{
    mAttachments.push_back(view->GetView());
}

void Framebuffer::AddBackbufferAsAttachment(u32 index)
{
    auto renderer = Renderer::Get();
    mAttachments.push_back(renderer->GetSwapchainImageView(index));
}

void Framebuffer::Bake(u32 width, u32 height, RenderPass *renderpass)
{
    ThrowIfFailed(
        renderpass->mAttachments.size() == mAttachments.size(),
        "Number of attachments in framebuffer must match the render pass");
    auto renderer = Renderer::Get();
    auto device = renderer->GetDevice();

    mWidth = width;
    mHeight = height;

    VkFramebufferCreateInfo framebufferInfo = {};
    {
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.width = width;
        framebufferInfo.height = height;
        framebufferInfo.layers = 1;
        framebufferInfo.attachmentCount = mAttachments.size();
        framebufferInfo.pAttachments = mAttachments.data();
        framebufferInfo.renderPass = renderpass->mRenderpass;
    }

    vkThrowIfFailed(
        jnrCreateFramebuffer(device, &framebufferInfo, nullptr, &mFramebuffer));
}

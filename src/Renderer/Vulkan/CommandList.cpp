#include "CommandList.h"
#include "Image.h"
#include "Pipeline.h"
#include "RenderPass.h"
#include "Renderer.h"
#include "RootSignature.h"
#include "VulkanLoader.h"

using namespace Vulkan;

CommandList::CommandList(CommandListType cmdListType) : mType(cmdListType)
{
    auto renderer = Renderer::Get();
    u32 queueIndex = (u32)(-1);
    if (cmdListType == CommandListType::Graphics)
        queueIndex = renderer->mQueueIndices.graphicsFamily.value();
    ThrowIfFailed(queueIndex != (u32)(-1), "Invalid command list provided");

    VkCommandPoolCreateInfo poolInfo{};
    {
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueIndex;
    }

    vkThrowIfFailed(jnrCreateCommandPool(renderer->mDevice, &poolInfo, nullptr, &mCommandPool));
}

CommandList::~CommandList()
{
    auto device = Renderer::Get()->GetDevice();
    jnrDestroyCommandPool(device, mCommandPool, nullptr);
}

void CommandList::Init(u32 numCommandBuffers)
{
    ThrowIfFailed(numCommandBuffers > 0, "There should be at least one command buffer initialised");

    auto device = Renderer::Get()->GetDevice();
    ResetAll();

    mCommandBuffers.resize(numCommandBuffers);
    VkCommandBufferAllocateInfo allocInfo{};
    {
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandBufferCount = numCommandBuffers;
        allocInfo.commandPool = mCommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    }

    vkThrowIfFailed(jnrAllocateCommandBuffers(device, &allocInfo, mCommandBuffers.data()));
}

void CommandList::ResetAll()
{
    auto device = Renderer::Get()->GetDevice();
    vkThrowIfFailed(jnrResetCommandPool(device, mCommandPool, 0));
}

void CommandList::Begin()
{
    VkCommandBufferBeginInfo beginInfo = {};
    {
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    }
    vkThrowIfFailed(jnrBeginCommandBuffer(mCommandBuffers[mActiveCommandIndex], &beginInfo));
    {
        /* Reset current recording info */
        mImageIndex = -1;
        mTemporaryStorageOffset = 0;
    }

    /* DSHOWINFO("[", (void *)mCommandBuffers[mActiveCommandIndex], "] Start recording command buffer"); */
}

void CommandList::End()
{
    if (mImageIndex != -1)
    {
        /* Then we must have used the back buffer so transition it back */
        {
            TransitionInfo ti{};
            {
                ti.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                ti.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                ti.srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                ti.dstStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            }
            TransitionBackbufferTo(ti);
        }
    }

    vkThrowIfFailed(jnrEndCommandBuffer(mCommandBuffers[mActiveCommandIndex]));

    /* TODO: To be more correct, we could do this after the CPUSynchronizationObject was triggered */
    mLayoutTracker.Flush();
    mMemoryTracker.Flush();

    /*DSHOWINFO("[", (void *)mCommandBuffers[mActiveCommandIndex], "] Finish recording command buffer");*/
}

void Vulkan::CommandList::CopyBuffer(Vulkan::Buffer *dst, Vulkan::Buffer *src)
{
    CopyBuffer(dst, 0, src, 0);
}

void Vulkan::CommandList::CopyBuffer(Vulkan::Buffer *dst, u32 dstOffset, Vulkan::Buffer *src)
{
    CopyBuffer(dst, dstOffset, src, 0);
}

void Vulkan::CommandList::CopyBuffer(Vulkan::Buffer *dst, u32 dstOffset, Vulkan::Buffer *src, u32 srcOffset)
{
    ThrowIfFailed(dst->mCount >= src->mCount, "Cannot copy a larger buffer into a smaller one");

    VkBufferCopy copyInfo{};
    {
        copyInfo.srcOffset = srcOffset;
        copyInfo.dstOffset = dstOffset;
        copyInfo.size = src->GetElementSize() * src->mCount;
    }
    jnrCmdCopyBuffer(mCommandBuffers[mActiveCommandIndex], src->mBuffer, dst->mBuffer, 1, &copyInfo);
}

void Vulkan::CommandList::BindVertexBuffer(Vulkan::Buffer const *buffer, u32 firstIndex)
{
    VkDeviceSize offsets[] = {0};
    jnrCmdBindVertexBuffers(mCommandBuffers[mActiveCommandIndex], firstIndex, 1, &buffer->mBuffer, offsets);
}

void Vulkan::CommandList::BindIndexBuffer(Vulkan::Buffer const *buffer)
{
    VkIndexType indexType = VK_INDEX_TYPE_NONE_KHR;
    if (buffer->GetElementSize() == sizeof(u32))
        indexType = VK_INDEX_TYPE_UINT32;
    else if (buffer->GetElementSize() == sizeof(u16))
        indexType = VK_INDEX_TYPE_UINT16;
    jnrCmdBindIndexBuffer(mCommandBuffers[mActiveCommandIndex], buffer->mBuffer, 0, indexType);
}

void CommandList::Draw(u32 vertexCount, u32 firstVertex)
{
    jnrCmdDraw(mCommandBuffers[mActiveCommandIndex], vertexCount, 1, firstVertex, 0);
}

void Vulkan::CommandList::DrawIndexedInstanced(u32 indexCount, u32 firstIndex, u32 vertexOffset)
{
    jnrCmdDrawIndexed(mCommandBuffers[mActiveCommandIndex], indexCount, 1, firstIndex, vertexOffset, 0);
}

void CommandList::BindPipeline(Pipeline *pipeline)
{
    jnrCmdBindPipeline(mCommandBuffers[mActiveCommandIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->mPipeline);
}

void CommandList::BindDescriptorSet(DescriptorSet *set, u32 descriptorSetInstance, RootSignature *rootSignature)
{
    jnrCmdBindDescriptorSets(mCommandBuffers[mActiveCommandIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
                             rootSignature->mPipelineLayout, 0, 1, &set->mDescriptorSets[descriptorSetInstance], 0,
                             nullptr);
}

void CommandList::SetScissor(std::vector<VkRect2D> const &scissors)
{
    jnrCmdSetScissor(mCommandBuffers[mActiveCommandIndex], 0, (u32)scissors.size(), scissors.data());
}

void CommandList::SetViewports(std::vector<VkViewport> const &viewports)
{
    jnrCmdSetViewport(mCommandBuffers[mActiveCommandIndex], 0, (u32)viewports.size(), viewports.data());
}

void CommandList::TransitionBackbufferTo(TransitionInfo const &transitionInfo)
{
    auto renderer = Renderer::Get();
    VkImageMemoryBarrier imageMemoryBarrier{};
    {
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = transitionInfo.srcAccessMask;
        imageMemoryBarrier.dstAccessMask = transitionInfo.dstAccessMask;
        imageMemoryBarrier.oldLayout = mLayoutTracker.GetBackbufferImageLayout(mImageIndex);
        imageMemoryBarrier.newLayout = transitionInfo.newLayout;
        imageMemoryBarrier.image = renderer->mSwapchainImages[mImageIndex];

        imageMemoryBarrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
    }

    jnrCmdPipelineBarrier(mCommandBuffers[mActiveCommandIndex], transitionInfo.srcStage, transitionInfo.dstStage, 0, 0,
                          nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    mLayoutTracker.TransitionBackBufferImage(mImageIndex, transitionInfo.newLayout);

    /*DSHOWINFO("[", (void *)mCommandBuffers[mActiveCommandIndex], "] Transitioning backbuffer image ", mImageIndex,
              " from layout ", string_VkImageLayout(imageMemoryBarrier.oldLayout), " to ",
              string_VkImageLayout(imageMemoryBarrier.newLayout));*/
}

void CommandList::TransitionImageTo(Image *img, TransitionInfo const &transitionInfo)
{
    VkImageAspectFlags aspectMask = 0;
    if (transitionInfo.newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL)
    {
        aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    else if (transitionInfo.newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
        aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    else
    {
        aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    VkImageMemoryBarrier imageMemoryBarrier{};
    {
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcAccessMask = transitionInfo.srcAccessMask;
        imageMemoryBarrier.dstAccessMask = transitionInfo.dstAccessMask;
        imageMemoryBarrier.oldLayout = mLayoutTracker.GetImageLayout(img);
        imageMemoryBarrier.newLayout = transitionInfo.newLayout;
        imageMemoryBarrier.image = img->mImage;

        imageMemoryBarrier.subresourceRange = {
            .aspectMask = aspectMask,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };
    }

    /* Do not transition if the layout is already set */
    if (imageMemoryBarrier.oldLayout == imageMemoryBarrier.newLayout)
    {
        return;
    }

    jnrCmdPipelineBarrier(mCommandBuffers[mActiveCommandIndex], transitionInfo.srcStage, transitionInfo.dstStage, 0, 0,
                          nullptr, 0, nullptr, 1, &imageMemoryBarrier);

    mLayoutTracker.TransitionImage(img, transitionInfo.newLayout);

    /*DSHOWINFO("[", (void *)mCommandBuffers[mActiveCommandIndex], "] Transitioning image ", img->mImage, " from layout
       ", string_VkImageLayout(imageMemoryBarrier.oldLayout), " to ",
              string_VkImageLayout(imageMemoryBarrier.newLayout));*/
}

void CommandList::CopyWholeBufferToImage(Image *image, Buffer *buffer)
{
    ThrowIfFailed(image->mExtent2D.width * image->mExtent2D.height == buffer->mCount, "Invalid image");
    ThrowIfFailed(image->mCreateInfo.imageType == VK_IMAGE_TYPE_2D, "Can only copy 2d images");

    VkImageAspectFlags imageAspectFlags = 0;
    for (auto const &[key, value] : image->mImageViews)
    {
        imageAspectFlags |= key;
    }
    VkImageSubresourceLayers imageLayers = {};
    {
        imageLayers.mipLevel = 0;
        imageLayers.layerCount = image->mCreateInfo.arrayLayers;
        imageLayers.baseArrayLayer = 0;
        imageLayers.aspectMask = imageAspectFlags;
    }
    VkExtent3D imageExtent{.width = image->mExtent2D.width, .height = image->mExtent2D.height, .depth = 1};
    VkBufferImageCopy region{};
    {
        region.bufferImageHeight = image->mExtent2D.height;
        region.bufferOffset = 0;
        region.bufferRowLength = image->mExtent2D.width;
        region.imageExtent = imageExtent;
        region.imageOffset = VkOffset3D{.x = 0, .y = 0, .z = 0};
        region.imageSubresource = imageLayers;
    }
    jnrCmdCopyBufferToImage(mCommandBuffers[mActiveCommandIndex], buffer->mBuffer, image->mImage,
                            mLayoutTracker.GetImageLayout(image), 1, &region);
}

u32 CommandList::GetCurrentBackbufferIndex()
{
    ThrowIfFailed(mImageIndex != -1, "Can't retrieve the backbuffer index without beginnig on a backbuffer");
    return mImageIndex;
}

void CommandList::BeginRenderPassOnBackbuffer(RenderPass *rp, std::vector<Framebuffer *> const &fb,
                                              float const *clearColor)
{
    if (mBackbufferAvailable == nullptr)
        mBackbufferAvailable = std::make_unique<GPUSynchronizationObject>();

    auto renderer = Renderer::Get();
    mImageIndex = renderer->AcquireNextImage(mBackbufferAvailable.get());

    VkClearValue clearValue = {};
    memcpy(clearValue.color.float32, clearColor, sizeof(float) * 4);

    auto pClearValue = (VkClearValue *)CopyToTemporaryStorage((u8 *)&clearValue, sizeof(clearValue));
    ThrowIfFailed(pClearValue != nullptr, "Unable to copy clear color value to temporary storage");
    VkRenderPassBeginInfo rpInfo = {};
    {
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpInfo.renderPass = rp->mRenderpass;
        rpInfo.framebuffer = fb[mImageIndex]->mFramebuffer;
        rpInfo.renderArea.offset = {0, 0};
        rpInfo.renderArea.extent = {fb[mImageIndex]->mWidth, fb[mImageIndex]->mHeight};
        rpInfo.clearValueCount = 1;
        rpInfo.pClearValues = pClearValue;
    }
    jnrCmdBeginRenderPass(mCommandBuffers[mActiveCommandIndex], &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandList::EndRenderPass()
{
    jnrCmdEndRenderPass(mCommandBuffers[mActiveCommandIndex]);
}

void CommandList::BeginRenderingOnBackbuffer(float const backgroundColor[4])
{
    if (mBackbufferAvailable == nullptr)
        mBackbufferAvailable = std::make_unique<GPUSynchronizationObject>();

    auto renderer = Renderer::Get();
    mImageIndex = renderer->AcquireNextImage(mBackbufferAvailable.get());
    {
        TransitionInfo ti{};
        {
            ti.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            ti.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ti.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            ti.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        TransitionBackbufferTo(ti);
    }

    VkClearValue clearValue{};
    memcpy(clearValue.color.float32, backgroundColor, sizeof(float) * 4);
    VkRenderingAttachmentInfo colorAttachment{};
    {
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.imageView = renderer->GetSwapchainImageView(mImageIndex);
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue = clearValue;
    }
    VkRenderingInfo renderingInfo{};
    {
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = nullptr;
        renderingInfo.pStencilAttachment = nullptr;
        renderingInfo.renderArea = {.offset = VkOffset2D{.x = 0, .y = 0}, .extent = renderer->GetBackbufferExtent()};
        renderingInfo.viewMask = 0;
        renderingInfo.layerCount = 1;
    }

    jnrCmdBeginRendering(mCommandBuffers[mActiveCommandIndex], &renderingInfo);
}

void CommandList::BeginRenderingOnImage(Image *img, float const backgroundColor[4], Image *depth, bool useStencil)
{
    /* TODO: Maybe batch these transitions? */
    /* Transition the image to color attachment */
    {
        TransitionInfo ti{};
        {
            ti.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            ti.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            ti.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            ti.dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        TransitionImageTo(img, ti);
    }
    if (depth)
    {
        /* Transition image to depth */
        TransitionInfo ti{};
        {
            ti.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            ti.newLayout = useStencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
                                      : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            ti.srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            ti.dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        TransitionImageTo(depth, ti);
    }
    auto colorImageView = img->GetImageView(VK_IMAGE_ASPECT_COLOR_BIT);
    VkRenderingAttachmentInfo colorAttachment{};
    {
        VkClearValue clearValue{};
        memcpy(clearValue.color.float32, backgroundColor, sizeof(float) * 4);

        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.imageView = colorImageView.GetView();
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue = clearValue;
    }
    VkImageAspectFlags depthAspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthAspectFlags |= useStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
    auto depthImageView = depth->GetImageView(depthAspectFlags);
    VkRenderingAttachmentInfo depthAttachment{};
    {
        VkClearValue clearValue{};
        clearValue.depthStencil.depth = 1.0f;
        clearValue.depthStencil.stencil = 0;
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageLayout =
            useStencil ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthAttachment.imageView = depthImageView.GetView();
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.clearValue = clearValue;
    }
    VkRenderingInfo renderingInfo{};
    {
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = depth != nullptr ? &depthAttachment : nullptr;
        renderingInfo.pStencilAttachment = useStencil ? &depthAttachment : nullptr;
        renderingInfo.renderArea = {.offset = VkOffset2D{.x = 0, .y = 0}, .extent = img->GetExtent2D()};
        renderingInfo.viewMask = 0;
        renderingInfo.layerCount = 1;
    }

    jnrCmdBeginRendering(mCommandBuffers[mActiveCommandIndex], &renderingInfo);
}

void CommandList::EndRendering()
{
    jnrCmdEndRendering(mCommandBuffers[mActiveCommandIndex]);
}

void Vulkan::CommandList::AddLocalBuffer(std::unique_ptr<Buffer> &&buffer)
{
    mMemoryTracker.AddBuffer(std::move(buffer));
}

void Vulkan::CommandList::AddLocalImage(std::unique_ptr<Image> &&image)
{
    mMemoryTracker.AddImage(std::move(image));
}

void CommandList::Submit(CPUSynchronizationObject *signalWhenFinished)
{
    auto renderer = Renderer::Get();
    VkSubmitInfo submitInfo{};
    {
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = (u32)mCommandBuffers.size();
        submitInfo.pCommandBuffers = mCommandBuffers.data();
        submitInfo.waitSemaphoreCount = 0;
        submitInfo.pWaitSemaphores = nullptr;
        submitInfo.pWaitDstStageMask = nullptr;
        submitInfo.signalSemaphoreCount = 0;
        submitInfo.pSignalSemaphores = nullptr;
    }

    if (mType == CommandListType::Graphics)
    {
        vkThrowIfFailed(
            jnrQueueSubmit(renderer->mGraphicsQueue, 1, &submitInfo,
                           signalWhenFinished == nullptr ? VK_NULL_HANDLE : signalWhenFinished->GetFence()));
    }
}

void CommandList::SubmitToScreen(CPUSynchronizationObject *signalWhenFinished)
{
    /*DSHOWINFO("Submitting command buffer ", (void *)mCommandBuffers[mActiveCommandIndex], " to the screen");*/
    if (mRenderingFinished == nullptr)
        mRenderingFinished = std::make_unique<GPUSynchronizationObject>();

    auto renderer = Renderer::Get();
    {
        /* Submit the command buffers */
        VkSemaphore waitSemaphores[] = {mBackbufferAvailable->GetSemaphore()};
        VkSemaphore signalSemaphores[] = {mRenderingFinished->GetSemaphore()};

        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo{};
        {
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = (u32)mCommandBuffers.size();
            submitInfo.pCommandBuffers = mCommandBuffers.data();
            submitInfo.waitSemaphoreCount = ARRAYSIZE(waitSemaphores);
            submitInfo.pWaitSemaphores = waitSemaphores;
            submitInfo.pWaitDstStageMask = waitStages;
            submitInfo.signalSemaphoreCount = ARRAYSIZE(signalSemaphores);
            submitInfo.pSignalSemaphores = signalSemaphores;
        }

        if (mType == CommandListType::Graphics)
        {
            vkThrowIfFailed(
                jnrQueueSubmit(renderer->mGraphicsQueue, 1, &submitInfo,
                               signalWhenFinished == nullptr ? VK_NULL_HANDLE : signalWhenFinished->GetFence()));
        }
    }

    {
        VkSemaphore waitSemaphores[] = {mRenderingFinished->GetSemaphore()};
        VkSwapchainKHR swapchains[] = {renderer->mSwapchain};
        u32 imageIndices[] = {(u32)mImageIndex};

        VkPresentInfoKHR presentInfo{};
        {
            presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
            presentInfo.pWaitSemaphores = waitSemaphores;
            presentInfo.waitSemaphoreCount = ARRAYSIZE(waitSemaphores);
            presentInfo.swapchainCount = ARRAYSIZE(swapchains);
            presentInfo.pSwapchains = swapchains;
            presentInfo.pImageIndices = imageIndices;
        }

        vkThrowIfFailed(jnrQueuePresentKHR(renderer->mPresentQueue, &presentInfo));
    }
}

void CommandList::SubmitAndWait()
{
    CPUSynchronizationObject cpuWait;
    Submit(&cpuWait);
    cpuWait.Wait();
}

void *CommandList::CopyToTemporaryStorage(u8 *data, u8 dataSize)
{
    if (mTemporaryStorageOffset + dataSize < TEMPORARY_STORAGE_SIZE)
    {
        memcpy(mTemporaryStorage + mTemporaryStorageOffset, data, dataSize);
        void *returnData = mTemporaryStorage + mTemporaryStorageOffset;
        mTemporaryStorageOffset += dataSize;

        return returnData;
    }
    return nullptr;
}

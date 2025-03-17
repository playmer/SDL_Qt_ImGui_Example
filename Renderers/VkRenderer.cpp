#define VMA_IMPLEMENTATION
#define VMA_STATS_STRING_ENABLED 0
#include "vk_mem_alloc.h"

#include "SDL3/SDL_vulkan.h"

#include "Renderers/Renderer.hpp"

#include "Renderers/VkRenderer.hpp"

std::unique_ptr<Renderer> CreateVkRenderer(SDL_Window* aWindow)
{
    return std::unique_ptr<Renderer>(new VkRenderer(aWindow));
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Helpers:
static void check_vk_result(VkResult err)
{    
    if (err == 0)
    {
        return;
    }

    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);

    if (err < 0)
    {
        abort();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// VulkanCommandBuffer:
void VulkanCommandBuffer::Begin()
{
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    auto err = vkBeginCommandBuffer(mBuffer, &begin_info);
    check_vk_result(err);
}

void VulkanCommandBuffer::End()
{
    auto err = vkEndCommandBuffer(mBuffer);
    check_vk_result(err);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// VulkanQueue:
VulkanQueue::VulkanQueue()
{
}

void VulkanQueue::Initialize(vkb::Device aDevice, vkb::QueueType aType, size_t aNumberOfBuffers)
{
    mDevice = aDevice;
    mCommandBuffers.resize(aNumberOfBuffers, VK_NULL_HANDLE);
    mFences.resize(aNumberOfBuffers, VK_NULL_HANDLE);
    mAvailableSemaphores.resize(aNumberOfBuffers, VK_NULL_HANDLE);
    mFinishedSemaphore.resize(aNumberOfBuffers, VK_NULL_HANDLE);
    mUsed.resize(aNumberOfBuffers, false);
    mType = aType;

    auto queue_ret = mDevice.get_queue(mType);
    if (!queue_ret)
    {
        printf("Failed to create Vulkan Queue. Error: %s\n", queue_ret.error().message().c_str());

        if (vkb::QueueType::transfer == mType)
        {
            // Okay try one more time for a graphics queue. We can use that as a fallback.
            mType = vkb::QueueType::graphics;
            auto alternate_queue_ret = mDevice.get_queue(mType);

            if (!alternate_queue_ret)
            {
                printf("Failed to create Vulkan Queue. Error: %s\n", alternate_queue_ret.error().message().c_str());
                return;
            }
            else
            {
                mQueue = alternate_queue_ret.value();
            }
        }
    }
    else
    {
        mQueue = queue_ret.value();
    }

    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = mDevice.get_queue_index(mType).value();
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    mPool = VK_NULL_HANDLE;
    if (vkCreateCommandPool(mDevice.device, &pool_info, mDevice.allocation_callbacks, &mPool) != VK_SUCCESS)
    {
        printf("failed to create command pool\n");
        return;
    }

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = mPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();

    if (vkAllocateCommandBuffers(mDevice.device, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
    {
        printf("Failed to allocate command buffers\n");
        return;
    }

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto& fence : mFences)
    {
        if (vkCreateFence(mDevice.device, &fence_info, nullptr, &fence) != VK_SUCCESS) {
        printf("Failed to create synchronization objects\n");
        return;
        }
    }

    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (auto& semaphore : mAvailableSemaphores)
    {
        if (vkCreateSemaphore(mDevice.device, &semaphore_info, nullptr, &semaphore) != VK_SUCCESS)
        {
            printf("Failed to create synchronization objects\n");
            return;
        }
    }

    for (auto& semaphore : mFinishedSemaphore)
    {
        if (vkCreateSemaphore(mDevice.device, &semaphore_info, nullptr, &semaphore) != VK_SUCCESS)
        {
            printf("Failed to create synchronization objects\n");
            return;
        }
    }
}


uint32_t VulkanQueue::GetQueueFamily()
{
    return mDevice.get_queue_index(mType).value();
}


VulkanCommandBuffer VulkanQueue::WaitOnNextCommandList()
{
    mCurrentBuffer = (1 + mCurrentBuffer) % mCommandBuffers.size();

    if (mUsed[mCurrentBuffer])
    {
        vkWaitForFences(mDevice.device, 1, &mFences[mCurrentBuffer], true, UINT64_MAX);
        vkResetFences(mDevice, 1, &mFences[mCurrentBuffer]);
        vkResetCommandBuffer(mCommandBuffers[mCurrentBuffer], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }
    else
    {
        vkResetFences(mDevice, 1, &mFences[mCurrentBuffer]);
        mUsed[mCurrentBuffer] = true;
    }

    return VulkanCommandBuffer{ mCommandBuffers[mCurrentBuffer], mFences[mCurrentBuffer], mAvailableSemaphores[mCurrentBuffer], mFinishedSemaphore[mCurrentBuffer] };
}


VulkanCommandBuffer VulkanQueue::GetNextCommandList()
{
    mCurrentBuffer = (1 + mCurrentBuffer) % mCommandBuffers.size();

    if (mUsed[mCurrentBuffer])
    {
        vkResetCommandBuffer(mCommandBuffers[mCurrentBuffer], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
    }
    else
    {
        mUsed[mCurrentBuffer] = true;
    }

    return VulkanCommandBuffer{ mCommandBuffers[mCurrentBuffer], mFences[mCurrentBuffer], mAvailableSemaphores[mCurrentBuffer], mFinishedSemaphore[mCurrentBuffer]};
}


VulkanCommandBuffer VulkanQueue::GetCurrentCommandList()
{
    return VulkanCommandBuffer{ mCommandBuffers[mCurrentBuffer], mFences[mCurrentBuffer], mAvailableSemaphores[mCurrentBuffer], mFinishedSemaphore[mCurrentBuffer] };
}

void VulkanQueue::Submit(VulkanCommandBuffer aCommandList)
{
    VkSubmitInfo end_info = {};
    end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    end_info.commandBufferCount = 1;
    end_info.pCommandBuffers = &aCommandList.mBuffer;
    auto err = vkQueueSubmit(mQueue, 1, &end_info, aCommandList.mFence);
    check_vk_result(err);
}

VkBool32 
#ifdef SYSTEM_ANDROID
__attribute__((pcs("aapcs-vfp")))
#endif
DebugUtilsCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* /*pUserData*/)
{
    auto severity = vkb::to_string_message_severity(messageSeverity);
    auto type = vkb::to_string_message_type(messageType);

    if ((VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT & messageSeverity)
        || (VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT & messageSeverity))
    {
        printf("[%s: %s] %s\n", severity, type, pCallbackData->pMessage);
    }

    return VK_FALSE;
}

VkRenderPass VkRenderer::CreateRenderPass()
{
    VkRenderPass renderPass;
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = mSwapchain.image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;// | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(mDevice, &render_pass_info, nullptr, &renderPass) != VK_SUCCESS)
    {
        printf("failed to create render pass\n");
        return VK_NULL_HANDLE;;
    }

    return renderPass;
}

VkRenderer::VkRenderer(SDL_Window* aWindow)
    : Renderer{ aWindow }
{
    ///////////////////////////////////////
    // Create Instance
    vkb::InstanceBuilder instance_builder;
    instance_builder
        .set_app_name("Application")
        .set_engine_name("SOIS")
        .require_api_version(1, 0, 0)
        .set_debug_callback(&DebugUtilsCallback);

    auto system_info_ret = vkb::SystemInfo::get_system_info();
    if (!system_info_ret)
    {
        printf("%s\n", system_info_ret.error().message().c_str());
        return;
    }

    auto system_info = system_info_ret.value();
    if (system_info.validation_layers_available) {
        instance_builder.enable_validation_layers();
    }

    Uint32 sdl_extensions_count;
    const char* const* sdl_extensions = SDL_Vulkan_GetInstanceExtensions(&sdl_extensions_count);
    if (nullptr == sdl_extensions)
    {
        printf("Failed to get required Vulkan Instance Extensions from SDL\n");
        return;
    }

    const char* const* extensions_end = sdl_extensions + sdl_extensions_count;
    for (sdl_extensions; sdl_extensions < extensions_end; ++sdl_extensions)
    {
        instance_builder.enable_extension(*sdl_extensions);
    }

    auto instance_builder_return = instance_builder.build();

    if (!instance_builder_return) {
        printf("Failed to create Vulkan instance. Error: %s\n", instance_builder_return.error().message().c_str());
        return;
    }
    mInstance = instance_builder_return.value();

    ///////////////////////////////////////
    // Create Surface
    {
        if (!SDL_Vulkan_CreateSurface(aWindow, mInstance.instance, nullptr, &mSurface)) {
            printf("Failed to create Vulkan Surface.\n");
            return;
        }
    }

    ///////////////////////////////////////
    // Select Physical Device
    vkb::PhysicalDeviceSelector phys_device_selector(mInstance);
    phys_device_selector.set_surface(mSurface);

    {
        auto physical_device_selector_return = phys_device_selector.select();

        if (!physical_device_selector_return) 
        {
            // We return out because there's really nothing we can do at this point, there's not a 
            // single suitable GPU on this system.
            printf("Failed to select Vulkan Physical Device. Error: %s\n", physical_device_selector_return.error().message().c_str());
            return;
        }
        else
        {
            mPhysicalDevice = physical_device_selector_return.value();
        }
    }

    ///////////////////////////////////////
    // Create Logical Device
    vkb::DeviceBuilder device_builder{ mPhysicalDevice };
    auto dev_ret = device_builder.build();
    if (!dev_ret) {
        printf("Failed to create Logical Device. Error: %s\n", dev_ret.error().message().c_str());
    }
    mDevice = dev_ret.value();

    ///////////////////////////////////////
    // Create Queues
    mTransferQueue.Initialize(mDevice, vkb::QueueType::transfer, 30);
    mGraphicsQueue.Initialize(mDevice, vkb::QueueType::graphics, cMinImageCount);
    mPresentQueue.Initialize(mDevice, vkb::QueueType::present, cMinImageCount);

    ///////////////////////////////////////
    // Create Swapchain
    vkb::SwapchainBuilder swapchain_builder{ mDevice, mSurface };
    swapchain_builder.set_desired_format(VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR });
    auto swap_ret = swapchain_builder.build();
    if (!swap_ret) 
    {
        printf("Failed to create Vulkan Swapchain. Error: %s\n", swap_ret.error().message().c_str());
    }
    mSwapchain = swap_ret.value();

    ///////////////////////////////////////
    // Create Descriptor Pool
    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * std::size(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        VkResult err = vkCreateDescriptorPool(mDevice, &pool_info, mInstance.allocation_callbacks, &mDescriptorPool);
        check_vk_result(err);
    }

    ///////////////////////////////////////
    // Create Font Sampler:
    {
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.minLod = -1000;
        info.maxLod = 1000;
        info.maxAnisotropy = 1.0f;
        VkResult err = vkCreateSampler(mDevice.device, &info, NULL, &mFontSampler);
        check_vk_result(err);
    }

    ///////////////////////////////////////
    // Create Descriptor Set Layout:
    {
        VkSampler sampler[1] = { mFontSampler };
        VkDescriptorSetLayoutBinding binding[1] = {};
        binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding[0].descriptorCount = 1;
        binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding[0].pImmutableSamplers = sampler;
        VkDescriptorSetLayoutCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = 1;
        info.pBindings = binding;
        VkResult err = vkCreateDescriptorSetLayout(mDevice.device, &info, NULL, &mDescriptorSetLayout);
        check_vk_result(err);
    }

    ///////////////////////////////////////
    // Create Allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorInfo.physicalDevice = mPhysicalDevice;
    allocatorInfo.device = mDevice;
    allocatorInfo.instance = mInstance;

    vmaCreateAllocator(&allocatorInfo, &mAllocator);
    printf("Create allocator %p\n", mAllocator);

    ///////////////////////////////////////
    // Create Render Pass
    mRenderPass = CreateRenderPass();

    ///////////////////////////////////////
    // Create Framebuffers
    int w, h;
    SDL_GetWindowSize(aWindow, &w, &h);

    swapchain_images = mSwapchain.get_images().value();
    swapchain_image_views = mSwapchain.get_image_views().value();

    mFramebuffers.resize(swapchain_image_views.size());

    for (size_t i = 0; i < swapchain_image_views.size(); i++) 
    {
        VkImageView attachments[] = { swapchain_image_views[i] };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = mRenderPass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = mSwapchain.extent.width;
        framebuffer_info.height = mSwapchain.extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(mDevice, &framebuffer_info, nullptr, &mFramebuffers[i]) != VK_SUCCESS)
        {
            printf("failed to create framebuffer\n");
            return;
        }
    }
}

void VkRenderer::Initialize()
{

}

void VkRenderer::Update()
{
    auto vulkanCommandBuffer = mGraphicsQueue.WaitOnNextCommandList();
    auto [commandBuffer, fence, waitSemaphore, signalSemphore] = vulkanCommandBuffer;

    // Wait on last frame/get next frame now, just in case we need to load the font textures.
    VkResult result = vkAcquireNextImageKHR(mDevice,
      mSwapchain,
      UINT64_MAX,
      waitSemaphore,
      VK_NULL_HANDLE,
      &mImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        return Resize(0, 0);
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        printf("failed to acquire swapchain image.\n");
        return;
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);


    mClearColor;

    VkClearColorValue color;
    color.float32[0] = mClearColor.r / 255.f;
    color.float32[1] = mClearColor.g / 255.f;
    color.float32[2] = mClearColor.b / 255.f;
    color.float32[3] = mClearColor.a / 255.f;

    VkClearValue clearColor{ { color } };
    VkRenderPassBeginInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    info.renderPass = mRenderPass;
    info.framebuffer = mFramebuffers[mImageIndex];
    info.renderArea.extent = mSwapchain.extent;
    info.clearValueCount = 1;
    info.pClearValues = &clearColor;
    vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);























    vkCmdEndRenderPass(commandBuffer);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = { waitSemaphore };
    VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = wait_semaphores;
    submitInfo.pWaitDstStageMask = wait_stages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signal_semaphores[] = { signalSemphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signal_semaphores;

    vkResetFences(mDevice, 1, &fence);

    if (vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, fence) != VK_SUCCESS)
    {
        printf("failed to submit draw command buffer\n");
        return;
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapChains[] = { mSwapchain };
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapChains;

    present_info.pImageIndices = &mImageIndex;

    result = vkQueuePresentKHR(mPresentQueue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        return Resize(0, 0);
    }
    else if (result != VK_SUCCESS)
    {
        printf("failed to present swapchain image\n");
        return;
    }

    mCurrentFrame = (mCurrentFrame + 1) % cMinImageCount;
}

void VkRenderer::Resize(unsigned int aWidth, unsigned int aHeight)
{
    vkb::SwapchainBuilder swapchain_builder{ mDevice };
    swapchain_builder.set_desired_format(VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR });
    auto swap_ret = swapchain_builder.set_old_swapchain(mSwapchain).build();
    if (!swap_ret)
    {
          // If it failed to create a swapchain, the old swapchain handle is invalid.
          mSwapchain.swapchain = VK_NULL_HANDLE;
    }

    // Even though we recycled the previous swapchain, we need to free its resources.
    vkb::destroy_swapchain(mSwapchain);

    // If we get this, we might be screwed, but we also might just be closing the app,
    // so lets just return and hope we're closing. Yes I should handle this better.
    if (!swap_ret.has_value() && swap_ret.vk_result() == VK_ERROR_SURFACE_LOST_KHR)
    {
        return;
    }

    // Get the new swapchain and place it in our variable
    mSwapchain = swap_ret.value();


    for (auto framebuffer : mFramebuffers)
    {
        vkDestroyFramebuffer(mDevice.device, framebuffer, mDevice.allocation_callbacks);
    }

    swapchain_images = mSwapchain.get_images().value();
    swapchain_image_views = mSwapchain.get_image_views().value();

    mFramebuffers.resize(swapchain_image_views.size());

    for (size_t i = 0; i < swapchain_image_views.size(); i++)
    {
        VkImageView attachments[] = { swapchain_image_views[i] };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = mRenderPass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = mSwapchain.extent.width;
        framebuffer_info.height = mSwapchain.extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(mDevice, &framebuffer_info, nullptr, &mFramebuffers[i]) != VK_SUCCESS)
        {
            printf("failed to create framebuffer\n");
            return;
        }
    }
}

#pragma once

#include "vulkan/vulkan.h"

#include "vk_mem_alloc.h"

#include "VkBootstrap.h"

#include "SDL3/SDL.h"

#include "Renderers/Renderer.hpp"


struct VulkanCommandBuffer
{
	auto operator->() const
	{
		return &mBuffer;
	}

	auto& operator&()
	{
		return mBuffer;
	}

	auto* operator&() const
	{
		return &mBuffer;
	}

	operator VkCommandBuffer()
	{
		return mBuffer;
	}

	void Begin();
	void End();

	VkCommandBuffer mBuffer;
	VkFence mFence;
	VkSemaphore mAvailableSemaphore;
	VkSemaphore mFinishedSemaphore;
};

class VulkanQueue
{
public:
	VulkanQueue();
	
	void Initialize(vkb::Device aDevice, vkb::QueueType aType, size_t aNumberOfBuffers);

	VulkanCommandBuffer WaitOnNextCommandList();
	VulkanCommandBuffer GetNextCommandList();
	VulkanCommandBuffer GetCurrentCommandList();

	auto* operator&()
	{
		return &mQueue;
	}

	auto* operator&() const
	{
		return &mQueue;
	}

	operator VkQueue()
	{
		return mQueue;
	}

	bool IsInitialized()
	{
		return (VK_NULL_HANDLE != mPool)
			&& (VK_NULL_HANDLE != mCommandBuffers.back())
			&& (VK_NULL_HANDLE != mFences.back())
			&& (VK_NULL_HANDLE != mAvailableSemaphores.back())
			&& (VK_NULL_HANDLE != mFinishedSemaphore.back());
	}

	uint32_t GetQueueFamily();

	void Submit(VulkanCommandBuffer aCommandList);

private:
	vkb::Device mDevice;
	VkQueue mQueue = VK_NULL_HANDLE;
	VkCommandPool mPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> mCommandBuffers;
	std::vector<VkFence> mFences;
	std::vector<VkSemaphore> mAvailableSemaphores;
	std::vector<VkSemaphore> mFinishedSemaphore;
	std::vector<bool> mUsed;
	size_t mCurrentBuffer = 0;
	vkb::QueueType mType;
};

class VkRenderer : public Renderer
{
public:
	VkRenderer(SDL_Window* aWindow);

	void Initialize() override;
	void Update() override;
	void Resize(unsigned int aWidth, unsigned int aHeight) override;
    virtual const char* Name() override { return "VkRenderer"; };

private:
	VkRenderPass CreateRenderPass();

    static constexpr uint32_t cMinImageCount = 3;

    vkb::Instance mInstance;
    VkSurfaceKHR mSurface;
    vkb::PhysicalDevice mPhysicalDevice;
    vkb::Device mDevice;
    VkPipelineCache mPipelineCache = VK_NULL_HANDLE;
    VulkanQueue mTransferQueue;
    VulkanQueue mGraphicsQueue;
    VulkanQueue mPresentQueue;
    vkb::Swapchain mSwapchain;
    VkDescriptorPool mDescriptorPool;

    VmaAllocator mAllocator;

    VkSampler mFontSampler;
    VkDescriptorSetLayout mDescriptorSetLayout;

    std::vector<VkImage> swapchain_images;
    std::vector<VkImageView> swapchain_image_views;
    std::vector<VkFramebuffer> mFramebuffers;

    VkRenderPass mRenderPass;

    size_t mCurrentFrame = 0;
    uint32_t mImageIndex = 0;


    bool mLoadedFontTexture = false;
};

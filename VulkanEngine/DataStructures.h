#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "pch.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

struct MainDevice {
	VkPhysicalDevice PhysicalDevice;
	VkDevice		 LogicalDevice;
	VkDeviceSize	 MinUniformBufferOffset;
};

struct VulkanRenderData {
	VkInstance			instance;

	MainDevice			main_device;
	VkPhysicalDevice	physical_device;
	VkDevice			device;

	uint32_t			graphic_queue_index;
	VkQueue				graphic_queue;

	uint32_t			min_image_count;
	uint32_t			image_count;

	VkDescriptorPool	imgui_descriptor_pool;
	VkDescriptorPool	texture_descriptor_pool;
	VkDescriptorSetLayout texture_descriptor_layout;

	VkCommandPool		command_pool;
	
	std::vector<VkCommandBuffer> command_buffers;

	VkRenderPass		render_pass;
};

struct ImageInfo {
	uint32_t				width;
	uint32_t				height;
	VkFormat				format;
	VkImageTiling			tiling;
	VkImageUsageFlags		usage;
	VkMemoryPropertyFlags	properties;
};

struct BufferSettings {
	VkDeviceSize			size;
	VkBufferUsageFlags		usage;
	VkMemoryAllocateFlags	properties;
};

struct TextureObjects {
	std::vector<VkImage>		 TextureImages;
	std::vector<VkDeviceMemory>  TextureImageMemory;
	std::vector<VkImageView>	 TextureImageViews;
	std::vector<VkDescriptorSet> SamplerDescriptorSets;

	VkSampler					 TextureSampler = {};

	void CreateSampler(MainDevice &mainDevice)
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType						= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter					= VK_FILTER_LINEAR;		// linear interpolation between the texels
		samplerCreateInfo.minFilter					= VK_FILTER_LINEAR;		// quando viene miniaturizzata come renderizzarla (lerp)
		samplerCreateInfo.addressModeU				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerCreateInfo.unnormalizedCoordinates	= VK_FALSE;			// � normalizzata
		samplerCreateInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.mipLodBias				= 0.0f;
		samplerCreateInfo.minLod					= 0.0f;
		samplerCreateInfo.maxLod					= 0.0f;
		samplerCreateInfo.anisotropyEnable			= VK_TRUE;
		samplerCreateInfo.maxAnisotropy				= 16;

		VkResult result = vkCreateSampler(mainDevice.LogicalDevice, &samplerCreateInfo, nullptr, &TextureSampler);

		if (result != VK_SUCCESS)
			throw std::runtime_error("Failed to create a Texture Sampler!");
	}
};

struct ViewProjectionData {
	glm::mat4 proj;
	glm::mat4 view;
};

struct SettingsData {
	int render_target;
};

struct BufferImage {
	VkImage			Image		= {};
	VkFormat		Format		= {};
	VkDeviceMemory  Memory		= {};
	VkImageView		ImageView	= {};
	VkSampler		Sampler		= {};

	void DestroyAndFree(MainDevice &mainDevice)
	{
		vkDestroySampler(mainDevice.LogicalDevice, Sampler, nullptr);
		vkDestroyImageView(mainDevice.LogicalDevice, ImageView, nullptr);
		vkDestroyImage(mainDevice.LogicalDevice, Image, nullptr);
		vkFreeMemory(mainDevice.LogicalDevice, Memory, nullptr);
	}
};

struct SubmissionSyncObjects {
	VkSemaphore OffScreenAvailable;
	VkSemaphore ImageAvailable; // Avvisa quanto l'immagine � disponibile
	VkSemaphore RenderFinished; // Avvisa quando il rendering � terminato
	VkFence		InFlight;		// Fence per il frame in esecuzione
};

struct QueueFamilyIndices {

	uint32_t GraphicsFamily		= UINT_MAX;
	uint32_t PresentationFamily = UINT_MAX;

	bool isValid() const
	{
		return GraphicsFamily >= 0 && PresentationFamily >= 0;
	}
};

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 col;
	glm::vec3 nrm; // normal
	glm::vec2 tex;
};
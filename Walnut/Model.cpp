#include "Model.h"
#include "imgui_impl_vulkan.h"

#include "Application.h"
#include "lve_utils.hpp"

#include "stb_image.h"
#include "tiny_obj_loader.h"
#include <glm/gtx/hash.hpp>

#ifndef ENGINE_DIR
#	define ENGINE_DIR "./"
#endif

namespace std
{
template <>
struct hash<Walnut::Vertex>
{
	size_t operator()(Walnut::Vertex const &vertex) const
	{
		size_t seed = 0;
		lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
		return seed;
	}
};
}        // namespace std

namespace Walnut
{

namespace Utils
{

static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(Application::GetPhysicalDevice(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

static uint32_t GetVulkanMemoryType(VkMemoryPropertyFlags properties, uint32_t type_bits)
{
	VkPhysicalDeviceMemoryProperties prop;
	vkGetPhysicalDeviceMemoryProperties(Application::GetPhysicalDevice(), &prop);
	for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
	{
		if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
			return i;
	}

	return 0xffffffff;
}

static uint32_t BytesPerPixel(ImageFormat format)
{
	switch (format)
	{
		case ImageFormat::RGBA:
			return 4;
		case ImageFormat::RGBA32F:
			return 16;
	}
	return 0;
}

static VkFormat WalnutFormatToVulkanFormat(ImageFormat format)
{
	switch (format)
	{
		case ImageFormat::RGBA:
			return VK_FORMAT_R8G8B8A8_UNORM;
		case ImageFormat::RGBA32F:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	return (VkFormat) 0;
}

}        // namespace Utils

Model::Model(const std::string &modelpath, const std::string &texturepath) :
    m_ModelFilepath(modelpath), m_TextureFilepath(texturepath)
{
	std::cout << "[1] Loading models from " << m_ModelFilepath << std::endl;
	m_Builder = std::make_unique<Builder>();
	m_Builder->loadModel(ENGINE_DIR + m_ModelFilepath);
	std::cout << "vertex size:" << m_Builder->vertices.size() << std::endl;
	std::cout << "indices size:" << m_Builder->indices.size() << std::endl;

	std::cout << "[2] Loading texture images from" << m_TextureFilepath << std::endl;
	int width, height, channels;
	if (stbi_is_hdr(m_TextureFilepath.c_str()))
	{
		m_ImageData = (uint8_t *) stbi_loadf(m_TextureFilepath.c_str(), &width, &height, &channels, 4);
		m_Format    = ImageFormat::RGBA32F;
	}
	else
	{
		m_ImageData = stbi_load(m_TextureFilepath.c_str(), &width, &height, &channels, 4);
		m_Format    = ImageFormat::RGBA;
	}

	m_MipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, height)))) + 1;
	m_Width     = width;
	m_Height    = height;

	AllocateTextureMemory(m_Width * m_Height * Utils::BytesPerPixel(m_Format));
	SetTextureData(m_ImageData);
	std::cout << "Texture images descriptorSet: " << m_DescriptorSet << std::endl;
}

Model::Model(uint32_t width, uint32_t height, Walnut::ImageFormat format, const void *data) :
    m_Width(width), m_Height(height), m_Format(format)

{
	size_t upload_size = m_Width * m_Height * Utils::BytesPerPixel(m_Format);
	AllocateTextureMemory(upload_size);
	if (data)
		SetTextureData(data);
	std::cout << "Texture images descriptorSet: " << m_DescriptorSet << std::endl;
}

Model::~Model()
{
	Release();
}

void Walnut::Builder::loadModel(const std::string &filepath)
{
	// std::cout << "Loading Model in builder" << std::endl;
	std::cout << filepath << std::endl;
	tinyobj::attrib_t                attrib;
	std::vector<tinyobj::shape_t>    shapes;
	std::vector<tinyobj::material_t> materials;
	std::string                      warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str()))
	{
		// std::cout << "Loading Model failed for loadObj " << std::endl;
		throw std::runtime_error(warn + err);
	}

	vertices.clear();
	indices.clear();

	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	for (const auto &shape : shapes)
	{
		for (const auto &index : shape.mesh.indices)
		{
			Vertex vertex{};

			if (index.vertex_index >= 0)
			{
				vertex.position = {
				    attrib.vertices[3 * index.vertex_index + 0],
				    attrib.vertices[3 * index.vertex_index + 1],
				    attrib.vertices[3 * index.vertex_index + 2],
				};

				vertex.color = {
				    attrib.colors[3 * index.vertex_index + 0],
				    attrib.colors[3 * index.vertex_index + 1],
				    attrib.colors[3 * index.vertex_index + 2],
				};
			}

			if (index.normal_index >= 0)
			{
				vertex.normal = {
				    attrib.normals[3 * index.normal_index + 0],
				    attrib.normals[3 * index.normal_index + 1],
				    attrib.normals[3 * index.normal_index + 2],
				};
			}

			if (index.texcoord_index >= 0)
			{
				vertex.uv = {
				    attrib.texcoords[2 * index.texcoord_index + 0],
				    attrib.texcoords[2 * index.texcoord_index + 1],
				};
			}

			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

// NOTE: for image texture
void Model::AllocateTextureMemory(uint64_t size)
{
	VkDevice device = Application::GetDevice();

	VkResult err;

	VkFormat vulkanFormat = Utils::WalnutFormatToVulkanFormat(m_Format);

	// Create the Image texture
	{
		// std::cout << "Image creation" << std::endl;
		VkImageCreateInfo info = {};
		info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.imageType         = VK_IMAGE_TYPE_2D;
		info.format            = vulkanFormat;
		info.extent.width      = m_Width;
		info.extent.height     = m_Height;
		info.extent.depth      = 1;
		info.mipLevels         = 1;
		info.mipLevels         = m_MipLevels;
		info.arrayLayers       = 1;
		info.samples           = VK_SAMPLE_COUNT_1_BIT;
		info.tiling            = VK_IMAGE_TILING_OPTIMAL;
		info.usage             = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		info.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
		info.initialLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		err                    = vkCreateImage(device, &info, nullptr, &m_TextureImage);
		check_vk_result(err);
		// std::cout << "Done Image creation" << std::endl;
		VkMemoryRequirements req;
		vkGetImageMemoryRequirements(device, m_TextureImage, &req);
		VkMemoryAllocateInfo alloc_info = {};
		alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize       = req.size;
		alloc_info.memoryTypeIndex      = Utils::GetVulkanMemoryType(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, req.memoryTypeBits);
		// std::cout << "Allocate Image memory" << std::endl;
		err = vkAllocateMemory(device, &alloc_info, nullptr, &m_TextureImageMemory);
		check_vk_result(err);
		// std::cout << "Done allocate Image memory" << std::endl;
		err = vkBindImageMemory(device, m_TextureImage, m_TextureImageMemory, 0);
		check_vk_result(err);
		// std::cout << "Don Binding Image memory" << std::endl;
	}

	// Create the Image View:
	{
		// std::cout << "Create Image View" << std::endl;
		VkImageViewCreateInfo info       = {};
		info.sType                       = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.image                       = m_TextureImage;
		info.viewType                    = VK_IMAGE_VIEW_TYPE_2D;
		info.format                      = vulkanFormat;
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.layerCount = 1;
		// info.subresourceRange.layerCount = m_MipLevels;
		err = vkCreateImageView(device, &info, nullptr, &m_TextureImageView);
		check_vk_result(err);
	}

	// Create sampler:
	{
		// std::cout << "Create sampler" << std::endl;
		VkSamplerCreateInfo info = {};
		info.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		info.magFilter           = VK_FILTER_LINEAR;
		info.minFilter           = VK_FILTER_LINEAR;
		info.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		info.addressModeU        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeV        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.addressModeW        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		info.minLod              = -1000;
		info.maxLod              = 1000;
		info.maxAnisotropy       = 1.0f;
		VkResult err             = vkCreateSampler(device, &info, nullptr, &m_TextureSampler);
		check_vk_result(err);
	}

	// Create the Descriptor Set:
	// std::cout << "Create Description set" << std::endl;
	m_DescriptorSet = (VkDescriptorSet) ImGui_ImplVulkan_AddTexture(m_TextureSampler, m_TextureImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Model::SetTextureData(const void *data)
{
	VkDevice device = Application::GetDevice();

	size_t upload_size = m_Width * m_Height * Utils::BytesPerPixel(m_Format);

	VkResult err;

	if (!m_StagingBuffer)
	{
		// Create the Upload Buffer
		// std::cout << "No stagebuffer, creating new one!" << std::endl;
		{
			VkBufferCreateInfo buffer_info = {};
			buffer_info.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffer_info.size               = upload_size;
			buffer_info.usage              = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			buffer_info.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;
			err                            = vkCreateBuffer(device, &buffer_info, nullptr, &m_StagingBuffer);
			check_vk_result(err);
			// std::cout << "Succeed to create stagebuffer" << std::endl;
			VkMemoryRequirements req;
			vkGetBufferMemoryRequirements(device, m_StagingBuffer, &req);
			m_AlignedSize                   = req.size;
			VkMemoryAllocateInfo alloc_info = {};
			alloc_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			alloc_info.allocationSize       = req.size;
			alloc_info.memoryTypeIndex      = Utils::GetVulkanMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, req.memoryTypeBits);
			err                             = vkAllocateMemory(device, &alloc_info, nullptr, &m_StagingBufferMemory);
			check_vk_result(err);
			// std::cout << "Succeed to allocate memory " << std::endl;
			err = vkBindBufferMemory(device, m_StagingBuffer, m_StagingBufferMemory, 0);
			check_vk_result(err);
			// std::cout << "Succeed to bind memory " << std::endl;
		}
	}

	// Upload to Buffer
	{
		char *map = NULL;
		err       = vkMapMemory(device, m_StagingBufferMemory, 0, m_AlignedSize, 0, (void **) (&map));
		check_vk_result(err);
		// std::cout << "Succeed to map memory " << std::endl;
		memcpy(map, data, upload_size);
		VkMappedMemoryRange range[1] = {};
		range[0].sType               = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range[0].memory              = m_StagingBufferMemory;
		range[0].size                = m_AlignedSize;
		err                          = vkFlushMappedMemoryRanges(device, 1, range);
		check_vk_result(err);
		// std::cout << "Succeed to copy memory " << std::endl;
		vkUnmapMemory(device, m_StagingBufferMemory);
	}
}

void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
	auto               device = Application::GetDevice();
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size        = size;
	bufferInfo.usage       = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize  = memRequirements.size;
	allocInfo.memoryTypeIndex = Utils::GetVulkanMemoryType(properties, memRequirements.memoryTypeBits);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = Application::GetCommandBuffer(true);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
}

void Model::CreateVertexBuffer()
{
	auto         device     = Application::GetDevice();
	VkDeviceSize bufferSize = sizeof(m_Builder->vertices[0]) * m_Builder->vertices.size();

	VkBuffer       stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void *data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_Builder->vertices.data(), (size_t) bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_VertexBuffer, m_VertexBufferMemory);

	copyBuffer(stagingBuffer, *m_VertexBuffer, bufferSize);
}

void Model::CreateIndexBuffer()
{
	auto         device     = Application::GetDevice();
	VkDeviceSize bufferSize = sizeof(m_Builder->indices[0]) * m_Builder->indices.size();

	VkBuffer       stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void *data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, m_Builder->indices.data(), (size_t) bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *m_IndexBuffer, m_IndexBufferMemory);

	copyBuffer(stagingBuffer, *m_IndexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void createUniformBuffers()
{
	// VkDeviceSize bufferSize = sizeof(UniformBufferObject);
	//
	// uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	// uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
	// uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
	//
	// for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	// {
	// 	createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
	//
	// 	vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
	// }
}

void Model::Release()
{
	Application::SubmitResourceFree([sampler = m_TextureSampler, imageView = m_TextureImageView, image = m_TextureImage, memory = m_TextureImageMemory, stagingBuffer = m_StagingBuffer, stagingBufferMemory = m_StagingBufferMemory]() {
		VkDevice device = Application::GetDevice();

		vkDestroySampler(device, sampler, nullptr);
		vkDestroyImageView(device, imageView, nullptr);
		vkDestroyImage(device, image, nullptr);
		vkFreeMemory(device, memory, nullptr);
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	});

	m_TextureSampler      = nullptr;
	m_TextureImageView    = nullptr;
	m_TextureImage        = nullptr;
	m_TextureImageMemory  = nullptr;
	m_StagingBuffer       = nullptr;
	m_StagingBufferMemory = nullptr;
}

void Model::Resize(uint32_t width, uint32_t height)
{
	if (m_TextureImage && m_Width == width && m_Height == height)
		return;

	// TODO: max size?

	m_Width  = width;
	m_Height = height;

	Release();
	AllocateTextureMemory(m_Width * m_Height * Utils::BytesPerPixel(m_Format));
}

}        // namespace Walnut

#pragma once
#include "vulkan/vulkan.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "image.h"
#include <glm/glm.hpp>
// std
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace Walnut
{

struct Vertex
{
	glm::vec3 position{};
	glm::vec3 color{};
	glm::vec3 normal{};
	glm::vec2 uv{};

	static std::vector<VkVertexInputBindingDescription>   getBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

	bool operator==(const Vertex &other) const
	{
		return position == other.position && color == other.color && normal == other.normal &&
		       uv == other.uv;
	}
};

struct Builder
{
	std::vector<Vertex>   vertices{};
	std::vector<uint32_t> indices{};

	void loadModel(const std::string &filepath);
};
class Model
{
  public:
	Model(const std::string &modelpath, const std::string &texturepath);
	Model(uint32_t width, uint32_t height, Walnut::ImageFormat format, const void *data = nullptr);
	~Model();

  public:
	void SetTextureData(const void *data);
	void Resize(uint32_t width, uint32_t height);

  public:
	VkDescriptorSet GetDescriptorSet() const
	{
		return m_DescriptorSet;
	}

	uint32_t GetWidth() const
	{
		return m_Width;
	}
	uint32_t GetHeight() const
	{
		return m_Height;
	}

	VkImage GetTextureImage() const
	{
		return m_TextureImage;
	}

  private:
	void AllocateTextureMemory(uint64_t size);
	void Release();
	void CreateVertexBuffers();
	void CreateIndexBuffers();

  private:
	uint32_t m_Width = 0, m_Height = 0;

	std::unique_ptr<Builder> m_Builder;

	std::unique_ptr<VkBuffer> m_VertexBuffer;
	std::unique_ptr<VkBuffer> m_IndexBuffer;

	// Image
	VkImage             m_TextureImage;
	VkDeviceMemory      m_TextureImageMemory;
	VkImageView         m_TextureImageView;
	VkSampler           m_TextureSampler;
	Walnut::ImageFormat m_Format = Walnut::ImageFormat::None;

	// Buffer
	VkBuffer       m_StagingBuffer       = nullptr;
	VkDeviceMemory m_StagingBufferMemory = nullptr;

	uint32_t vertexCount;
	bool     hasIndexBuffer = false;
	uint32_t indexCount;

	size_t m_AlignedSize = 0;

	VkDescriptorSet m_DescriptorSet = nullptr;

	std::string m_TextureFilepath;
	std::string m_ModelFilepath;
};

}        // namespace Walnut

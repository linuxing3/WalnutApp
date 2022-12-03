#pragma once
#include "XingBuffer.h"
#include "XingDevice.h"
#include "vulkan/vulkan.h"
#include <string>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <memory>
#include <vector>

namespace xing
{
class XingModel
{
  public:
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

	XingModel(XingDevice &device, const XingModel::Builder &builder);
	~XingModel();

	XingModel(const XingModel &)            = delete;
	XingModel &operator=(const XingModel &) = delete;

	static std::unique_ptr<XingModel> createModelFromFile(
	    XingDevice &device, const std::string &filepath);

	void bind(VkCommandBuffer commandBuffer);
	void draw(VkCommandBuffer commandBuffer);

  private:
	void createVertexBuffers(const std::vector<Vertex> &vertices);
	void createIndexBuffers(const std::vector<uint32_t> &indices);

	XingDevice &xingDevice;

	std::unique_ptr<xing::XingBuffer> vertexBuffer;
	uint32_t                          vertexCount;

	bool                              hasIndexBuffer = false;
	std::unique_ptr<xing::XingBuffer> indexBuffer;
	uint32_t                          indexCount;
};
}        // namespace xing

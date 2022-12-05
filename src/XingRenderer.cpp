#include "XingRenderer.h"
#include "Application.h"
#include <algorithm>

#include "Image.h"
#include "Model.h"
#include "glm/fwd.hpp"
#include "loguru.hpp"
#include <cmath>
#include <iostream>
#include <memory>
#include <ostream>
namespace Utils
{

static uint32_t ConvertToRGBA(const glm::vec4 &color)
{
	uint8_t r = (uint8_t) (color.r * 255.0f);
	uint8_t g = (uint8_t) (color.g * 255.0f);
	uint8_t b = (uint8_t) (color.b * 255.0f);
	uint8_t a = (uint8_t) (color.a * 255.0f);

	uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
	return result;
}
}        // namespace Utils

namespace xing
{

void XingRenderer::OnResize(uint32_t width, uint32_t height)
{
	if (m_FinalImage)
	{
		// No resize necessary
		if (m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
			return;

		m_FinalImage->Resize(width, height);
	}
	else
	{
		m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ImageData;
	m_ImageData = new uint32_t[width * height];

	if (m_FinalModel)
	{
		// No resize necessary
		if (m_FinalModel->GetWidth() == width && m_FinalModel->GetHeight() == height)
			return;

		m_FinalModel->Resize(width, height);
	}
}

//
void XingRenderer::LoadGameObjects()
{
	std::cout << "Loading models and texture images " << std::endl;
	m_FinalModel = std::make_shared<Walnut::Model>("models/viking_room.obj", "models/viking_room.png");
	// m_FinalModel->CreateVertexBuffer();
	// m_FinalModel->CreateIndexBuffer();
}

// FIXME: render texture image of a model
void XingRenderer::RenderModel(const Scene &scene, const Camera &camera)
{
	m_ActiveScene  = &scene;
	m_ActiveCamera = &camera;
	// auto data      = m_FinalModel->GetTextImageData();
	// for (uint32_t y = 0; y < m_FinalImage->GetHeight(); ++y)
	// {
	// 	for (uint32_t x = 0; x < m_FinalImage->GetWidth(); ++x)
	// 	{
	// 		uint32_t index          = x + (uint32_t) (y * m_FinalImage->GetWidth());
	// 		m_ModelImageData[index] = data[index];
	// 	}
	// }
	//
	// m_FinalModel->SetTextureData(m_ModelImageData);
}

void XingRenderer::RenderBackground(const Scene &scene, const Camera &camera)
{
	m_ActiveScene  = &scene;
	m_ActiveCamera = &camera;

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); ++y)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); ++x)
		{
			glm::vec4 color = PerPixel(x, y);
			color           = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			// FIXME: index must be int
			uint32_t index     = x + (uint32_t) (y * m_FinalImage->GetWidth());
			m_ImageData[index] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);
}

glm::vec4 XingRenderer::PerPixel(uint32_t x, uint32_t y)
{
	if (x > m_FinalImage->GetWidth() / 4 && x < m_FinalImage->GetWidth() * 3 / 4 && y > m_FinalImage->GetHeight() / 4 && y < m_FinalImage->GetHeight() * 3 / 4)
	{
		return glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	return glm::vec4(0.5f, 0.3f, 0.4f, 1.0f);
}

}        // namespace xing

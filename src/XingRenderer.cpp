#include "XingRenderer.h"
#include "Application.h"
#include <algorithm>

#include "Image.h"
#include "Model.h"
#include "loguru.hpp"
#include <cmath>
#include <iostream>
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
	else
	{
		m_FinalModel = std::make_shared<Walnut::Model>(width, height, Walnut::ImageFormat::RGBA);
	}

	delete[] m_ModelData;
	m_ModelData = new uint32_t[width * height];
}

void XingRenderer::RenderModel(const Scene &scene, const Camera &camera)
{
	m_ActiveScene  = &scene;
	m_ActiveCamera = &camera;
	//
	for (uint32_t y = 0; y < m_FinalImage->GetHeight() / 2; ++y)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth() / 2; ++x)
		{
			glm::vec4 color(0.3f, 0.6f, 0.2f, 1.0f);
			color              = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			uint32_t index     = x + (uint32_t) (y * m_FinalModel->GetWidth() / 2);
			m_ModelData[index] = Utils::ConvertToRGBA(color);
		}
	}
	m_FinalModel->SetTextureData(m_ModelData);
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
	return glm::vec4(0.5f, 0.3f, 0.4f, 1.0f);
}

//
void XingRenderer::LoadGameObjects()
{
	std::cout << "Loading models and texture images " << std::endl;
	Walnut::Model viking_room_model{"models/viking_room.obj", "models/viking_room.png"};
}
}        // namespace xing

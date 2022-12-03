#include "XingRenderer.h"
#include "Application.h"
#include <algorithm>

#include "Image.h"
#include "loguru.hpp"
#include <cmath>
#include <iostream>
#include <ostream>
namespace Utils
{

static uint32_t ConvertToRGBA(const glm::vec4 &color)
{
	uint8_t r = static_cast<uint8_t>(color.r * 255.0f);
	uint8_t g = static_cast<uint8_t>(color.g * 255.0f);
	uint8_t b = static_cast<uint8_t>(color.b * 255.0f);
	uint8_t a = static_cast<uint8_t>(color.a * 255.0f);

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
}

void XingRenderer::Render(const Scene &scene, const Camera &camera)
{
	m_ActiveScene  = &scene;
	m_ActiveCamera = &camera;

	// lve renderer stuff
	//

	for (uint32_t y = 0; y < m_FinalImage->GetHeight(); ++y)
	{
		for (uint32_t x = 0; x < m_FinalImage->GetWidth(); ++x)
		{
			glm::vec4 color    = PerPixel(x, y);
			color              = glm::clamp(color, glm::vec4(0.0f), glm::vec4(1.0f));
			auto index         = x + y * m_FinalImage->GetWidth();
			m_ImageData[index] = Utils::ConvertToRGBA(color);
		}
	}

	m_FinalImage->SetData(m_ImageData);

	// TODO: load objects
	LoadGameObjects();
}

glm::vec4 XingRenderer::PerPixel(uint32_t x, uint32_t y)
{
	return glm::vec4(0.3f, 0.3f, 0.4, 1.0f);
}

//
void XingRenderer::LoadGameObjects()
{
	std::shared_ptr<XingModel> xingModel =
	    XingModel::createModelFromFile(*m_Device, "models/flat_vase.obj");

	std::cout << "flat vase: " << xingModel << std::endl;
	auto flatVase                  = XingGameObject::createGameObject();
	flatVase.model                 = xingModel;
	flatVase.transform.translation = {-.5f, .5f, 0.f};
	flatVase.transform.scale       = {3.f, 1.5f, 3.f};
	m_GameObjects.emplace(flatVase.getId(), std::move(flatVase));
	//
	xingModel = XingModel::createModelFromFile(*m_Device, "models/smooth_vase.obj");
	std::cout << "smooth vase: " << xingModel << std::endl;
	auto smoothVase                  = XingGameObject::createGameObject();
	smoothVase.model                 = xingModel;
	smoothVase.transform.translation = {.5f, .5f, 0.f};
	smoothVase.transform.scale       = {3.f, 1.5f, 3.f};
	m_GameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

	xingModel = XingModel::createModelFromFile(*m_Device, "models/quad.obj");
	std::cout << "quad vase: " << xingModel << std::endl;
	auto floor                  = XingGameObject::createGameObject();
	floor.model                 = xingModel;
	floor.transform.translation = {0.f, .5f, 0.f};
	floor.transform.scale       = {3.f, 1.f, 3.f};
	m_GameObjects.emplace(floor.getId(), std::move(floor));
	//
	std::vector<glm::vec3> lightColors{
	    {1.f, .1f, .1f},
	    {.1f, .1f, 1.f},
	    {.1f, 1.f, .1f},
	    {1.f, 1.f, .1f},
	    {.1f, 1.f, 1.f},
	    {1.f, 1.f, 1.f}        //
	};

	for (int i = 0; i < lightColors.size(); i++)
	{
		auto pointLight  = XingGameObject::makePointLight(0.2f);
		pointLight.color = lightColors[i];
		auto rotateLight = glm::rotate(
		    glm::mat4(1.f),
		    (i * glm::two_pi<float>()) / lightColors.size(),
		    {0.f, -1.f, 0.f});
		pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
		m_GameObjects.emplace(pointLight.getId(), std::move(pointLight));
	}
}
}        // namespace xing

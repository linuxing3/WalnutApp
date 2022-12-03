#pragma once

#include "Image.h"

#include "Camera.h"
#include "Scene.h"

#include "XingGameObject.h"
#include <glm/glm.hpp>
#include <memory>

namespace xing
{

class XingRenderer
{
  public:
	XingRenderer() = default;

  public:
	void OnResize(uint32_t width, uint32_t height);
	void Render(const Scene &scene, const Camera &camera);

	std::shared_ptr<Walnut::Image> GetFinalImage() const
	{
		return m_FinalImage;
	}

	// FIXME:
	void LoadGameObjects();

  private:
	glm::vec4 PerPixel(uint32_t x, uint32_t y);

  private:
	std::shared_ptr<Walnut::Image> m_FinalImage;

	const Scene  *m_ActiveScene  = nullptr;
	const Camera *m_ActiveCamera = nullptr;

	uint32_t           *m_ImageData = nullptr;
	XingGameObject::Map m_GameObjects;
	XingDevice         *m_Device = nullptr;
};
}        // namespace xing

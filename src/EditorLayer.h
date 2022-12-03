#include "Application.h"

#include "Image.h"
#include "Timer.h"

#include "Camera.h"
#include "Renderer.h"

#include <glm/gtc/type_ptr.hpp>
#include <utility>

using namespace Walnut;

class EditorLayer : public Walnut::Layer
{
  public:
	EditorLayer() :
	    m_Camera(45.0f, 0.1f, 100.0f)
	{
	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Editor Viewport");

		m_ViewportWidth  = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if (image)
			ImGui::Image(image->GetDescriptorSet(),
			             {(float) image->GetWidth(), (float) image->GetHeight()},
			             ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();

		Render();
	}

	void Render()
	{
		Timer timer;

		m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}

  private:
	Renderer m_Renderer;
	Camera   m_Camera;
	Scene    m_Scene;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0F;
};

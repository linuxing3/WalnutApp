#include "Application.h"

#include "Timer.h"

#include "Camera.h"
#include "XingRenderer.h"

#include <glm/gtc/type_ptr.hpp>
#include <utility>

using namespace Walnut;

class GameEditorLayer : public Walnut::Layer
{
  public:
	GameEditorLayer() :
	    m_Camera(45.0f, 0.1f, 100.0f)
	{
	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
		xing::XingDevice device(*m_window);
		m_xingRenderer.LoadGameObjects(device);
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Editor Viewport");

		m_ViewportWidth  = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		ImGui::End();

		// Render();
	}

	void Render()
	{
		Timer timer;

		m_xingRenderer.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		m_xingRenderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}

	void setWindow(GLFWwindow *window)
	{
		m_window = window;
	}

  private:
	xing::XingRenderer m_xingRenderer;
	Camera             m_Camera;
	Scene              m_Scene;
	GLFWwindow        *m_window        = nullptr;
	uint32_t           m_ViewportWidth = 0, m_ViewportHeight = 0;

	float m_LastRenderTime = 0.0F;
};

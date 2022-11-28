#include "Application.h"
#include "imgui.h"
using Walnut::Layer;
class DemoLayer : public Layer {
  bool m_ShowDemo = false;

public:
  virtual void OnAttach() override { m_ShowDemo = false; }
  virtual void OnUIRender() override {
    const char *version = ImGui::GetVersion();
    ImGui::Begin("Hello");
    if (ImGui::Button("Show demo")) {
      m_ShowDemo = !m_ShowDemo;
    }
    ImGui::Text("ImGui Version: %s", version);
    ImGui::End();

    if (m_ShowDemo) {
      ImGui::ShowDemoWindow();
    }
  }
};
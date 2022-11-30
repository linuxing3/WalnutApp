#include "Application.h"
#include "Roboto-Regular.embed"
#include "imgui.h"
using Walnut::Layer;
class DemoLayer : public Layer {
  bool m_ShowDemo = false;
  ImFont *font;

public:
  virtual void OnAttach() override {
    m_ShowDemo = false;
    ImFontConfig fontConfig;
    ImGuiIO &io = ImGui::GetIO();
    fontConfig.FontDataOwnedByAtlas = false;
    font = io.Fonts->AddFontFromMemoryTTF(
        (void *)g_RobotoRegular, sizeof(g_RobotoRegular), 20.0f, &fontConfig);
  }
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

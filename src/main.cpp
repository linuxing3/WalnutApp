#include "Application.h"
#include "EntryPoint.h"
#include <iostream>
#include <memory>

#include "RayLayer.h"

using std::make_shared;
using std::shared_ptr;
using Walnut::Application;
using Walnut::ApplicationSpecification;
using Walnut::CreateApplication;
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

Application *Walnut::CreateApplication(int argc, char **argv) {
  ApplicationSpecification spec;
  spec.Name = "Walnut Example";

  auto *app = new Application(spec);
  auto demoLayer = make_shared<DemoLayer>();
  auto rayLayer = make_shared<RayLayer>();
  app->PushLayer(demoLayer);
  app->PushLayer(rayLayer);
  //   app->PushLayer<ExampleLayer>();
  app->SetMenubarCallback([app]() {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Exit")) {
        app->Close();
      }
      ImGui::EndMenu();
    }
  });
  return app;
}

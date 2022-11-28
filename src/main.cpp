#include "Application.h"
#include "EntryPoint.h"
#include <iostream>
#include <memory>

#include "RayLayer.h"
#include "DemoLayer.h"

using std::make_shared;
using std::shared_ptr;
using Walnut::Application;
using Walnut::ApplicationSpecification;
using Walnut::CreateApplication;
using Walnut::Layer;

Application *Walnut::CreateApplication(int argc, char **argv) {
  ApplicationSpecification spec;
  spec.Name = "Walnut Example";

  auto *app = new Application(spec);
  auto demoLayer = make_shared<DemoLayer>();
  auto rayLayer = make_shared<RayLayer>();
  app->PushLayer(demoLayer);
  app->PushLayer(rayLayer);
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

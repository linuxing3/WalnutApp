#include "Application.h"
#include "EntryPoint.h"
#include <emilib/strprintf.cpp>
#include <iostream>
#include <loguru.hpp>
#include <memory>

#include "DemoLayer.h"
#include "EditorLayer.h"
#include "GameEditorLayer.h"
#include "RayLayer.h"

using std::make_shared;
using std::shared_ptr;
using Walnut::Application;
using Walnut::ApplicationSpecification;
using Walnut::CreateApplication;
using Walnut::Layer;

Application *Walnut::CreateApplication(int argc, char **argv)
{
	std::cout << emilib::strprintf("Format float: %.1f", 1.234) << std::endl;
	ApplicationSpecification spec;
	spec.Name = "Walnut Example";

	auto *app         = new Application(spec);
	auto  demoLayer   = make_shared<DemoLayer>();
	auto  editorLayer = make_shared<EditorLayer>();
	// auto  rayLayer    = make_shared<RayLayer>();

	// TODO:
	// std::shared_ptr<GameEditorLayer> gameEditorLayer;
	// gameEditorLayer->setWindow(app->GetWindowHandle());

	app->PushLayer(demoLayer);
	app->PushLayer(editorLayer);
	// app->PushLayer(gameEditorLayer);
	//   app->PushLayer(rayLayer);
	app->SetMenubarCallback([app]() {
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}

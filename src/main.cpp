#include "Application.h"
#include "EntryPoint.h"
#include <iostream>
#include <loguru.hpp>
#include <memory>

#include "DemoLayer.h"
#include "EditorLayer.h"
// #include "GameEditorLayer.h"
// #include "RayLayer.h"

using std::make_shared;
using std::shared_ptr;
using Walnut::Application;
using Walnut::ApplicationSpecification;
using Walnut::CreateApplication;
using Walnut::Layer;

Application *Walnut::CreateApplication(int argc, char **argv)
{
	ApplicationSpecification spec;
	spec.Name = "XingApp";

	auto *app       = new Application(spec);
	auto  demoLayer = make_shared<DemoLayer>();
	// auto  rayLayer    = make_shared<RayLayer>();
	//
	// NOTE: editor layer with some vulkan functions
	auto editorLayer   = make_shared<EditorLayer>();
	editorLayer->m_App = app;

	app->PushLayer(demoLayer);
	app->PushLayer(editorLayer);
	// app->PushLayer(rayLayer);

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

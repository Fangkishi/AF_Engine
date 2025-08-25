#include <AF.h>
#include <AF/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace AF {

	class AFEditor : public Application
	{
	public:
		AFEditor(ApplicationSpecification& specification)
			: Application(specification)
		{
			PushLayer(new EditorLayer());
		}

		~AFEditor()
		{

		}

	};

	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "AF-Editor";
		spec.CommandLineArgs = args;

		return new AFEditor(spec);
	}

}


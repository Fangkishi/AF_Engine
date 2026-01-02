/**
 * @file AFEditorApp.cpp
 * @brief AFEngine 编辑器应用程序入口
 * 
 * 该文件定义了编辑器的主类 AFEditor，继承自 AF::Application。
 * 它负责初始化编辑器图层 (EditorLayer) 并启动应用程序循环。
 */

#include <AF.h>
#include <AF/Core/EntryPoint.h>

#include "EditorLayer.h"

namespace AF {

	// ===================================================================================
	// --- 应用程序类 (Application Class) ---
	// ===================================================================================

	/**
	 * @class AFEditor
	 * @brief 编辑器主程序类
	 * 
	 * 继承自 Application，负责初始化编辑器层。
	 */
	class AFEditor : public Application
	{
	public:
		// ===================================================================================
		// --- 生命周期 (Lifecycle) ---
		// ===================================================================================

		AFEditor(ApplicationSpecification& specification)
			: Application(specification)
		{
			// 将编辑器层推入层栈，由 Application 主循环进行更新和渲染
			PushLayer(new EditorLayer());
		}

		~AFEditor()
		{
			// Application 基类会自动清理 LayerStack
		}
	};

	// ===================================================================================
	// --- 入口点 (Entry Point) ---
	// ===================================================================================

	/**
	 * @brief 创建应用程序实例
	 * 
	 * 由引擎核心 EntryPoint 调用。
	 */
	Application* CreateApplication(ApplicationCommandLineArgs args)
	{
		ApplicationSpecification spec;
		spec.Name = "AF-Editor";
		spec.CommandLineArgs = args;

		return new AFEditor(spec);
	}
}

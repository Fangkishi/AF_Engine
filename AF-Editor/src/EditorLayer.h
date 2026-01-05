#pragma once

#include "AF/Core/Layer.h"
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ToolbarPanel.h"
#include "Panels/ViewportPanel.h"
#include "Panels/StatsPanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/PropertiesPanel.h"

#include "AF/Events/Event.h"
#include "AF/Events/KeyEvent.h"
#include "AF/Events/MouseEvent.h"

#include "AF/Renderer/EditorCamera.h"

namespace AF {

	/**
	* @brief 编辑器主图层类
	* 负责管理编辑器的 UI、场景更新、渲染流程以及各个面板的交互。
	*/
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		// ===================================================================================
		// --- 生命周期 (Lifecycle) ---
		// ===================================================================================

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		// ===================================================================================
		// --- 核心逻辑 (Core Logic) ---
		// ===================================================================================

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;

	private:
		// --- 场景更新分支 ---
		void OnUpdateRuntime(Timestep ts);                                       // 运行模式下的场景更新
		void OnUpdateSimulation(Timestep ts, Ref<EditorCamera>& camera);        // 模拟模式下的场景更新
		void OnUpdateEditor(Timestep ts, Ref<EditorCamera>& camera);            // 编辑模式下的场景更新

		// --- 渲染逻辑 ---
		void OnOverlayRender();                                                 // 渲染辅助叠加层 (Gizmos, 物理边框等)

		// --- UI 渲染辅助函数 ---
		void UI_DrawDockSpace();                                                // 绘制 DockSpace 停靠空间
		void UI_DrawMenuBar();                                                  // 绘制主菜单栏
		void UI_DrawPanels();                                                   // 绘制各个子面板

		// ===================================================================================
		// --- 事件处理 (Event Handling) ---
		// ===================================================================================

	public:
		virtual void OnEvent(Event& e) override;

	private:
		bool OnKeyPressed(KeyPressedEvent& e);                                  // 键盘按键事件处理
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);                  // 鼠标按键事件处理

		// ===================================================================================
		// --- 私有辅助 (Private Helpers) ---
		// ===================================================================================

		// --- 播放状态控制 ---
		void OnScenePlay();                                                     // 开始播放场景
		void OnSceneSimulate();                                                 // 开始模拟物理
		void OnSceneStop();                                                     // 停止播放/模拟
		void OnScenePause();                                                    // 暂停/恢复场景

		// --- 项目与场景管理 ---
		void NewProject();                                                      // 创建新项目
		bool OpenProject();                                                     // 弹出对话框打开项目
		void OpenProject(const std::filesystem::path& path);                    // 打开指定路径的项目
		void SaveProject();                                                     // 保存当前项目

		void NewScene();                                                        // 创建新场景
		void OpenScene();                                                       // 弹出对话框打开场景
		void OpenScene(const std::filesystem::path& path);                      // 打开指定路径的场景
		void SaveScene();                                                       // 保存当前场景
		void SaveSceneAs();                                                     // 场景另存为

		void ImportModel();                                                     // 导入 3D 模型

		// --- 序列化辅助 ---
		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		// --- 实体操作 ---
		void OnDuplicateEntity();                                               // 复制当前选中的实体

		// --- 调试状态 ---
		std::string m_DebugTextureName;                                         // 当前视口调试显示的纹理名称

	private:
		// --- 场景与相机 ---
		Ref<Scene> m_ActiveScene;                                               // 当前活跃的场景 (可能是编辑场景或运行副本)
		Ref<Scene> m_EditorScene;                                               // 原始编辑场景 (持久化数据源)
		std::filesystem::path m_EditorScenePath;                                // 当前编辑场景的磁盘路径
		Ref<EditorCamera> m_EditorCamera;                                       // 编辑器专用的自由视角相机

		// --- 资源与渲染设置 ---
		Ref<Texture2D> m_CheckerboardTexture;                                   // 默认棋盘格贴图
		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };                   // 临时颜色设置

		// --- 编辑器运行状态 ---
		enum class SceneState { Edit = 0, Play = 1, Simulate = 2 };
		SceneState m_SceneState = SceneState::Edit;                             // 当前编辑器所处的模式

		int m_GizmoType = -1;                                                   // 当前使用的 Gizmo 类型 (平移/旋转/缩放)
		bool m_ShowPhysicsColliders = false;                                    // 是否显示物理碰撞体调试信息

		// --- UI 面板 (Panels) ---
		SceneHierarchyPanel m_SceneHierarchyPanel;                              // 场景层级面板
		Scope<ContentBrowserPanel> m_ContentBrowserPanel;                       // 内容浏览器面板 (资源管理器)
		ToolbarPanel m_ToolbarPanel;                                            // 顶部工具栏 (播放/暂停/停止)
		ViewportPanel m_ViewportPanel;                                          // 主视口面板 (渲染画面显示)
		StatsPanel m_StatsPanel;                                                // 性能统计面板
		SettingsPanel m_SettingsPanel;                                          // 编辑器设置面板
		PropertiesPanel m_PropertiesPanel;                                      // 属性面板
	};

}

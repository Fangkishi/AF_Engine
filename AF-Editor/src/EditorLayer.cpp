#include "EditorLayer.h"
#include "AF/Core/Application.h"
#include "AF/Core/Input.h"
#include "AF/Core/KeyCodes.h"
#include "AF/Renderer/SceneRenderer.h"
#include "AF/Scene/SceneSerializer.h"
#include "AF/Utils/PlatformUtils.h"
#include "AF/Loader/AssimpLoader.h"

#include "AF/Project/Project.h"

#include <imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AF/Renderer/LightProbeManager.h"
#include "ImGuizmo.h"

namespace AF {

	EditorLayer::EditorLayer()
		: Layer("EditorLayer"), m_SquareColor({ 0.2f, 0.3f, 0.8f, 1.0f })
	{
	}

	// ===================================================================================
	// --- 生命周期 (Lifecycle) ---
	// ===================================================================================

	void EditorLayer::OnAttach()
	{
		AF_PROFILE_FUNCTION();

		// 1. 加载编辑器默认资源
		m_CheckerboardTexture = Texture2D::Create("assets/textures/defaultTexture.jpg");

		// 2. 初始化各个面板
		m_ToolbarPanel.OnAttach();

		// 3. 初始化场景 (默认创建一个空场景)
		m_EditorScene = CreateRef<Scene>();
		m_ActiveScene = m_EditorScene;

		// 4. 默认场景内容初始化 (演示用代码，正式项目可删除)
#if 1
		// --- SSGI 演示场景 (Cornell Box 变体) ---
		
		// 1. 创建材质 (稍微增加粗糙度，降低反射率，使得漫反射溢色更纯粹)
		auto matWhite = Material::CreatePBR();
		matWhite->SetUniform("u_Material.AlbedoColor", glm::vec4(0.85f, 0.85f, 0.85f, 1.0f));
		matWhite->SetUniform("u_Material.Roughness", 1.0f);
		matWhite->SetUniform("u_Material.Metallic", 0.0f);

		auto matRed = Material::CreatePBR();
		matRed->SetUniform("u_Material.AlbedoColor", glm::vec4(0.85f, 0.05f, 0.05f, 1.0f)); // 更纯的红
		matRed->SetUniform("u_Material.Roughness", 1.0f);
		matRed->SetUniform("u_Material.Metallic", 0.0f);

		auto matGreen = Material::CreatePBR();
		matGreen->SetUniform("u_Material.AlbedoColor", glm::vec4(0.05f, 0.85f, 0.05f, 1.0f)); // 更纯的绿
		matGreen->SetUniform("u_Material.Roughness", 1.0f);
		matGreen->SetUniform("u_Material.Metallic", 0.0f);

		// 2. 搭建经典的等比例 Cornell Box (假设房间内部尺寸为 4x4x4，原点在底面中心)
		auto floor = m_ActiveScene->CreateEntity("Floor");
		floor.AddComponent<MeshComponent>(Mesh::CreateBox(1.0f));
		floor.AddComponent<MaterialComponent>(matWhite);
		floor.GetComponent<TransformComponent>().Translation = { 0.0f, 0.0f, 0.0f };
		floor.GetComponent<TransformComponent>().Scale = { 4.0f, 0.1f, 4.0f };

		auto ceiling = m_ActiveScene->CreateEntity("Ceiling");
		ceiling.AddComponent<MeshComponent>(Mesh::CreateBox(1.0f));
		ceiling.AddComponent<MaterialComponent>(matWhite);
		ceiling.GetComponent<TransformComponent>().Translation = { 0.0f, 4.0f, 0.0f };
		ceiling.GetComponent<TransformComponent>().Scale = { 4.0f, 0.1f, 4.0f };

		auto leftWall = m_ActiveScene->CreateEntity("Left Wall");
		leftWall.AddComponent<MeshComponent>(Mesh::CreateBox(1.0f));
		leftWall.AddComponent<MaterialComponent>(matRed);
		leftWall.GetComponent<TransformComponent>().Translation = { -2.0f, 2.0f, 0.0f };
		leftWall.GetComponent<TransformComponent>().Scale = { 0.1f, 4.0f, 4.0f };

		auto rightWall = m_ActiveScene->CreateEntity("Right Wall");
		rightWall.AddComponent<MeshComponent>(Mesh::CreateBox(1.0f));
		rightWall.AddComponent<MaterialComponent>(matGreen);
		rightWall.GetComponent<TransformComponent>().Translation = { 2.0f, 2.0f, 0.0f };
		rightWall.GetComponent<TransformComponent>().Scale = { 0.1f, 4.0f, 4.0f };

		auto backWall = m_ActiveScene->CreateEntity("Back Wall");
		backWall.AddComponent<MeshComponent>(Mesh::CreateBox(1.0f));
		backWall.AddComponent<MaterialComponent>(matWhite);
		backWall.GetComponent<TransformComponent>().Translation = { 0.0f, 2.0f, -2.0f };
		backWall.GetComponent<TransformComponent>().Scale = { 4.0f, 4.0f, 0.1f };

		// 3. 放置中心物体
		// 高盒子 (靠后，靠左，轻微旋转)
		auto tallBox = m_ActiveScene->CreateEntity("Tall Box");
		tallBox.AddComponent<MeshComponent>(Mesh::CreateBox(1.0f));
		tallBox.AddComponent<MaterialComponent>(matWhite);
		tallBox.GetComponent<TransformComponent>().Translation = { -0.6f, 1.2f, -0.6f };
		tallBox.GetComponent<TransformComponent>().Scale = { 1.2f, 2.4f, 1.2f };
		tallBox.GetComponent<TransformComponent>().Rotation = { 0.0f, 0.25f, 0.0f };

		// 矮盒子 (靠前，靠右，轻微旋转，代替球体以展示更多硬边遮蔽)
		auto shortBox = m_ActiveScene->CreateEntity("Short Box");
		shortBox.AddComponent<MeshComponent>(Mesh::CreateBox(1.0f));
		shortBox.AddComponent<MaterialComponent>(matWhite);
		shortBox.GetComponent<TransformComponent>().Translation = { 0.6f, 0.6f, 0.4f };
		shortBox.GetComponent<TransformComponent>().Scale = { 1.2f, 1.2f, 1.2f };
		shortBox.GetComponent<TransformComponent>().Rotation = { 0.0f, -0.25f, 0.0f };

		// 4. 设置顶部主光源
		auto pointLight = m_ActiveScene->CreateEntity("Main Light");
		auto& pl = pointLight.AddComponent<PointLightComponent>();
		pl.Color = { 1.0f, 0.98f, 0.95f }; 
		// 在 4x4 的小空间里，适当的强度能照亮地面并弹射
		pl.Intensity = 1.0f; 
		// 紧贴天花板中心
		pointLight.GetComponent<TransformComponent>().Translation = { 0.0f, 3.8f, 0.0f };

		// 5. 生成光照探针网格
		LightProbeGrid probeGrid;
		probeGrid.MinBounds = { -3.0f, 0.0f, -3.0f };
		probeGrid.MaxBounds = {  3.0f, 4.0f,  3.0f };
		probeGrid.Counts = { 3, 3, 3 }; // 生成 3x3x3 = 27 个探针
		probeGrid.ProbeRadius = 4.0f; // 影响半径覆盖整个房间
		
		LightProbeManager::GenerateProbes(m_ActiveScene, probeGrid);
		LightProbeManager::BakeProbes(m_ActiveScene);

#endif

		// 6. 将场景上下文传递给 UI 面板
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();

		// 6. 初始化编辑器自由相机
		m_EditorCamera = CreateRef<EditorCamera>(30.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	}

	void EditorLayer::OnDetach()
	{
		AF_PROFILE_FUNCTION();
	}

	// ===================================================================================
	// --- 核心逻辑 (Core Logic) ---
	// ===================================================================================

	void EditorLayer::OnUpdate(Timestep ts)
	{
		AF_PROFILE_FUNCTION();

		// 1. 处理视口尺寸调整 (只有当视口大小改变时才通知场景和相机)
		glm::vec2 viewportSize = m_ViewportPanel.GetSize();
		if (viewportSize.x > 0.0f && viewportSize.y > 0.0f)
		{
			static glm::vec2 lastViewportSize = { 0.0f, 0.0f };
			if (lastViewportSize.x != viewportSize.x || lastViewportSize.y != viewportSize.y)
			{
				m_ActiveScene->OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
				SceneRenderer::OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
				m_EditorCamera->SetViewportSize(viewportSize.x, viewportSize.y);
				lastViewportSize = viewportSize;
			}
		}

		// 2. 准备渲染 (清理缓冲)
		Renderer2D::ResetStats();
		{
			AF_PROFILE_SCOPE("Renderer Prep");
			RenderCommand::Clear();
		}

		// 3. 根据编辑器状态执行不同的更新与渲染逻辑
		{
			AF_PROFILE_SCOPE("Renderer Draw");
			switch (m_SceneState)
			{
			case SceneState::Edit:
				m_EditorCamera->OnUpdate(ts);
				OnUpdateEditor(ts, m_EditorCamera);
				break;
			case SceneState::Simulate:
				m_EditorCamera->OnUpdate(ts);
				OnUpdateSimulation(ts, m_EditorCamera);
				break;
			case SceneState::Play:
				OnUpdateRuntime(ts);
				break;
			}
		}

		// 4. 渲染辅助信息 (Gizmos, 碰撞框等)
		OnOverlayRender();
	}

	void EditorLayer::OnUpdateRuntime(Timestep ts)
	{
		m_ActiveScene->OnUpdateRuntime(ts);
	}

	void EditorLayer::OnUpdateSimulation(Timestep ts, Ref<EditorCamera>& camera)
	{
		m_ActiveScene->OnUpdateSimulation(ts, camera);
	}

	void EditorLayer::OnUpdateEditor(Timestep ts, Ref<EditorCamera>& camera)
	{
		m_ActiveScene->OnUpdateEditor(ts, camera);
	}

	void EditorLayer::OnOverlayRender()
	{
		// 1. 开始 2D 渲染叠加层
		if (m_SceneState == SceneState::Play)
		{
			// 运行模式使用主摄像机
			Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			if (cameraEntity)
				Renderer2D::BeginScene(cameraEntity.GetComponent<CameraComponent>().Camera);
		}
		else
		{
			// 编辑/模拟模式使用编辑器相机
			Renderer2D::BeginScene(m_EditorCamera);
		}

		// 2. 渲染物理调试信息 (如碰撞体边框)
		if (m_ShowPhysicsColliders)
		{
			// TODO: 实现物理调试渲染逻辑
		}

		// 3. 结束渲染
		Renderer2D::EndScene();
	}

	void EditorLayer::OnImGuiRender()
	{
		AF_PROFILE_FUNCTION();

		// 1. 停靠空间 (DockSpace) 设置
		UI_DrawDockSpace();

		// 2. 菜单栏 (MenuBar)
		UI_DrawMenuBar();

		// 3. 渲染各个面板 (Panels)
		UI_DrawPanels();

		ImGui::End();
	}



	// ===================================================================================
	// --- 事件处理 (Event Handling) ---
	// ===================================================================================

	void EditorLayer::OnEvent(Event& e)
	{
		AF_PROFILE_FUNCTION();

		// 只有在编辑模式下，编辑器相机才处理事件 (如缩放、平移视角)
		if (m_SceneState == SceneState::Edit)
		{
			m_EditorCamera->OnEvent(e);
		}

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(AF_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(AF_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		// 键盘快捷键处理
		if (e.IsRepeat())
			return false;

		bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
		bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

		switch (e.GetKeyCode())
		{
		// --- 场景文件操作 ---
		case Key::N:
			if (control) NewScene();
			break;
		case Key::O:
			if (control)
			{
				if (shift) OpenProject();
				else OpenScene();
			}
			break;
		case Key::S:
			if (control)
			{
				if (shift) SaveSceneAs();
				else SaveScene();
			}
			break;

		// --- Gizmo 切换 ---
		case Key::Q: // 禁用 Gizmo
			if (!ImGuizmo::IsUsing())
				m_GizmoType = -1;
			break;
		case Key::W: // 平移 (Translate)
			if (!ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case Key::E: // 旋转 (Rotate)
			if (!ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		case Key::R: // 缩放 (Scale)
			if (!ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		}
		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		// 视口内的实体选择逻辑
		if (e.GetMouseButton() == Mouse::ButtonLeft)
		{
			// 如果鼠标在视口内，且没有在使用 Gizmo，且没有按下 Alt (Alt 通常用于相机旋转)
			if (m_ViewportPanel.IsHovered() && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
			{
				auto [mx, my] = ImGui::GetMousePos();
				mx -= m_ViewportPanel.GetViewportMinBound().x;
				my -= m_ViewportPanel.GetViewportMinBound().y;
				glm::vec2 viewportSize = m_ViewportPanel.GetViewportMaxBound() - m_ViewportPanel.GetViewportMinBound();
				my = viewportSize.y - my;

				int mouseX = (int)mx;
				int mouseY = (int)my;

				if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
				{
					int pixelData = SceneRenderer::ReadPixel(mouseX, mouseY);
					if (pixelData != -1)
					{
						Entity pickedEntity = Entity((entt::entity)pixelData, m_ActiveScene.get());
						m_SceneHierarchyPanel.SetSelectedEntity(pickedEntity);
					}
					else
					{
						m_SceneHierarchyPanel.SetSelectedEntity(Entity());
					}
				}
			}
		}
		return false;
	}

	// ===================================================================================
	// --- 私有辅助 (Private Helpers) ---
	// ===================================================================================

	// --- UI 辅助 (UI Helpers) ---

	void EditorLayer::UI_DrawDockSpace()
	{
		static bool dockspaceOpen = true;
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
				ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);

		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// 提交 DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		style.WindowMinSize.x = minWinSizeX;
	}

	void EditorLayer::UI_DrawMenuBar()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Scene", "Ctrl+N")) NewScene();
				if (ImGui::MenuItem("Open Scene", "Ctrl+O")) OpenScene();
				if (ImGui::MenuItem("Save Scene", "Ctrl+S")) SaveScene();
				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) SaveSceneAs();
				if (ImGui::MenuItem("Import")) ImportModel();
				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) Application::Get().Close();
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Script"))
			{
				if (ImGui::MenuItem("Reload assembly", "Ctrl+R")) {}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}

	void EditorLayer::UI_DrawPanels()
	{
		// 3.1 场景层级面板 (Scene Hierarchy)
		m_SceneHierarchyPanel.OnImGuiRender();

		// 3.2 内容浏览器面板 (Content Browser)
		m_ContentBrowserPanel->OnImGuiRender();

		// 3.3 性能统计面板 (Stats)
		m_StatsPanel.OnImGuiRender();

		// 3.4 编辑器设置面板 (Settings)
		m_SettingsPanel.OnImGuiRender(m_ShowPhysicsColliders, m_DebugTextureName);

		// 3.5 属性面板 (Properties)
		// 将当前层级面板选中的实体传递给属性面板
		m_PropertiesPanel.SetContext(m_SceneHierarchyPanel.GetSelectedEntity());
		m_PropertiesPanel.OnImGuiRender();

		// 3.6 视口面板 (Viewport)
		m_ViewportPanel.OnImGuiRender(m_ActiveScene, m_EditorCamera,
			m_SceneHierarchyPanel.GetSelectedEntity(), m_GizmoType, m_ShowPhysicsColliders,
			[this](const std::filesystem::path& path) { OpenScene(path); },
			m_DebugTextureName);

		// 如果鼠标不在视口内，则阻塞事件，防止 UI 操作干扰视口内交互
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportPanel.IsHovered());

		// 3.6 顶部工具栏 (Toolbar)
		m_ToolbarPanel.OnImGuiRender((int)m_SceneState, m_ActiveScene->IsPaused(),
			[this]() { OnScenePlay(); },
			[this]() { OnSceneSimulate(); },
			[this]() { OnSceneStop(); },
			[this]() {
				bool isPaused = m_ActiveScene->IsPaused();
				m_ActiveScene->SetPaused(!isPaused);
			},
			[this]() { m_ActiveScene->Step(); }
		);
	}

	// --- 项目与场景管理 ---

	void EditorLayer::NewProject()
	{
		Project::New();
	}

	bool EditorLayer::OpenProject()
	{
		std::string filepath = FileDialogs::OpenFile("Hazel Project (*.hproj)\0*.hproj\0");
		if (filepath.empty())
			return false;

		OpenProject(filepath);
		return true;
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path))
		{
			auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene);
			OpenScene(startScenePath);
			m_ContentBrowserPanel = CreateScope<ContentBrowserPanel>();
		}
	}

	void EditorLayer::SaveProject()
	{
		// Project::SaveActive();
	}

	void EditorLayer::NewScene()
	{
		m_EditorScene = CreateRef<Scene>();
		m_EditorScene->OnViewportResize((uint32_t)m_ViewportPanel.GetSize().x, (uint32_t)m_ViewportPanel.GetSize().y);
		m_ActiveScene = m_EditorScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_EditorScenePath = std::filesystem::path();
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = FileDialogs::OpenFile("Hazel Scene (*.hazel)\0*.hazel\0");
		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		if (path.extension().string() != ".hazel")
		{
			AF_CORE_WARN("Could not load {0} - not a scene file", path.filename().string());
			return;
		}

		Ref<Scene> newScene = CreateRef<Scene>();
		SceneSerializer serializer(newScene);
		if (serializer.Deserialize(path.string()))
		{
			m_EditorScene = newScene;
			m_EditorScene->OnViewportResize((uint32_t)m_ViewportPanel.GetSize().x, (uint32_t)m_ViewportPanel.GetSize().y);
			m_SceneHierarchyPanel.SetContext(m_EditorScene);
			m_ActiveScene = m_EditorScene;
			m_EditorScenePath = path;
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!m_EditorScenePath.empty())
			SerializeScene(m_EditorScene, m_EditorScenePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = FileDialogs::SaveFile("Hazel Scene (*.hazel)\0*.hazel\0");
		if (!filepath.empty())
		{
			SerializeScene(m_EditorScene, filepath);
			m_EditorScenePath = filepath;
		}
	}

	void EditorLayer::ImportModel()
	{
		std::string filepath = FileDialogs::OpenFile("Model (*.obj;*.fbx;*.gltf)\0*.obj;*.fbx;*.gltf\0");
		if (!filepath.empty())
		{
			// 确认使用绝对路径
			std::filesystem::path absolutePath = std::filesystem::absolute(filepath);
			std::string normalizedPath = absolutePath.string();

			// 统一路径分隔符
			std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');

			AF_INFO("Loading: {}", normalizedPath);
			AssimpLoader::Load(normalizedPath, m_EditorScene);
		}
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(path.string());
	}

	// --- 播放状态控制 ---

	void EditorLayer::OnScenePlay()
	{
		// 检查场景中是否存在主摄像机，如果不存在则不允许播放
		Entity cameraEntity = m_EditorScene->GetPrimaryCameraEntity();
		if (!cameraEntity)
		{
			AF_CORE_WARN("无法开始播放：场景中没有设置主摄像机！");
			return;
		}

		if (m_SceneState == SceneState::Simulate)
			OnSceneStop();

		m_SceneState = SceneState::Play;
		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnRuntimeStart();
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneSimulate()
	{
		// 检查场景中是否存在主摄像机，如果不存在则不允许模拟
		Entity cameraEntity = m_EditorScene->GetPrimaryCameraEntity();
		if (!cameraEntity)
		{
			AF_CORE_WARN("无法开始模拟：场景中没有设置主摄像机！");
			return;
		}

		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		m_SceneState = SceneState::Simulate;
		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ActiveScene->OnSimulationStart();
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		AF_CORE_ASSERT(m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate);

		if (m_SceneState == SceneState::Play)
			m_ActiveScene->OnRuntimeStop();
		else if (m_SceneState == SceneState::Simulate)
			m_ActiveScene->OnSimulationStop();

		m_SceneState = SceneState::Edit;
		m_ActiveScene = m_EditorScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnScenePause()
	{
		if (m_SceneState == SceneState::Edit)
			return;

		m_ActiveScene->SetPaused(!m_ActiveScene->IsPaused());
	}

	// --- 实体操作 ---

	void EditorLayer::OnDuplicateEntity()
	{
		// 只有在编辑模式下才允许复制实体，防止污染运行时副本
		if (m_SceneState != SceneState::Edit)
			return;

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity)
			m_EditorScene->DuplicateEntity(selectedEntity);
	}

}

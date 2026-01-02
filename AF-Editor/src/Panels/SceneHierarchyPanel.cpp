#include "SceneHierarchyPanel.h"
#include "AF/Scene/Components.h"

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

namespace AF {

	// ===================================================================================
	// --- 生命周期 (Lifecycle) ---
	// ===================================================================================

	SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
	{
		SetContext(context);
	}

	// 设置当前场景上下文
	void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
	{
		m_Context = context;
		m_SelectionContext = {};
	}

	// ===================================================================================
	// --- 核心逻辑 (Core Logic) ---
	// ===================================================================================

	// 渲染层级面板主逻辑
	void SceneHierarchyPanel::OnImGuiRender()
	{
		// 1. 渲染层级面板 (Scene Hierarchy)
		// 显示场景中所有实体的树状列表
		ImGui::Begin("Scene Hierarchy");

		if (m_Context)
		{
			// 遍历场景中所有实体并绘制节点
			auto view = m_Context->m_Registry.view<IDComponent>();
			for (auto entityID : view)
			{
				Entity entity{entityID, m_Context.get()};
				DrawEntityNode(entity);
			}

			// 点击空白处取消选中
			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectionContext = {};

			// 右键点击空白处弹出创建菜单
			if (ImGui::BeginPopupContextWindow(
				nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Empty Entity"))
					m_Context->CreateEntity("Empty Entity");

				ImGui::EndPopup();
			}
		}
		ImGui::End();
	}

	// ===================================================================================
	// --- 事件处理 (Event Handling) ---
	// ===================================================================================

	// 设置当前选中的实体 (通常由外部调用，如点击视口中的物体)
	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{
		m_SelectionContext = entity;
	}

	// ===================================================================================
	// --- 私有辅助 (Private Helpers) ---
	// ===================================================================================

	// 绘制单个实体节点
	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		// 设置节点标志位：如果被选中则高亮，支持箭头展开
		ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
			ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		
		// 绘制树节点
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectionContext = entity;
		}

		// 右键点击实体弹出删除菜单
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
				entityDeleted = true;

			ImGui::EndPopup();
		}

		if (opened)
		{
			// TODO: 实现父子级层级显示 (目前 AFEngine 实体系统是平坦的，暂不支持父子关系)
			ImGui::TreePop();
		}

		// 处理删除操作 (放在绘制逻辑之后，防止在迭代 Registry 时进行删除导致崩溃)
		if (entityDeleted)
		{
			m_Context->DestroyEntity(entity);
			if (m_SelectionContext == entity)
				m_SelectionContext = {};
		}
	}

}

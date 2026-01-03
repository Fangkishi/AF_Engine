#include "ContentBrowserPanel.h"

#include "AF/Project/Project.h"

#include <imgui.h>
#include <string>
#include <algorithm>

namespace AF {
	
	// 默认资产目录
	constexpr char* s_AssetsDirectory = "assets";

	// ===================================================================================
	// --- 生命周期 (Lifecycle) ---
	// ===================================================================================

	ContentBrowserPanel::ContentBrowserPanel()
		: m_BaseDirectory(s_AssetsDirectory), m_CurrentDirectory(m_BaseDirectory)
	{
		// 加载资产浏览器图标
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");

		PreloadAssets();
	}

	// ===================================================================================
	// --- 核心逻辑 (Core Logic) ---
	// ===================================================================================

	void ContentBrowserPanel::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		// 1. 顶部导航栏
		RenderTopBar();

		// 2. 网格布局计算与内容绘制
		RenderItems();

		ImGui::End();
	}

	// ===================================================================================
	// --- 私有辅助 (Private Helpers) ---
	// ===================================================================================

	void ContentBrowserPanel::RenderTopBar()
	{
		// 显示当前路径并提供返回上级目录的按钮
		if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}
	}

	void ContentBrowserPanel::RenderItems()
	{
		float cellSize = m_ThumbnailSize + 8.0f;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		// 3. 遍历并绘制目录内容
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().string();

			ImGui::PushID(filenameString.c_str());

			// 根据文件类型选择显示图标
			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;

			if (!directoryEntry.is_directory())
			{
				std::string extension = path.extension().string();
				std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

				if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".tga" || extension == ".hdr" || extension == ".psd")
				{
					std::string pathStr = path.string();
					if (TextureLibrary::Exists(pathStr))
					{
						icon = TextureLibrary::GetTexture(pathStr);
					}
					else
					{
						TextureLibrary::LoadTexture(pathStr);
						if (TextureLibrary::Exists(pathStr))
							icon = TextureLibrary::GetTexture(pathStr);
					}
				}
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton((ImTextureID)icon->GetRendererID(), { m_ThumbnailSize, m_ThumbnailSize }, { 0, 1 }, { 1, 0 });

			// 4. 处理拖放源 (Drag & Drop Source)
			// 允许将资产文件拖拽到其他面板 (如视口或属性面板)
			if (ImGui::BeginDragDropSource())
			{
				std::filesystem::path relativePath(path);
				const wchar_t* itemPath = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, (wcslen(itemPath) + 1) * sizeof(wchar_t));

				ImGui::Text("Loading asset: %s", filenameString.c_str());

				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();

			// 5. 处理交互
			// 双击文件夹进入，双击文件可扩展为打开文件
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
				{
					m_CurrentDirectory /= path.filename();
				}
			}

			// 显示文件名 (自动换行)
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);
	}

	void ContentBrowserPanel::PreloadAssets()
	{
		if (!std::filesystem::exists(m_BaseDirectory))
			return;

		for (auto& directoryEntry : std::filesystem::recursive_directory_iterator(m_BaseDirectory))
		{
			if (!directoryEntry.is_directory())
			{
				auto path = directoryEntry.path();
				std::string extension = path.extension().string();
				std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

				if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".tga" || extension == ".hdr" || extension == ".psd")
				{
					TextureLibrary::LoadTexture(path.string());
				}
			}
		}
	}

}

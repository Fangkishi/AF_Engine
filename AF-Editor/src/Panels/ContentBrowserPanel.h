#pragma once

#include "AF/Renderer/Texture.h"

#include <filesystem>
#include <map>

namespace AF {

	/**
	* @brief 内容浏览器面板
	* 负责显示项目资产目录，支持文件导航和资源拖放。
	*/
	class ContentBrowserPanel
	{
	public:
		ContentBrowserPanel();

		/**
		 * @brief 渲染 ImGui 界面
		 */
		void OnImGuiRender();
	private:
		// --- UI 渲染辅助函数 ---
		void RenderTopBar();
		void RenderItems();

		std::filesystem::path m_BaseDirectory;    // 资产根目录
		std::filesystem::path m_CurrentDirectory; // 当前导航目录

		Ref<Texture2D> m_DirectoryIcon; // 文件夹图标
		Ref<Texture2D> m_FileIcon;      // 文件图标

		float m_ThumbnailSize = 128.0f;

		void PreloadAssets();
	};

}

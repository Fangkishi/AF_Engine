#pragma once

#include <AF.h>
#include "AF/Core/Layer.h"
#include "AF/Renderer/API/Texture.h"
#include "AF/Core/Timestep.h"

#include "AF/Scene/Scene.h"
#include "AF/Scene/Entity.h"
#include "TetrisGame.h"

namespace AF {
	class Scene;
	class Entity;
}

class Sandbox2D : public AF::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(AF::Timestep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(AF::Event& e) override;

private:
	void RenderTetrisGame();
	void RenderTetrisMenu(TetrisGame* tetrisGame);
	void RenderTetrisBoard(TetrisGame* tetrisGame);
	void RenderGameOver(TetrisGame* tetrisGame);

	AF::Ref<AF::Texture2D> m_CheckerboardTexture;
	glm::vec4 m_SquareColor;

	AF::Ref<AF::Scene> m_Scene;
	AF::Entity m_TetrisEntity;
};
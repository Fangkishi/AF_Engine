#include "Sandbox2D.h"
#include "TetrisGame.h"
#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <AF.h>

Sandbox2D::Sandbox2D()
    : AF::Layer("Sandbox2D"), m_SquareColor({ 0.2f, 0.3f, 0.8f, 1.0f })
{
}

void Sandbox2D::OnAttach()
{
    AF_PROFILE_FUNCTION();

    m_CheckerboardTexture = AF::Texture2D::Create("assets/textures/defaultTexture.jpg");

    // Create scene
    m_Scene = AF::CreateRef<AF::Scene>();

    // Create an entity for our Tetris game
    m_TetrisEntity = m_Scene->CreateEntity("TetrisGame");
    auto& nsc = m_TetrisEntity.AddComponent<AF::NativeScriptComponent>();
    nsc.Bind<TetrisGame>();

    // Add a camera to the scene
    auto cameraEntity = m_Scene->CreateEntity("Camera");
    auto& cc = cameraEntity.AddComponent<AF::CameraComponent>();
    cc.Primary = true;

    // Set orthographic projection to see the game
    // Game board is 10 wide and 20 high, total visible range
    cc.Camera->SetOrthographic(20.0f, -1.0f, 1.0f); // Field of view range
    cc.Camera->SetViewportSize(1280, 720);

    // Set position to ensure game visibility
    auto& cameraTransform = cameraEntity.GetComponent<AF::TransformComponent>();
    cameraTransform.Translation = glm::vec3(0.0f, 0.0f, 10.0f); // Move camera

    // Initialize the scene
    m_Scene->OnRuntimeStart();

    AF_INFO("Sandbox2D attached, camera setup complete");
}

void Sandbox2D::OnDetach()
{

}

void Sandbox2D::OnUpdate(AF::Timestep ts)
{
    AF_PROFILE_FUNCTION();

    // Clear the screen
    AF::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
    AF::RenderCommand::Clear();

    // Update the scene
    if (m_Scene)
    {
        m_Scene->UpdateScripts(ts);
    }

    // Reset stats
    AF::Renderer2D::ResetStats();

    if (m_Scene)
    {

        auto view = m_Scene->GetAllEntitiesWithView<AF::CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComponent = view.get<AF::CameraComponent>(entity);
            if (cameraComponent.Primary)
            {
                AF::Renderer2D::BeginScene(cameraComponent.Camera); // ʹõλΪͼ

                RenderTetrisGame();

                AF::Renderer2D::EndScene();
                break;
            }
        }
    }
}

void Sandbox2D::OnImGuiRender()
{
    AF_PROFILE_FUNCTION();

    ImGui::Begin("Tetris Game");

    if (m_TetrisEntity && m_TetrisEntity.HasComponent<AF::NativeScriptComponent>())
    {
        auto& scriptComponent = m_TetrisEntity.GetComponent<AF::NativeScriptComponent>();
        if (scriptComponent.Instance)
        {
            TetrisGame* tetrisGame = dynamic_cast<TetrisGame*>(scriptComponent.Instance);
            if (tetrisGame)
            {
                TetrisGame::GameState gameState = tetrisGame->GetGameState();
                switch (gameState)
                {
                case TetrisGame::GameState::Menu:
                    ImGui::Text("Welcome to Tetris!");
                    ImGui::Text("Controls:");
                    ImGui::BulletText("A - Move Left");
                    ImGui::BulletText("D - Move Right");
                    ImGui::BulletText("S - Move Down");
                    ImGui::BulletText("W - Rotate");
                    ImGui::BulletText("Space - Hard Drop");
                    break;

                case TetrisGame::GameState::Playing:
                    ImGui::Text("Score: %d", tetrisGame->GetScore());
                    break;

                case TetrisGame::GameState::GameOver:
                    ImGui::Text("Game Over!");
                    ImGui::Text("Final Score: %d", tetrisGame->GetScore());
                    break;
                }
            }
        }
    }

    auto stats = AF::Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("Quads: %d", stats.QuadCount);

    ImGui::End();
}

void Sandbox2D::OnEvent(AF::Event& e)
{
    AF_PROFILE_FUNCTION();

    AF::EventDispatcher dispatcher(e);
    dispatcher.Dispatch<AF::MouseButtonPressedEvent>([this](AF::MouseButtonPressedEvent& e) {
        if (m_TetrisEntity && m_TetrisEntity.HasComponent<AF::NativeScriptComponent>())
        {
            auto& scriptComponent = m_TetrisEntity.GetComponent<AF::NativeScriptComponent>();
            if (scriptComponent.Instance)
            {
                TetrisGame* tetrisGame = dynamic_cast<TetrisGame*>(scriptComponent.Instance);
                if (tetrisGame && tetrisGame->GetGameState() == TetrisGame::GameState::Menu)
                {
                    // 在菜单状态下，任何鼠标点击都应该开始游戏
                    tetrisGame->StartGame();
                }
                else if (tetrisGame && tetrisGame->GetGameState() == TetrisGame::GameState::GameOver)
                {
                    // 在游戏结束状态下，任何鼠标点击都应该重置游戏
                    tetrisGame->ResetGame();
                }
            }
        }
        return false;
        });
}

void Sandbox2D::RenderTetrisGame()
{
    if (!m_TetrisEntity || !m_TetrisEntity.HasComponent<AF::NativeScriptComponent>())
        return;

    auto& scriptComponent = m_TetrisEntity.GetComponent<AF::NativeScriptComponent>();
    if (!scriptComponent.Instance)
        return;

    TetrisGame* tetrisGame = dynamic_cast<TetrisGame*>(scriptComponent.Instance);
    if (!tetrisGame)
        return;

    TetrisGame::GameState gameState = tetrisGame->GetGameState();

    // 根据游戏状态渲染不同的界面
    switch (gameState)
    {
    case TetrisGame::GameState::Menu:
        RenderTetrisMenu(tetrisGame);
        break;

    case TetrisGame::GameState::Playing:
        RenderTetrisBoard(tetrisGame);
        break;

    case TetrisGame::GameState::GameOver:
        RenderTetrisBoard(tetrisGame);
        RenderGameOver(tetrisGame);
        break;
    }
}

void Sandbox2D::RenderTetrisMenu(TetrisGame* tetrisGame)
{
    // 渲染标题
    std::string title = "TETRIS";
    float titleWidth = title.length() * 0.6f; // 估算宽度
    float startX = -titleWidth / 2.0f;

    for (size_t i = 0; i < title.length(); i++)
    {
        char c = title[i];
        float posX = startX + i * 0.6f;
        AF::Renderer2D::DrawQuad({ posX, 2.0f, 0.5f }, { 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    }

    // 渲染开始提示
    std::string startText = "Click or Press ENTER/SPACE";
    float textWidth = startText.length() * 0.3f;
    startX = -textWidth / 2.0f;

    for (size_t i = 0; i < startText.length(); i++)
    {
        char c = startText[i];
        float posX = startX + i * 0.3f;
        AF::Renderer2D::DrawQuad({ posX, -1.0f, 0.5f }, { 0.25f, 0.25f }, { 1.0f, 1.0f, 0.0f, 1.0f });
    }

    // 渲染控制说明
    std::string controls[] = {
        "CONTROLS:",
        "A - Left",
        "D - Right",
        "S - Down",
        "W - Rotate",
        "SPACE - Drop"
    };

    for (int i = 0; i < 6; i++)
    {
        const std::string& text = controls[i];
        float textWidth = text.length() * 0.2f;
        float startX = -textWidth / 2.0f;
        float posY = -3.0f - i * 0.4f;

        for (size_t j = 0; j < text.length(); j++)
        {
            char c = text[j];
            float posX = startX + j * 0.2f;
            AF::Renderer2D::DrawQuad({ posX, posY, 0.5f }, { 0.15f, 0.15f }, { 0.8f, 0.8f, 0.8f, 1.0f });
        }
    }
}

void Sandbox2D::RenderGameOver(TetrisGame* tetrisGame)
{
    // 渲染半透明覆盖层
    AF::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.8f },
        { static_cast<float>(TetrisGame::BOARD_WIDTH), static_cast<float>(TetrisGame::BOARD_HEIGHT) },
        { 0.0f, 0.0f, 0.0f, 0.7f });

    // 渲染游戏结束文本
    std::string gameOverText = "GAME OVER";
    float textWidth = gameOverText.length() * 0.4f;
    float startX = -textWidth / 2.0f;

    for (size_t i = 0; i < gameOverText.length(); i++)
    {
        char c = gameOverText[i];
        float posX = startX + i * 0.4f;
        AF::Renderer2D::DrawQuad({ posX, 1.0f, 0.9f }, { 0.35f, 0.35f }, { 1.0f, 0.0f, 0.0f, 1.0f });
    }

    // 渲染最终分数
    std::string scoreText = "Score: " + std::to_string(tetrisGame->GetScore());
    float scoreWidth = scoreText.length() * 0.3f;
    startX = -scoreWidth / 2.0f;

    for (size_t i = 0; i < scoreText.length(); i++)
    {
        char c = scoreText[i];
        float posX = startX + i * 0.3f;
        AF::Renderer2D::DrawQuad({ posX, 0.0f, 0.9f }, { 0.25f, 0.25f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    }

    // 渲染重新开始提示
    std::string restartText = "Click or Press ENTER";
    float restartWidth = restartText.length() * 0.3f;
    startX = -restartWidth / 2.0f;

    for (size_t i = 0; i < restartText.length(); i++)
    {
        char c = restartText[i];
        float posX = startX + i * 0.3f;
        AF::Renderer2D::DrawQuad({ posX, -1.0f, 0.9f }, { 0.25f, 0.25f }, { 1.0f, 1.0f, 0.0f, 1.0f });
    }
}

void Sandbox2D::RenderTetrisBoard(TetrisGame* tetrisGame)
{
    const auto& board = tetrisGame->GetBoard();

    // Draw the placed blocks
    for (int y = 0; y < TetrisGame::BOARD_HEIGHT; y++)
    {
        for (int x = 0; x < TetrisGame::BOARD_WIDTH; x++)
        {
            if (board[y][x] != TetrominoType::None)
            {
                float posX = static_cast<float>(x);
                float posY = static_cast<float>(y);

                // Center the board
                posX -= TetrisGame::BOARD_WIDTH / 2.0f - 0.5f;
                posY -= TetrisGame::BOARD_HEIGHT / 2.0f - 0.5f;

                glm::vec4 color;
                switch (board[y][x])
                {
                case TetrominoType::I: color = { 0.0f, 1.0f, 1.0f, 1.0f }; break;
                case TetrominoType::O: color = { 1.0f, 1.0f, 0.0f, 1.0f }; break;
                case TetrominoType::T: color = { 1.0f, 0.0f, 1.0f, 1.0f }; break;
                case TetrominoType::S: color = { 0.0f, 1.0f, 0.0f, 1.0f }; break;
                case TetrominoType::Z: color = { 1.0f, 0.0f, 0.0f, 1.0f }; break;
                case TetrominoType::J: color = { 0.0f, 0.0f, 1.0f, 1.0f }; break;
                case TetrominoType::L: color = { 1.0f, 0.5f, 0.0f, 1.0f }; break;
                default: color = { 1.0f, 1.0f, 1.0f, 1.0f };
                }

                AF::Renderer2D::DrawQuad({ posX, posY, 0.0f }, { 0.9f, 0.9f }, color);
            }
        }
    }

    // Draw the current tetromino
    const auto& currentTetromino = tetrisGame->GetCurrentTetromino();
    if (currentTetromino.Type != TetrominoType::None)
    {
        int shapeSize = static_cast<int>(currentTetromino.Shape.size());

        for (int y = 0; y < shapeSize; y++)
        {
            for (int x = 0; x < shapeSize; x++)
            {
                if (currentTetromino.Shape[y][x] == 1)
                {
                    int boardX = currentTetromino.Position.x + x - shapeSize / 2;
                    int boardY = currentTetromino.Position.y + y - shapeSize / 2;

                    float posX = static_cast<float>(boardX);
                    float posY = static_cast<float>(boardY);
                    posX -= TetrisGame::BOARD_WIDTH / 2.0f - 0.5f;
                    posY -= TetrisGame::BOARD_HEIGHT / 2.0f - 0.5f;

                    AF::Renderer2D::DrawQuad({ posX, posY, 0.1f }, { 0.9f, 0.9f }, currentTetromino.Color);
                }
            }
        }
    }

    // Draw the grid
    glm::vec3 start, end;

    // Draw vertical lines
    for (int x = 0; x <= TetrisGame::BOARD_WIDTH; x++)
    {
        float posX = static_cast<float>(x) - TetrisGame::BOARD_WIDTH / 2.0f;
        start = { posX, -TetrisGame::BOARD_HEIGHT / 2.0f, 0.0f };
        end = { posX, TetrisGame::BOARD_HEIGHT / 2.0f, 0.0f };
        AF::Renderer2D::DrawLine(start, end, { 0.3f, 0.3f, 0.3f, 1.0f });
    }

    // Draw horizontal lines
    for (int y = 0; y <= TetrisGame::BOARD_HEIGHT; y++)
    {
        float posY = static_cast<float>(y) - TetrisGame::BOARD_HEIGHT / 2.0f;
        start = { -TetrisGame::BOARD_WIDTH / 2.0f, posY, 0.0f };
        end = { TetrisGame::BOARD_WIDTH / 2.0f, posY, 0.0f };
        AF::Renderer2D::DrawLine(start, end, { 0.3f, 0.3f, 0.3f, 0.5f });
    }
}
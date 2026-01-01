#include "TetrisGame.h"
#include "AF/Core/Input.h"
#include "AF/Core/KeyCodes.h"

TetrisGame::TetrisGame()
	: m_RandomGenerator(std::random_device{}())
	, m_TetrominoDistribution(0, 6)
{
}

void TetrisGame::OnCreate()
{
	InitBoard();
	// 不再在创建时初始化方块，等待游戏开始
	m_NextTetromino.Type = TetrominoType::None;
}

void TetrisGame::OnDestroy()
{
}

void TetrisGame::OnUpdate(AF::Timestep ts)
{
	// Handle input for menu and game over states
	bool enterKeyDown = AF::Input::IsKeyPressed(AF::Key::Enter);
	bool spaceKeyDown = AF::Input::IsKeyPressed(AF::Key::Space);

	if (m_GameState == GameState::Menu)
	{
		// 在菜单状态，按回车键开始游戏
		if (enterKeyDown && !m_SpacePressed) // 重用SpacePressed变量避免重复触发
		{
			StartGame();
			m_SpacePressed = true;
		}
		else if (!enterKeyDown)
		{
			m_SpacePressed = false;
		}
		return;
	}

	if (m_GameState == GameState::GameOver)
	{
		// 在游戏结束状态，按回车键重新开始
		if (enterKeyDown && !m_SpacePressed)
		{
			ResetGame();
			m_SpacePressed = true;
		}
		else if (!enterKeyDown)
		{
			m_SpacePressed = false;
		}
		return;
	}

	// Handle input with timing controls
	bool leftKeyDown = AF::Input::IsKeyPressed(AF::Key::A);
	bool rightKeyDown = AF::Input::IsKeyPressed(AF::Key::D);
	bool downKeyDown = AF::Input::IsKeyPressed(AF::Key::S);
	bool upKeyDown = AF::Input::IsKeyPressed(AF::Key::W);

	// 处理左移
	if (leftKeyDown)
	{
		if (!m_LeftPressed)
		{
			// 立即移动
			MoveTetromino(-1, 0);
			m_LeftPressed = true;
			m_MoveTime = 0.0f;
		}
		else
		{
			// 延迟重复移动
			m_MoveTime += ts;
			if (m_MoveTime >= m_MoveDelay)
			{
				MoveTetromino(-1, 0);
				m_MoveTime = 0.0f;
			}
		}
	}
	else
	{
		m_LeftPressed = false;
	}

	// 处理右移
	if (rightKeyDown)
	{
		if (!m_RightPressed)
		{
			// 立即移动
			MoveTetromino(1, 0);
			m_RightPressed = true;
			m_MoveTime = 0.0f;
		}
		else
		{
			// 延迟重复移动
			m_MoveTime += ts;
			if (m_MoveTime >= m_MoveDelay)
			{
				MoveTetromino(1, 0);
				m_MoveTime = 0.0f;
			}
		}
	}
	else
	{
		m_RightPressed = false;
	}

	// 处理下移
	if (downKeyDown)
	{
		if (!m_DownPressed)
		{
			MoveTetromino(0, -1);
			m_DownPressed = true;
			m_MoveTime = 0.0f;
		}
		else
		{
			m_MoveTime += ts;
			if (m_MoveTime >= m_MoveDelay)
			{
				MoveTetromino(0, -1);
				m_MoveTime = 0.0f;
			}
		}
	}
	else
	{
		m_DownPressed = false;
	}

	// 处理旋转
	if (upKeyDown)
	{
		if (!m_UpPressed)
		{
			RotateTetromino(true); // 顺时针旋转
			m_UpPressed = true;
			m_RotateTime = 0.0f;
		}
		else
		{
			m_RotateTime += ts;
			if (m_RotateTime >= m_RotateDelay)
			{
				RotateTetromino(true); // 顺时针旋转
				m_RotateTime = 0.0f;
			}
		}
	}
	else
	{
		m_UpPressed = false;
	}

	// 处理瞬降
	if (spaceKeyDown)
	{
		if (!m_SpacePressed)
		{
			DropTetromino();
			m_SpacePressed = true;
		}
	}
	else
	{
		m_SpacePressed = false;
	}

	// Automatic drop
	m_DropTime += ts;
	if (m_DropTime >= m_DropSpeed)
	{
		m_DropTime = 0.0f;
		MoveTetromino(0, -1);
	}
}

void TetrisGame::InitBoard()
{
	m_Board.resize(BOARD_HEIGHT, std::vector<TetrominoType>(BOARD_WIDTH, TetrominoType::None));
}

void TetrisGame::StartGame()
{
	m_GameState = GameState::Playing;
	m_Score = 0;
	m_GameOver = false;
	InitBoard();
	NewTetromino();
	m_NextTetromino.Type = TetrominoType::None;
}

void TetrisGame::ResetGame()
{
	m_GameState = GameState::Playing;
	m_Score = 0;
	m_GameOver = false;

	// 清空游戏板
	for (int y = 0; y < BOARD_HEIGHT; y++)
	{
		for (int x = 0; x < BOARD_WIDTH; x++)
		{
			m_Board[y][x] = TetrominoType::None;
		}
	}

	// 重置当前和下一个方块
	m_CurrentTetromino = {};
	m_NextTetromino = {};

	// 生成新的方块
	NewTetromino();
	if (m_NextTetromino.Type == TetrominoType::None)
	{
		m_NextTetromino.Type = static_cast<TetrominoType>(m_TetrominoDistribution(m_RandomGenerator));
		m_NextTetromino.Position = { BOARD_WIDTH / 2, BOARD_HEIGHT - 1 };
	}
}

void TetrisGame::NewTetromino()
{
	// If we don't have a next tetromino, generate one
	if (m_NextTetromino.Type == TetrominoType::None)
	{
		m_NextTetromino.Type = static_cast<TetrominoType>(m_TetrominoDistribution(m_RandomGenerator));
		m_NextTetromino.Position = { BOARD_WIDTH / 2, BOARD_HEIGHT - 1 };
	}

	// Set current tetromino to next tetromino
	m_CurrentTetromino = m_NextTetromino;

	// Generate new next tetromino
	m_NextTetromino.Type = static_cast<TetrominoType>(m_TetrominoDistribution(m_RandomGenerator));
	m_NextTetromino.Position = { BOARD_WIDTH / 2, BOARD_HEIGHT - 1 };

	// Set shape and color based on type
	switch (m_CurrentTetromino.Type)
	{
	case TetrominoType::I:
		m_CurrentTetromino.Shape = {
			{0, 0, 0, 0},
			{1, 1, 1, 1},
			{0, 0, 0, 0},
			{0, 0, 0, 0}
		};
		m_CurrentTetromino.Color = { 0.0f, 1.0f, 1.0f, 1.0f }; // Cyan
		break;
	case TetrominoType::O:
		m_CurrentTetromino.Shape = {
			{1, 1},
			{1, 1}
		};
		m_CurrentTetromino.Color = { 1.0f, 1.0f, 0.0f, 1.0f }; // Yellow
		break;
	case TetrominoType::T:
		m_CurrentTetromino.Shape = {
			{0, 1, 0},
			{1, 1, 1},
			{0, 0, 0}
		};
		m_CurrentTetromino.Color = { 1.0f, 0.0f, 1.0f, 1.0f }; // Purple
		break;
	case TetrominoType::S:
		m_CurrentTetromino.Shape = {
			{0, 1, 1},
			{1, 1, 0},
			{0, 0, 0}
		};
		m_CurrentTetromino.Color = { 0.0f, 1.0f, 0.0f, 1.0f }; // Green
		break;
	case TetrominoType::Z:
		m_CurrentTetromino.Shape = {
			{1, 1, 0},
			{0, 1, 1},
			{0, 0, 0}
		};
		m_CurrentTetromino.Color = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red
		break;
	case TetrominoType::J:
		m_CurrentTetromino.Shape = {
			{1, 0, 0},
			{1, 1, 1},
			{0, 0, 0}
		};
		m_CurrentTetromino.Color = { 0.0f, 0.0f, 1.0f, 1.0f }; // Blue
		break;
	case TetrominoType::L:
		m_CurrentTetromino.Shape = {
			{0, 0, 1},
			{1, 1, 1},
			{0, 0, 0}
		};
		m_CurrentTetromino.Color = { 1.0f, 0.5f, 0.0f, 1.0f }; // Orange
		break;
	}
}

void TetrisGame::MoveTetromino(int dx, int dy)
{
	glm::ivec2 newPosition = { m_CurrentTetromino.Position.x + dx, m_CurrentTetromino.Position.y + dy };

	if (!CheckCollision(m_CurrentTetromino, newPosition))
	{
		m_CurrentTetromino.Position = newPosition;
	}
	else if (dy < 0) // If moving down and collision occurs
	{
		LockTetromino();
		ClearLines();
		NewTetromino();

		// Check if game over
		if (CheckCollision(m_CurrentTetromino, m_CurrentTetromino.Position))
		{
			GameOver();
		}
	}
}

void TetrisGame::RotateTetromino(bool clockwise)
{
	// Create a rotated version of the shape
	Tetromino rotated = m_CurrentTetromino;
	int size = rotated.Shape.size();

	if (clockwise)
	{
		// 顺时针旋转
		// 先转置矩阵
		for (int i = 0; i < size; i++)
		{
			for (int j = i; j < size; j++)
			{
				std::swap(rotated.Shape[i][j], rotated.Shape[j][i]);
			}
		}

		// 然后反转行
		for (int i = 0; i < size / 2; i++)
		{
			std::swap(rotated.Shape[i], rotated.Shape[size - 1 - i]);
		}
	}
	else
	{
		// 逆时针旋转
		// 先转置矩阵
		for (int i = 0; i < size; i++)
		{
			for (int j = i; j < size; j++)
			{
				std::swap(rotated.Shape[i][j], rotated.Shape[j][i]);
			}
		}

		// 然后反转列
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size / 2; j++)
			{
				std::swap(rotated.Shape[i][j], rotated.Shape[i][size - 1 - j]);
			}
		}
	}

	// 检查旋转是否有效，如果无效则尝试踢墙
	if (!CheckCollision(rotated, m_CurrentTetromino.Position))
	{
		m_CurrentTetromino.Shape = rotated.Shape;
	}
	else
	{
		// 尝试踢墙 - 向左移动一格
		if (!CheckCollision(rotated, { m_CurrentTetromino.Position.x - 1, m_CurrentTetromino.Position.y }))
		{
			m_CurrentTetromino.Shape = rotated.Shape;
			m_CurrentTetromino.Position.x -= 1;
		}
		// 尝试踢墙 - 向右移动一格
		else if (!CheckCollision(rotated, { m_CurrentTetromino.Position.x + 1, m_CurrentTetromino.Position.y }))
		{
			m_CurrentTetromino.Shape = rotated.Shape;
			m_CurrentTetromino.Position.x += 1;
		}
		// 尝试踢墙 - 向左移动两格
		else if (!CheckCollision(rotated, { m_CurrentTetromino.Position.x - 2, m_CurrentTetromino.Position.y }))
		{
			m_CurrentTetromino.Shape = rotated.Shape;
			m_CurrentTetromino.Position.x -= 2;
		}
		// 尝试踢墙 - 向右移动两格
		else if (!CheckCollision(rotated, { m_CurrentTetromino.Position.x + 2, m_CurrentTetromino.Position.y }))
		{
			m_CurrentTetromino.Shape = rotated.Shape;
			m_CurrentTetromino.Position.x += 2;
		}
	}
}

void TetrisGame::DropTetromino()
{
	while (!CheckCollision(m_CurrentTetromino, { m_CurrentTetromino.Position.x, m_CurrentTetromino.Position.y - 1 }))
	{
		m_CurrentTetromino.Position.y--;
	}
	MoveTetromino(0, -1); // This will trigger the locking
}

bool TetrisGame::CheckCollision(const Tetromino& tetromino, const glm::ivec2& position)
{
	int shapeSize = tetromino.Shape.size();

	for (int y = 0; y < shapeSize; y++)
	{
		for (int x = 0; x < shapeSize; x++)
		{
			// Check if the block is part of the tetromino
			if (tetromino.Shape[y][x] == 1)
			{
				int boardX = position.x + x - shapeSize / 2;
				int boardY = position.y + y - shapeSize / 2;

				// Check if out of bounds
				if (boardX < 0 || boardX >= BOARD_WIDTH || boardY < 0)
				{
					return true;
				}

				// Check if collides with existing blocks
				if (boardY < BOARD_HEIGHT && m_Board[boardY][boardX] != TetrominoType::None)
				{
					return true;
				}
			}
		}
	}

	return false;
}

void TetrisGame::LockTetromino()
{
	int shapeSize = m_CurrentTetromino.Shape.size();

	for (int y = 0; y < shapeSize; y++)
	{
		for (int x = 0; x < shapeSize; x++)
		{
			// Check if the block is part of the tetromino
			if (m_CurrentTetromino.Shape[y][x] == 1)
			{
				int boardX = m_CurrentTetromino.Position.x + x - shapeSize / 2;
				int boardY = m_CurrentTetromino.Position.y + y - shapeSize / 2;

				// Place the block on the board
				if (boardX >= 0 && boardX < BOARD_WIDTH && boardY >= 0 && boardY < BOARD_HEIGHT)
				{
					m_Board[boardY][boardX] = m_CurrentTetromino.Type;
				}
			}
		}
	}
}

void TetrisGame::ClearLines()
{
	int linesCleared = 0;

	for (int y = 0; y < BOARD_HEIGHT; y++)
	{
		bool fullLine = true;
		for (int x = 0; x < BOARD_WIDTH; x++)
		{
			if (m_Board[y][x] == TetrominoType::None)
			{
				fullLine = false;
				break;
			}
		}

		if (fullLine)
		{
			linesCleared++;

			// Move all lines above down
			for (int ty = y; ty < BOARD_HEIGHT - 1; ty++)
			{
				for (int x = 0; x < BOARD_WIDTH; x++)
				{
					m_Board[ty][x] = m_Board[ty + 1][x];
				}
			}

			// Clear the top line
			for (int x = 0; x < BOARD_WIDTH; x++)
			{
				m_Board[BOARD_HEIGHT - 1][x] = TetrominoType::None;
			}

			// Recheck the same line index since we moved everything down
			y--;
		}
	}

	// Update score
	m_Score += linesCleared * 100;
}

void TetrisGame::GameOver()
{
	m_GameState = GameState::GameOver;
	// In a real implementation, you would display a game over message
}
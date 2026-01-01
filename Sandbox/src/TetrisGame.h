#pragma once

#include "AF/Scene/ScriptableEntity.h"
#include "AF/Core/Timestep.h"
#include "AF/Scene/Components.h"
#include <vector>
#include <random>

enum class TetrominoType
{
	None = -1,
	I = 0,
	O = 1,
	T = 2,
	S = 3,
	Z = 4,
	J = 5,
	L = 6
};

struct Tetromino
{
	TetrominoType Type = TetrominoType::None;
	glm::vec4 Color;
	std::vector<std::vector<int>> Shape;
	glm::ivec2 Position;
};

class TetrisGame : public AF::ScriptableEntity
{
public:
	static const int BOARD_WIDTH = 10;
	static const int BOARD_HEIGHT = 20;

	enum class GameState
	{
		Menu,
		Playing,
		GameOver
	};

public:
	TetrisGame();
	virtual ~TetrisGame() = default;

	virtual void OnCreate() override;
	virtual void OnDestroy() override;
	virtual void OnUpdate(AF::Timestep ts) override;

	// 获取游戏状态的方法
	const std::vector<std::vector<TetrominoType>>& GetBoard() const { return m_Board; }
	const Tetromino& GetCurrentTetromino() const { return m_CurrentTetromino; }
	const Tetromino& GetNextTetromino() const { return m_NextTetromino; }
	bool IsGameOver() const { return m_GameState == GameState::GameOver; }
	int GetScore() const { return m_Score; }
	GameState GetGameState() const { return m_GameState; }
	void StartGame();
	void ResetGame();

private:
	void InitBoard();
	void NewTetromino();
	void MoveTetromino(int dx, int dy);
	void RotateTetromino(bool clockwise = true);
	void DropTetromino();
	bool CheckCollision(const Tetromino& tetromino, const glm::ivec2& position);
	void LockTetromino();
	void ClearLines();
	void GameOver();

private:
	std::vector<std::vector<TetrominoType>> m_Board;
	Tetromino m_CurrentTetromino;
	Tetromino m_NextTetromino;

	float m_DropTime = 0.0f;
	float m_DropSpeed = 1.0f; // 1 second per drop

	// 控制相关的计时器
	float m_MoveTime = 0.0f;
	float m_MoveDelay = 0.2f; // 延迟移动
	float m_RotateTime = 0.0f;
	float m_RotateDelay = 0.2f; // 延迟旋转

	int m_Score = 0;
	GameState m_GameState = GameState::Menu;

	std::mt19937 m_RandomGenerator;
	std::uniform_int_distribution<int> m_TetrominoDistribution;

	// 按键状态追踪
	bool m_LeftPressed = false;
	bool m_RightPressed = false;
	bool m_DownPressed = false;
	bool m_UpPressed = false;
	bool m_SpacePressed = false;
	bool m_GameOver = false;
};
#pragma once
#include "ai_player.hpp"

struct Player
{
  Player(int id, AIPlayer* ai = nullptr) : id(id), ai(ai) {}
  ~Player() { delete ai; }
  int id;
  AIPlayer* ai;
};

enum
{
  GAME_END_DRAW = -2,
  NO_WINNER = 0,
};

struct WinningMove
{
  WinningMove() : player(NO_WINNER) {}
  WinningMove(int player, int x, int y, int dirX, int dirY)
      : player(player), x(x), y(y), dirX(dirX), dirY(dirY)
  {
  }
  int player;
  int x, y;
  int dirX, dirY;
};

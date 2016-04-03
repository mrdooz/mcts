#pragma once
#include "game_types.hpp"

//------------------------------------------------------------------------------
enum
{
  BOARD_WIDTH = 20,
  BOARD_HEIGHT = 10,
  WIN_LENGTH = 5,
};

//------------------------------------------------------------------------------
struct GameState;
struct Board
{
  Board();
  bool InsideBoard(int row, int col) const;
  bool ValidMove(int col) const;
  vector<int> GetValidMoves() const;
  bool EmptySlot(int row, int col) const;

  bool ApplyMove(int col, char player);
  char& At(int row, int col);
  char At(int row, int col) const;
  int LongestLine(int row, int col, int dirX, int dirY) const;
  WinningMove Winner() const;
  bool IsBoardFull() const;

  char state[BOARD_HEIGHT * BOARD_WIDTH];
};

#include "board.hpp"
#include "game_state.hpp"

extern SDL_Window* g_window;
extern SDL_Renderer* g_renderer;

static const char UNUSED_CELL = 0;

//------------------------------------------------------------------------------
Board::Board()
{
  memset(state, UNUSED_CELL, sizeof(state));
}

//------------------------------------------------------------------------------
bool Board::InsideBoard(int row, int col) const
{
  return row >= 0 && row < BOARD_HEIGHT && col >= 0 && col < BOARD_WIDTH;
}

//------------------------------------------------------------------------------
bool Board::ValidMove(int col) const
{
  return EmptySlot(0, col);
}

//------------------------------------------------------------------------------
vector<int> Board::GetValidMoves() const
{
  vector<int> res;
  res.reserve(BOARD_WIDTH);
  for (int i = 0; i < BOARD_WIDTH; ++i)
  {
    if (ValidMove(i))
      res.push_back(i);
  }
  return res;
}

//------------------------------------------------------------------------------
bool Board::EmptySlot(int row, int col) const
{
  return InsideBoard(row, col) && At(row, col) == UNUSED_CELL;
}

//------------------------------------------------------------------------------
bool Board::ApplyMove(int col, char player)
{
  // don't allow the move if the column is full
  if (!ValidMove(col))
    return false;

  // determine how far the piece will fall
  int row = 0;
  while (EmptySlot(row + 1, col))
  {
    row += 1;
  }

  At(row, col) = player;

  return true;
}

//------------------------------------------------------------------------------
char& Board::At(int row, int col)
{
  return state[row * BOARD_WIDTH + col];
}

//------------------------------------------------------------------------------
char Board::At(int row, int col) const
{
  return state[row * BOARD_WIDTH + col];
}

//------------------------------------------------------------------------------
int Board::LongestLine(int row, int col, int dirX, int dirY) const
{
  char player = At(row, col);
  if (player == UNUSED_CELL)
    return 0;

  int res = 0;
  do
  {
    res += 1;
    row += dirY;
    col += dirX;
  } while (InsideBoard(row, col) && At(row, col) == player);

  return res;
}

//------------------------------------------------------------------------------
bool Board::IsBoardFull() const
{
  for (int i = 0; i < BOARD_HEIGHT * BOARD_WIDTH; ++i)
  {
    if (state[i] == UNUSED_CELL)
      return false;
  }
  return true;
}

//------------------------------------------------------------------------------
WinningMove Board::Winner() const
{
  for (int i = 0; i < BOARD_HEIGHT; ++i)
  {
    for (int j = 0; j < BOARD_WIDTH; ++j)
    {
      int dir[] = {0, 1, 1, 0, 1, 1, -1, 1};
      for (int n = 0; n < 4; ++n)
      {
        if (LongestLine(i, j, dir[n * 2 + 0], dir[n * 2 + 1]) >= WIN_LENGTH)
        {
          return WinningMove{At(i, j), j, i, dir[n * 2 + 0], dir[n * 2 + 1]};
        }
      }
    }
  }

  return WinningMove();
}

#include "minimax.hpp"
#include "board.hpp"
#include "game_state.hpp"
#if 0

static unordered_map<string, int> g_BoardHash;
static const int MAX_HASH_SIZE = 32 * 1024 * 1024;

//------------------------------------------------------------------------------
static int DoMiniMax(
    const Board& oldBoard, bool maxPlayer, int minValue, int maxValue, int* bestMoveOut)
{
  {

    // Check if leaf node
    if (oldBoard.IsBoardFull())
      return 0;

    GameState state;
    WinningMove w = oldBoard.Winner();
    //char player = state.winningMove.player;
    if (w.player == PLAYER_HUMAN)
      return 100;
    if (w.player == PLAYER_AI)
      return -100;
  }

  int bestMove = -1;
  int bestScore = maxPlayer ? minValue : maxValue;
  int cand;
  if (maxPlayer)
  {
    for (int i = 0; i < BOARD_WIDTH; ++i)
    {
      Board newBoard(oldBoard);
      if (newBoard.ApplyMove(i, PLAYER_HUMAN))
      {
        const string& h = newBoard.ToHash();
        auto it = g_BoardHash.find(h);
        if (it != g_BoardHash.end())
        {
          cand = it->second;
        }
        else
        {
          cand = DoMiniMax(newBoard, false, bestScore, maxValue, nullptr);
          if (g_BoardHash.size() < MAX_HASH_SIZE)
            g_BoardHash[h] = cand;
        }

        if (cand > maxValue)
          return maxValue;

        if (cand > bestScore)
        {
          bestScore = cand;
          bestMove = i;
        }
        if (cand == 100)
          break;
      }
    }
  }
  else
  {
    for (int i = 0; i < BOARD_WIDTH; ++i)
    {
      Board newBoard(oldBoard);
      if (newBoard.ApplyMove(i, PLAYER_AI))
      {
        const string& h = newBoard.ToHash();
        auto it = g_BoardHash.find(h);
        if (it != g_BoardHash.end())
        {
          cand = it->second;
        }
        else
        {
          cand = DoMiniMax(newBoard, true, minValue, bestScore, nullptr);
          if (g_BoardHash.size() < MAX_HASH_SIZE)
            g_BoardHash[h] = cand;
        }

        if (cand < minValue)
          return minValue;

        if (cand < bestScore)
        {
          bestScore = cand;
          bestMove = i;
        }
        if (cand == -100)
          break;
      }
    }
  }

  if (bestMoveOut)
    *bestMoveOut = bestMove;

  return bestScore;
}

//------------------------------------------------------------------------------
void MiniMax::ApplyMove(Board& board)
{
  int move;
  int res = DoMiniMax(board, false, INT_MIN, INT_MAX, &move);
  board.ApplyMove(move, PLAYER_AI);
}
#endif

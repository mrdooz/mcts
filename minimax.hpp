#pragma once
#include "ai_player.hpp"

// NB: minimax is going to be broken, because I'm switching away from the 2 player requirement, and
// the -neg/+pos scoring system (because MCTS doesn't need it)

#if 0
struct MiniMax : public AIPlayer
{
  virtual void ApplyMove(Board& board);
};
#endif
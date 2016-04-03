#include "ai_player.hpp"
#include "board.hpp"
#include "game_state.hpp"

//------------------------------------------------------------------------------
void RandomPlayer::Think(GameState* state)
{
  while (true)
  {
    int move = rand() % BOARD_WIDTH;
    if (state->board.ValidMove(move))
    {
      state->board.ApplyMove(move, playerId);
      return;
    }
  }
}

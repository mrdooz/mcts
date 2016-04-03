#pragma once

struct GameState;

//------------------------------------------------------------------------------
struct AIPlayer
{
  AIPlayer(int playerId) : playerId(playerId) {}
  virtual ~AIPlayer() {};
  virtual void Think(GameState* state) = 0;

  int playerId;
};

//------------------------------------------------------------------------------
struct RandomPlayer : public AIPlayer
{
  RandomPlayer(int playerId) : AIPlayer(playerId) {}
  virtual void Think(GameState* state);
};

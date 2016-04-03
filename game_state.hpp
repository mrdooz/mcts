#pragma once
#include "board.hpp"
#include "game_types.hpp"

template <typename T>
struct CycleVector
{
  CycleVector(const vector<T>& data) : data(data), idx(0) {}

  void Next() { idx = (idx + 1) % data.size(); }
  size_t Size() const { return data.size(); }

  const T& Cur() { return data[idx]; }

  vector<T> data;
  size_t idx;
};

struct GameState
{
  ~GameState()
  {
    for (Player* p : players.data)
      delete p;
  }
  GameState(const vector<Player*>& players) : players(players) { winningMove.player = NO_WINNER; }
  Board board;

  WinningMove winningMove;
  CycleVector<Player*> players;
  vector<int> moves;
};

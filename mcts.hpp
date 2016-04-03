#pragma once
#include "ai_player.hpp"
#include "board.hpp"

struct MCTS : public AIPlayer
{
  MCTS(int playerId);
  ~MCTS();
  virtual void Think(GameState* state);

  enum 
  {
    MAX_TREE_NODES = 4 * 1024 * 1024,
    NUM_BUFFER_NODES = BOARD_WIDTH*BOARD_HEIGHT,
    TOTAL_NODES = MAX_TREE_NODES + NUM_BUFFER_NODES,
  };

  struct TreeNode
  {
    TreeNode* parent;
    TreeNode* children[BOARD_WIDTH];
    Board board;
    int numPlayed;
    int numWon;
    // NB: `player` means whose turn it is to play, so the states where that player has moved are the children
    // of the current node.
    int player;
  };

  TreeNode* AddNode(TreeNode* parent, const Board& board, int player);

  TreeNode* FindExpansionNode(GameState* state);
  void SimulateFromNode(TreeNode* node, GameState* state);
  int BestMove();

  bool CompactTree(GameState* state);
  TreeNode* CompactNode(TreeNode* node, TreeNode* parent, TreeNode* nodes);

  TreeNode* nodeBufs[2];
  int curBuf = 0;
  int nodesUsed = 0;
  bool doReset = false;
};

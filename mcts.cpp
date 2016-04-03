/*
  Monte Carlo Tree Search.

  Based on: http://andysalerno.com/2016/03/Monte-Carlo-Reversi
*/
#include "mcts.hpp"
#include "board.hpp"
#include "game_state.hpp"

#define WITH_REFINMENT 1

//------------------------------------------------------------------------------
MCTS::MCTS(int playerId) : AIPlayer(playerId)
{
  nodeBufs[0] = new TreeNode[TOTAL_NODES];
  nodeBufs[1] = new TreeNode[TOTAL_NODES];
  memset(nodeBufs[0], 0, sizeof(TreeNode) * TOTAL_NODES);
  memset(nodeBufs[1], 0, sizeof(TreeNode) * TOTAL_NODES);
  curBuf = 0;
}

//------------------------------------------------------------------------------
MCTS::~MCTS()
{
  delete[] nodeBufs[0];
  delete[] nodeBufs[1];
}

//------------------------------------------------------------------------------
void MCTS::Think(GameState* state)
{
#if WITH_REFINMENT
  if (nodesUsed == 0 || doReset)
  {
    nodesUsed = 0;
    memset(nodeBufs[curBuf], 0, sizeof(TreeNode) * TOTAL_NODES);

    // Create the first node
    AddNode(nullptr, state->board, playerId);
  }
  else
  {
    if (!CompactTree(state))
    {
      AddNode(nullptr, state->board, playerId);
    }
  }
#else
  nodesUsed = 0;
  memset(nodeBufs[curBuf], 0, sizeof(TreeNode) * TOTAL_NODES);

  // Create the first node
  AddNode(nullptr, state->board, playerId);

#endif

  u32 startTime = timeGetTime();
  u32 elapsedTime = 0;
  int runs = 0;
  while (nodesUsed < MAX_TREE_NODES && elapsedTime < 2500)
  {
    // NB: we compare runs here, in case we run boards with less than 1000 states, in which
    // case this won't be trigged if we compare against nodesUsed
    if ((runs++ % 1000) == 0)
    {
      elapsedTime = timeGetTime() - startTime;
    }

    // MCTS executes the following 4 steps each run:
    // 1) selection - find an unexpanded child node, using UCB1 to determine the path to traverse
    // 2) expansion - create the new child
    // 3) simulation - choose random moves from the new child until we reach an end state for the game
    // 4) back propagation - propagate the results from the end state up to the root

    TreeNode* node = FindExpansionNode(state);
    SimulateFromNode(node, state);
  }

  int bestMove = BestMove();  
  state->board.ApplyMove(bestMove, playerId);
  state->moves.push_back(bestMove);
}

//------------------------------------------------------------------------------
MCTS::TreeNode* MCTS::FindExpansionNode(GameState* state)
{
  TreeNode* nodes = nodeBufs[curBuf];
  TreeNode* node = &nodes[0];

  while (true)
  {
    float logParentPlayed = node->numPlayed ? (float)log(node->numPlayed) : 0;

    // either select the child with the best UCB1, or the first unexpanded child
    float bestChildScore = 0.f;
    TreeNode* bestChild = nullptr;
    int unvisitedChildren[BOARD_WIDTH];
    int numUnvisitedChildren = 0;
    int numValidMoves = 0;
    for (int i = 0; i < BOARD_WIDTH; ++i)
    {
      if (!node->board.ValidMove(i))
        continue;

      numValidMoves++;
      if (TreeNode* curChild = node->children[i])
      {
        // upper confidence bound
        float C = 2.0f;
        float childScore = curChild->numWon / curChild->numPlayed
          + sqrtf(C + logParentPlayed / curChild->numPlayed);
        if (childScore > bestChildScore || !bestChild)
        {
          bestChild = curChild;
          bestChildScore = childScore;
        }
      }
      else
      {
        // found unexpanded child
        unvisitedChildren[numUnvisitedChildren++] = i;
      }
    }

    if (numUnvisitedChildren)
    {
      // Found unexpanded child, so assign it to the next player, and update its state
      int numPlayers = (int)state->players.Size();
      int move = unvisitedChildren[rand() % numUnvisitedChildren];
      Board newBoard = node->board;
      newBoard.ApplyMove(move, node->player);
      int nextPlayer = 1 + (node->player % numPlayers);
      TreeNode* leafNode = AddNode(node, newBoard, nextPlayer);
      node->children[move] = leafNode;
      return leafNode;
    }
    else if (numValidMoves > 0)
    {
      node = bestChild;
    }
    else
    {
      // no valid moves, so use the last node as the leaf node, and break
      return node;
    }
  }

  return nullptr;
}

//------------------------------------------------------------------------------
void MCTS::SimulateFromNode(TreeNode* node, GameState* state)
{
  // Randomly simulate while we're not in an end state
  Board board = node->board;
  int numPlayers = (int)state->players.Size();
  int player = node->player;
  while (board.Winner().player == NO_WINNER && !board.IsBoardFull())
  {
    // pick random move
    vector<int> validMoves = board.GetValidMoves();
    int move = validMoves[rand() % validMoves.size()];
    board.ApplyMove(move, player);
    // create a new tree node for the current state
    player = 1 + (player % numPlayers);
    TreeNode* newNode = AddNode(node, board, player);
    node->children[move] = newNode;
    node = newNode;
  }
  // back propagation
  int winningPlayer = board.Winner().player;
  while (node)
  {
    node->numPlayed++;
    if (node->parent && winningPlayer == node->parent->player)
      node->numWon++;
    node = node->parent;
  }
}

//------------------------------------------------------------------------------
MCTS::TreeNode* MCTS::AddNode(TreeNode* parent, const Board& board, int player)
{
  TreeNode* nodes = nodeBufs[curBuf];
  TreeNode* newNode = &nodes[nodesUsed++];
  newNode->parent = parent;
  newNode->board = board;
  newNode->player = player;
  return newNode;
}

//------------------------------------------------------------------------------
int MCTS::BestMove()
{
  TreeNode* nodes = nodeBufs[curBuf];

  struct SortNode
  {
    int numPlayed;
    int numWon;
    int idx;
  };

  int numSortNodes = 0;
  SortNode sortNodes[BOARD_WIDTH];
  for (int i = 0; i < BOARD_WIDTH; ++i)
  {
    TreeNode* node = nodes[0].children[i];
    if (node)
      sortNodes[numSortNodes++] = SortNode{ node->numPlayed, node->numWon, i };
  }

  sort(sortNodes, sortNodes + numSortNodes, [](const SortNode& lhs, const SortNode& rhs)
  {
    return lhs.numWon / max(1.0f, (float)lhs.numPlayed) > rhs.numWon / max(1.0f, (float)rhs.numPlayed);
  });

  for (int i = 0; i < numSortNodes; ++i)
  {
    printf("%d: %d/%d\n", sortNodes[i].idx, sortNodes[i].numWon, sortNodes[i].numPlayed);
  }
  printf("%d total nodes\n", nodes[0].numPlayed);

  return sortNodes[0].idx;
}

//------------------------------------------------------------------------------
MCTS::TreeNode* MCTS::CompactNode(TreeNode* node, TreeNode* parent, TreeNode* nodes)
{
  // Copy over the fields we want to the new node
  TreeNode* newNode = nodes + nodesUsed;
  newNode->parent = parent;
  newNode->board = node->board;
  newNode->numPlayed = node->numPlayed;
  newNode->numWon = node->numWon;
  newNode->player = node->player;
  nodesUsed++;

  for (int i = 0; i < BOARD_WIDTH; ++i)
  {
    if (node->children[i])
    {
      newNode->children[i] = CompactNode(node->children[i], newNode, nodes);
    }
  }

  return newNode;
}

//------------------------------------------------------------------------------
bool MCTS::CompactTree(GameState* state)
{
  // Compact the tree, by making the current board state the new root, and copying in
  // all children, creating a new tree buffer just containing "live" children.
  TreeNode* src = nodeBufs[curBuf];
  TreeNode* dst = nodeBufs[curBuf ^ 1];

  memset(dst, 0, sizeof(TreeNode) * TOTAL_NODES);

  // BFS to find the node holding the current game state
  deque<TreeNode*> nodes;
  TreeNode* root = nullptr;
  nodes.push_back(&src[0]);
  while (!nodes.empty())
  {
    TreeNode* node = nodes.front();
    nodes.pop_front();

    if (memcmp(node->board.state, state->board.state, sizeof(state->board)) == 0)
    {
      root = node;
      break;
    }

    for (int i = 0; i < BOARD_WIDTH; ++i)
    {
      if (node->children[i])
        nodes.push_back(node->children[i]);
    }
  }

  nodesUsed = 0;
  if (root)
  {
    CompactNode(root, nullptr, dst);
  }
  curBuf ^= 1;

  return root != nullptr;
}


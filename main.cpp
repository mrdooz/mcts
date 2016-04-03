#include "ai_player.hpp"
#include "board.hpp"
#include "game_state.hpp"
#include "mcts.hpp"
#include "minimax.hpp"
#include "sdl_utils.hpp"

static int SCREEN_WIDTH = 640;
static int SCREEN_HEIGHT = 300;

SDL_Window* g_window = nullptr;
SDL_Renderer* g_renderer = nullptr;

//------------------------------------------------------------------------------
void RenderBoard(int marginX, int marginY, int playerOfs, const GameState& state)
{
  int canvasWidth = SCREEN_WIDTH - 2 * marginX;
  int canvasHeight = SCREEN_HEIGHT - 2 * marginY;
  int padding = 2;

  int cellWidth = (canvasWidth - (padding * (BOARD_WIDTH - 1))) / BOARD_WIDTH;
  int cellHeight = (canvasHeight - (padding * (BOARD_HEIGHT - 1))) / BOARD_HEIGHT;
  int cellSize = min(cellWidth, cellHeight);
  int cellAndPad = cellSize + padding;

  bool humanPlayers = false;
  for (const Player* p : state.players.data)
  {
    if (!p->ai)
    {
      humanPlayers = true;
      break;
    }
  }

  SDL_Rect rect{marginX, marginY, SCREEN_WIDTH - 2 * marginX, SCREEN_HEIGHT - 2 * marginY};
  SDL_SetRenderDrawColor(g_renderer, 20, 20, 20, 0);
  SDL_RenderFillRect(g_renderer, &rect);

  if (humanPlayers)
  {
    // render player pos
    SDL_Rect rect{marginX + playerOfs * cellAndPad, max(0, marginY - cellHeight), cellSize, cellSize};
    if (state.board.ValidMove(playerOfs))
    {
      SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 0);
    }
    else
    {
      SDL_SetRenderDrawColor(g_renderer, 255, 0, 0, 0);
    }

    SDL_RenderDrawRect(
        g_renderer, &SDL_Rect{marginX + playerOfs * cellAndPad, marginY, cellSize, canvasHeight});
  }

  unordered_map<int, vector<SDL_Rect>> rectsByPlayerId;
  for (int i = 0; i < BOARD_HEIGHT; ++i)
  {
    for (int j = 0; j < BOARD_WIDTH; ++j)
    {
      int x = marginX + j * cellAndPad;
      int y = marginY + i * cellAndPad;
      if (!state.board.EmptySlot(i, j))
      {
        char ch = state.board.At(i, j);
        rectsByPlayerId[ch].push_back(SDL_Rect{x, y, cellSize, cellSize});
      }
    }
  }

  unordered_map<int, vector<int>> playerCols = {
      {1, {200, 0, 0}}, {2, {200, 200, 0}}, {3, {200, 0, 200}}, {4, {0, 200, 0}},
  };

  for (auto& kv : rectsByPlayerId)
  {
    int id = kv.first;
    const vector<SDL_Rect>& rects = kv.second;

    if (playerCols.count(id))
    {
      const vector<int>& cols = playerCols[id];
      SDL_SetRenderDrawColor(g_renderer, cols[0], cols[1], cols[2], 0);
      SDL_RenderFillRects(g_renderer, rects.data(), (int)rects.size());
    }
  }

  if (state.winningMove.player != NO_WINNER)
  {
    SDL_SetRenderDrawColor(g_renderer, 255, 255, 255, 0);
    SDL_Rect rects[WIN_LENGTH];
    for (int i = 0; i < WIN_LENGTH; ++i)
    {
      rects[i] = SDL_Rect{marginX + cellSize * (state.winningMove.x + state.winningMove.dirX * i),
          marginY + cellSize * (state.winningMove.y + state.winningMove.dirY * i),
          cellSize,
          cellSize};
    }
    SDL_RenderDrawRects(g_renderer, rects, WIN_LENGTH);
  }
}

//------------------------------------------------------------------------------
int main(int argc, char** argv)
{
  srand(1337);

  if (SDL_Init(SDL_INIT_VIDEO) != 0)
  {
    printf("SDL_Init Error: %s\n", SDL_GetError());
    return 1;
  }

  if (TTF_Init() != 0)
  {
    SDL_Quit();
    return 1;
  }

  g_window = SDL_CreateWindow(
      "Hello World!", 50, 50, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  if (!g_window)
  {
    printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
    return 1;
  }

  g_renderer =
      SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!g_renderer)
  {
    SDL_DestroyWindow(g_window);
    printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_Color color = {255, 255, 255, 255};
  SDL_Texture* winnerTexture = nullptr;

  // clang-format off

  GameState state({
    new Player{ 1, nullptr },
    new Player{ 2, new MCTS(2)}});

  //GameState state({
  //  new Player{1, new MCTS{1}}, 
  //  new Player{2, new MCTS(2)}, 
  //  new Player{3, new MCTS(3)}});

  // clang-format on
  int dropPosition = 0;

  while (true)
  {
    const Player* curPlayer = state.players.Cur();

    WinningMove w = state.board.Winner();
    int winner = w.player;
    if (winner == NO_WINNER && state.board.IsBoardFull())
      winner = GAME_END_DRAW;

    bool done = false;
    int playerMove = -1;
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
      if (e.type == SDL_KEYUP)
      {
        SDL_Keycode key = e.key.keysym.sym;
        if (key == SDLK_ESCAPE)
        {
          done = true;
          break;
        }

        if (!curPlayer->ai && winner == NO_WINNER)
        {
          if (key == SDLK_LEFT)
          {
            dropPosition = dropPosition == 0 ? BOARD_WIDTH - 1 : dropPosition - 1;
          }
          else if (key == SDLK_RIGHT)
          {
            dropPosition = (dropPosition + 1) % BOARD_WIDTH;
          }
          else if (key == SDLK_RETURN)
          {
            if (!curPlayer->ai)
            {
              if (state.board.ValidMove(dropPosition))
                playerMove = dropPosition;
            }
          }
        }
      }
      else if (e.type == SDL_WINDOWEVENT)
      {
        if (e.window.event == SDL_WINDOWEVENT_RESIZED)
        {
          SCREEN_WIDTH = e.window.data1;
          SCREEN_HEIGHT = e.window.data2;
          SDL_SetWindowSize(g_window, e.window.data1, e.window.data2);
        }
      }
    }

    if (done)
      break;

    SDL_SetRenderDrawColor(g_renderer, 0, 0, 0, 0);
    SDL_RenderClear(g_renderer);

    if (winner == NO_WINNER)
    {
      if (!curPlayer->ai)
      {
        if (playerMove != -1)
        {
          state.board.ApplyMove(playerMove, curPlayer->id);
          state.moves.push_back(playerMove);
          state.players.Next();
        }
      }
      else
      {
        curPlayer->ai->Think(&state);
        state.players.Next();
      }
    }

    RenderBoard(20, 20, dropPosition, state);

    if (winner != NO_WINNER)
    {
      if (!winnerTexture)
      {
        char buf[256];
        if (winner == GAME_END_DRAW)
          sprintf_s(buf, sizeof(buf), "%s", "Draw!");
        else
          sprintf_s(buf, sizeof(buf), "Player %d wins!", winner);
        winnerTexture = RenderText(buf, "arial.ttf", color, 32);
      }

      int w, h;
      SDL_QueryTexture(winnerTexture, NULL, NULL, &w, &h);
      int x = SCREEN_WIDTH / 2 - w / 2;
      int y = SCREEN_HEIGHT / 2 - h / 2;
      SDL_RenderCopy(g_renderer, winnerTexture, NULL, &SDL_Rect{x, y, w, h});
    }

    SDL_RenderPresent(g_renderer);
  }

  SDL_DestroyRenderer(g_renderer);
  SDL_DestroyWindow(g_window);
  SDL_Quit();

  return 0;
}

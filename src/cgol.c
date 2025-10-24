#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

#define WIDTH  501
#define HEIGHT WIDTH

#define CELL_SIZE    25
#define SIMUL_PERIOD 50

#define MAX_GRID_SIZE 1000

static SDL_Window *window     = NULL;
static SDL_Renderer *renderer = NULL;

static unsigned char *cells = NULL, *back_cells = NULL;

static void
eprintln (const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  fprintf (stderr, "\n");
  va_end (args);
}

static void
set_padding (int realwidth, int realheight, int width, int height, int *lpad,
             int *tpad)
{
  if (realwidth > width)
    *lpad = (realwidth - width - 1) / 2;
  else
    *lpad = 0;

  if (realheight > height)
    *tpad = (realheight - height - 1) / 2;
  else
    *tpad = 0;
}

static void
cleanup (void)
{
  if (cells != NULL)
    free (cells);

  if (back_cells != NULL)
    free (back_cells);

  if (renderer)
    SDL_DestroyRenderer (renderer);

  if (window)
    SDL_DestroyWindow (window);
}

int
main (int argc, char *argv[])
{
  (void) argc;
  (void) argv;

  if (!SDL_Init (SDL_INIT_VIDEO))
    {
      eprintln ("failed to init SDL");
      goto fail;
    }

  if (!SDL_CreateWindowAndRenderer ("Conway's Game of Life", WIDTH, HEIGHT,
                                    SDL_WINDOW_RESIZABLE, &window, &renderer))
    {
      eprintln ("failed to create SDL window and renderer");
      goto fail;
    }

  SDL_SetWindowMinimumSize (window, CELL_SIZE + 1, CELL_SIZE + 1);

  SDL_Event event;

  int width = WIDTH, realwidth = WIDTH;
  int height = HEIGHT, realheight = HEIGHT;
  int lpad = 0, tpad = 0;

  if ((width - 1) / CELL_SIZE > MAX_GRID_SIZE)
    width = MAX_GRID_SIZE * CELL_SIZE + 1;

  if ((height - 1) / CELL_SIZE > MAX_GRID_SIZE)
    height = MAX_GRID_SIZE * CELL_SIZE + 1;

  set_padding (realwidth, realheight, width, height, &lpad, &tpad);

  int cw = (width - 1) / CELL_SIZE, ch = (height - 1) / CELL_SIZE;
  int resize_pending = 0, is_playing = 0;
  Uint64 resize_ticks = SDL_GetTicks (), game_ticks = SDL_GetTicks (),
         frame_ticks;

  size_t num_max_cells = MAX_GRID_SIZE * MAX_GRID_SIZE;

  cells      = malloc (num_max_cells);
  back_cells = malloc (num_max_cells);

  if (!cells || !back_cells)
    {
      eprintln ("out of memory");
      goto fail;
    }

  memset (cells, 0, num_max_cells);
  memset (back_cells, 0, num_max_cells);

  while (1)
    {
      while (SDL_PollEvent (&event))
        {
          if (event.type == SDL_EVENT_QUIT)
            goto stop;
          else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
            {
              if (event.button.button == SDL_BUTTON_RIGHT)
                {
                  is_playing = !is_playing;
                  game_ticks = SDL_GetTicks ();
                }

              if (event.button.button != SDL_BUTTON_LEFT)
                continue;

              int x = event.button.x;
              int y = event.button.y;

              if (x < lpad || y < tpad)
                continue;

              x -= lpad;
              y -= tpad;

              if (!(x % CELL_SIZE) || !(y % CELL_SIZE))
                continue;

              int cx = x / CELL_SIZE;
              int cy = y / CELL_SIZE;

              cells[cy * cw + cx] = !cells[cy * cw + cx];
            }
          else if (event.type == SDL_EVENT_WINDOW_RESIZED)
            {
              realwidth  = event.window.data1;
              realheight = event.window.data2;

              width  = (realwidth - 1) / CELL_SIZE * CELL_SIZE + 1;
              height = (realheight - 1) / CELL_SIZE * CELL_SIZE + 1;

              cw = (width - 1) / CELL_SIZE;

              if (cw > MAX_GRID_SIZE)
                {
                  cw    = MAX_GRID_SIZE;
                  width = cw * CELL_SIZE + 1;
                }

              ch = (height - 1) / CELL_SIZE;

              if (ch > MAX_GRID_SIZE)
                {
                  ch     = MAX_GRID_SIZE;
                  height = ch * CELL_SIZE + 1;
                }

              set_padding (realwidth, realheight, width, height, &lpad, &tpad);

              int tmpwidth
                  = (realwidth + CELL_SIZE - 2) / CELL_SIZE * CELL_SIZE + 1;
              int tmpheight
                  = (realheight + CELL_SIZE - 2) / CELL_SIZE * CELL_SIZE + 1;

              resize_pending = tmpwidth != width || tmpheight != height;

              if (resize_pending)
                resize_ticks = SDL_GetTicks ();
            }
        }

      frame_ticks = SDL_GetTicks ();

      if (resize_pending && frame_ticks - resize_ticks >= 100)
        {
          resize_ticks = frame_ticks;

          int tmpwidth
              = (realwidth + CELL_SIZE - 2) / CELL_SIZE * CELL_SIZE + 1;
          int tmpheight
              = (realheight + CELL_SIZE - 2) / CELL_SIZE * CELL_SIZE + 1;

          resize_pending = 0;

          if (tmpwidth != width || tmpheight != height)
            SDL_SetWindowSize (window, tmpwidth, tmpheight);

          is_playing = 0;
        }

      if (is_playing && frame_ticks - game_ticks >= SIMUL_PERIOD)
        {
          game_ticks = frame_ticks;

          for (int y = 0; y < ch; ++y)
            for (int x = 0; x < cw; ++x)
              {
                int num_neighbors = 0;
                int is_live       = cells[y * cw + x];

                for (int _y = -1; _y <= 1; ++_y)
                  for (int _x = -1; _x <= 1; ++_x)
                    {
                      if (!_y && !_x)
                        continue;

                      int cx = x + _x, cy = y + _y;

                      if (cx == -1)
                        cx = cw - 1;

                      if (cx == cw)
                        cx = 0;

                      if (cy == -1)
                        cy = ch - 1;

                      if (cy == ch)
                        cy = 0;

                      if (cells[cy * cw + cx])
                        ++num_neighbors;
                    }

                if (is_live)
                  back_cells[y * cw + x]
                      = num_neighbors == 2 || num_neighbors == 3;
                else
                  back_cells[y * cw + x] = num_neighbors == 3;
              }

          // we operate on a back buffer of the cells so as to not impede
          // calculations each simulation period
          unsigned char *tmpcells = cells;
          cells                   = back_cells;
          back_cells              = tmpcells;
        }

      SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);
      SDL_RenderClear (renderer);

      SDL_SetRenderDrawColor (renderer, 100, 100, 100, 255);

      for (int i = 0; i < width; i += CELL_SIZE)
        SDL_RenderLine (renderer, i + lpad, tpad, i + lpad, height + tpad - 1);

      for (int i = 0; i < height; i += CELL_SIZE)
        SDL_RenderLine (renderer, lpad, i + tpad, width + lpad - 1, i + tpad);

      SDL_SetRenderDrawColor (renderer, 255, 255, 255, 255);

      for (int y = 0; y < ch; ++y)
        for (int x = 0; x < cw; ++x)
          if (cells[y * cw + x])
            {
              SDL_FRect rect = {
                .x = x * CELL_SIZE + 1 + lpad,
                .y = y * CELL_SIZE + 1 + tpad,
                .w = CELL_SIZE - 1,
                .h = CELL_SIZE - 1,
              };
              SDL_RenderFillRect (renderer, &rect);
            }

      if (is_playing)
        {
          float progress = (frame_ticks - game_ticks) / (float) SIMUL_PERIOD;
          SDL_SetRenderDrawColor (renderer, 0, 255, 0, 255);
          SDL_RenderLine (renderer, 0, realheight - 1,
                          progress * (realwidth - 1), realheight - 1);
        }

      SDL_RenderPresent (renderer);
    }

stop:
  cleanup ();
  return 0;

fail:
  cleanup ();
  return -1;
}

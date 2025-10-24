#include <stdarg.h>
#include <stdio.h>

#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#define WIDTH  500
#define HEIGHT WIDTH

static SDL_Window *window     = NULL;
static SDL_Renderer *renderer = NULL;

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
SDL_Cleanup (void)
{
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

  SDL_Event event;

  SDL_SetRenderDrawColor (renderer, 0, 0, 0, 255);
  SDL_RenderClear (renderer);

  while (1)
    {
      while (SDL_PollEvent (&event))
        {
          if (event.type == SDL_EVENT_QUIT)
            goto stop;
        }

      SDL_RenderPresent (renderer);
    }

stop:
  SDL_Cleanup ();
  return 0;

fail:
  SDL_Cleanup ();
  return -1;
}

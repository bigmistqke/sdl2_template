#include <SDL2/SDL.h>
#include <iostream>

// Global variable for filter alpha
float FILTER_ALPHA = 0.01f;

int main()
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    std::cerr << "SDL Initialization Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow("Filter Alpha Control",
                                        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                        800, 600, SDL_WINDOW_SHOWN);
  if (!window)
  {
    std::cerr << "Window Creation Error: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  bool quit = false;
  SDL_Event event;

  while (!quit)
  {
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        quit = true;
      }
      else if (event.type == SDL_MOUSEMOTION)
      {
        int mouseY = event.motion.y;
        int windowHeight = 600; // Replace with your window's height
        FILTER_ALPHA = static_cast<float>(mouseY) / windowHeight;
        std::cout << "FILTER_ALPHA: " << FILTER_ALPHA << std::endl;
      }
    }
  }

  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

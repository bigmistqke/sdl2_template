c++ template with SDL2 and glew for macos (m1)

steps:
1. `brew install sdl2`
1. `brew install sdl2_ttf`
1. `brew install glew`
1. run example:
- `make run file=main`: no local dependencies: 
- `make run file=main2 deps=SDLAudioManager,SDLConsole`: with local dependencies
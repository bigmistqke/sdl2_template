c++ template with SDL2 and glew for macos (m1)

steps:
1. `brew install glew sdl2 sdl2_ttf sld2_mixer`
1. build and run examples:
- `make run file=gl`
- `make run file=synth deps=SDLAudioManager,SDLConsole`
- `make run file=echo`
- `make run file=lopass`

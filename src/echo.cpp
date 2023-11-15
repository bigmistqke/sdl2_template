#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

// Custom audio callback function to apply the echo effect
void AudioCallback(void* userdata, Uint8* stream, int len) {
  // Cast the userdata to access the delay buffer
  std::vector<Sint16>& delayBuffer = *static_cast<std::vector<Sint16>*>(userdata);

  // Process the audio stream as 16-bit samples (Sint16)
  Sint16* samples = reinterpret_cast<Sint16*>(stream);
  int sampleCount = len / sizeof(Sint16); // Number of 16-bit samples

  for (int i = 0; i < sampleCount; ++i) {
    int newSample = static_cast<int>(samples[i]) + static_cast<int>(delayBuffer[0]);
    newSample = (newSample > 32767) ? 32767 : (newSample < -32768 ? -32768 : newSample);

    // Apply the echo effect by adding the delayed audio
    samples[i] = newSample;

    // Push the current audio data to the delay buffer
    delayBuffer.push_back(samples[i] * 0.5);

    // Remove the oldest sample from the delay buffer
    delayBuffer.erase(delayBuffer.begin());
  }
}



int main() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        std::cerr << "SDL_mixer initialization failed: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    char* basePath = SDL_GetBasePath();
    // Define the relative path to your audio file from the base path
    const char* relativePath = "assets/audio.mp3"; // Adjust this path as needed

    // Construct the full path to the audio file
    std::string fullPath = std::string(basePath) + relativePath;

    // Load the audio file
    Mix_Music* music = Mix_LoadMUS(fullPath.c_str());
    
    if (!music) {
        std::cerr << "Failed to load audio: " << fullPath.c_str() << Mix_GetError() << std::endl;
        Mix_CloseAudio();
        SDL_Quit();
        return 1;
    }

    // Create an audio delay buffer
    const int delayBufferSize = 44100; // Adjust this for the desired delay length
    std::vector<Uint8> delayBuffer(delayBufferSize, 0);

    // Set the custom audio callback
    Mix_SetPostMix(AudioCallback, &delayBuffer);

    // Start playing the music
    if (Mix_PlayMusic(music, -1) == -1) { // -1 for looping indefinitely
      std::cerr << "Failed to play music: " << Mix_GetError() << std::endl;
      Mix_FreeMusic(music);
      Mix_CloseAudio();
      SDL_Quit();
      return 1;
    }

    // Main loop
    bool quit = false;
    while (!quit) {
      SDL_Event e;
      while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            quit = true;
        }
      }
    }

    // Clean up resources
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}

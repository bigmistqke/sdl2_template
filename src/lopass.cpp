#include <SDL2/SDL.h>
#include <iostream>

const int SAMPLE_RATE = 44100;
const int AUDIO_FORMAT = AUDIO_S16LSB;
const int AUDIO_CHANNELS = 2;
const int AUDIO_BUFFER_SIZE = 4096;

const float FILTER_ALPHA = 0.1f;

void applyLowPassFilter(Sint16 *buffer, int length)
{
  static float previousSample = 0.0f;

  for (int i = 0; i < length / sizeof(Sint16); ++i)
  {
    float currentSample = static_cast<float>(buffer[i]);
    float filteredSample = FILTER_ALPHA * currentSample + (1.0f - FILTER_ALPHA) * previousSample;
    buffer[i] = static_cast<Sint16>(filteredSample);
    previousSample = filteredSample;
  }
}

int main(int argc, char *argv[])
{
  if (SDL_Init(SDL_INIT_AUDIO) < 0)
  {
    std::cerr << "SDL Initialization Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_AudioSpec desiredSpec;
  desiredSpec.freq = SAMPLE_RATE;
  desiredSpec.format = AUDIO_FORMAT;
  desiredSpec.channels = AUDIO_CHANNELS;
  desiredSpec.samples = AUDIO_BUFFER_SIZE;

  SDL_AudioSpec obtainedSpec;

  SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, 0);
  if (audioDevice == 0)
  {
    std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  // Load the WAV file
  SDL_RWops *rw = SDL_RWFromFile("assets/moody.wav", "rb");
  if (!rw)
  {
    std::cerr << "Failed to open WAV file: " << SDL_GetError() << std::endl;
    SDL_CloseAudioDevice(audioDevice);
    SDL_Quit();
    return 1;
  }

  SDL_AudioSpec wavSpec;
  Uint8 *wavBuffer;
  Uint32 wavLength;

  if (SDL_LoadWAV_RW(rw, 1, &wavSpec, &wavBuffer, &wavLength) == nullptr)
  {
    std::cerr << "Failed to load WAV file: " << SDL_GetError() << std::endl;
    SDL_RWclose(rw);
    SDL_CloseAudioDevice(audioDevice);
    SDL_Quit();
    return 1;
  }

  // Start audio playback
  SDL_PauseAudioDevice(audioDevice, 0);

  // Main audio processing loop
  int bufferIndex = 0;
  bool quit = false;

  while (!quit)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT)
      {
        quit = true;
      }
    }

    // Process the audio buffer
    Sint16 *audioBuffer = reinterpret_cast<Sint16 *>(wavBuffer + bufferIndex);
    int audioLength = std::min(static_cast<int>(wavLength - bufferIndex), AUDIO_BUFFER_SIZE);

    if (audioLength > 0)
    {
      applyLowPassFilter(audioBuffer, audioLength);
      SDL_QueueAudio(audioDevice, audioBuffer, audioLength);
      bufferIndex += audioLength;
    }

    if (bufferIndex >= static_cast<int>(wavLength))
    {
      quit = true; // Audio playback finished
    }

    SDL_Delay(10); // Simulate some processing time
  }

  // Clean up and quit
  SDL_FreeWAV(wavBuffer);
  SDL_CloseAudioDevice(audioDevice);
  SDL_Quit();

  return 0;
}

#include <SDL2/SDL.h>
#include <iostream>
#include <mpg123.h>

const int SAMPLE_RATE = 48000;
const int AUDIO_FORMAT = AUDIO_S16LSB;
const int AUDIO_CHANNELS = 2;
const int AUDIO_BUFFER_SIZE = 4096 * 2;

const float FILTER_ALPHA = 0.01f;

bool loadMP3(const std::string &filename, std::vector<unsigned char> &audioBuffer, SDL_AudioSpec &mp3Spec)
{
  mpg123_handle *mpg123;
  int err;

  err = mpg123_init();
  if (err != MPG123_OK)
  {
    std::cerr << "Failed to initialize mpg123: " << mpg123_plain_strerror(err) << std::endl;
    return false;
  }

  mpg123 = mpg123_new(nullptr, &err);
  if (!mpg123)
  {
    std::cerr << "Failed to create mpg123 handle: " << mpg123_plain_strerror(err) << std::endl;
    return false;
  }

  if (mpg123_open(mpg123, filename.c_str()) != MPG123_OK)
  {
    std::cerr << "Failed to open MP3 file: " << mpg123_strerror(mpg123) << std::endl;
    mpg123_close(mpg123);
    mpg123_delete(mpg123);
    mpg123_exit();
    return false;
  }

  long rate;
  int channels, encoding;
  mpg123_getformat(mpg123, &rate, &channels, &encoding);

  // Setup SDL AudioSpec
  mp3Spec.freq = rate;
  mp3Spec.format = AUDIO_S16SYS; // Assuming the MP3 is in a standard format
  mp3Spec.channels = channels;
  mp3Spec.samples = 4096; // You can adjust this

  size_t done;
  unsigned char buffer[4096];
  while (mpg123_read(mpg123, buffer, sizeof(buffer), &done) == MPG123_OK)
  {
    audioBuffer.insert(audioBuffer.end(), buffer, buffer + done);
  }

  mpg123_close(mpg123);
  mpg123_delete(mpg123);
  mpg123_exit();

  return true;
}

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
  std::string audioBaseName = argc > 1 ? argv[1] : "moody.wav";
  std::string audioFileName = "assets/" + audioBaseName;

  if (SDL_Init(SDL_INIT_AUDIO) < 0)
  {
    std::cerr << "SDL Initialization Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  SDL_AudioSpec desiredSpec, obtainedSpec;
  desiredSpec.freq = SAMPLE_RATE;
  desiredSpec.format = AUDIO_FORMAT;
  desiredSpec.channels = AUDIO_CHANNELS;
  desiredSpec.samples = AUDIO_BUFFER_SIZE;

  SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, 0);
  if (audioDevice == 0)
  {
    std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return 1;
  }

  std::vector<unsigned char> mp3Buffer;
  Uint8 *wavBuffer;
  Uint32 wavLength;

  if (audioBaseName.substr(audioBaseName.find_last_of('.') + 1) == "mp3")
  {
    if (!loadMP3(audioFileName, mp3Buffer, obtainedSpec))
    {
      std::cerr << "Failed to load MP3 file." << std::endl;
      SDL_CloseAudioDevice(audioDevice);
      SDL_Quit();
      return 1;
    }
    wavBuffer = mp3Buffer.data();
    wavLength = static_cast<Uint32>(mp3Buffer.size());
  }
  else
  {
    SDL_RWops *rw = SDL_RWFromFile(audioFileName.c_str(), "rb");
    if (!rw)
    {
      std::cerr << "Failed to open WAV file: " << SDL_GetError() << std::endl;
      SDL_CloseAudioDevice(audioDevice);
      SDL_Quit();
      return 1;
    }
    if (SDL_LoadWAV_RW(rw, 1, &obtainedSpec, &wavBuffer, &wavLength) == nullptr)
    {
      std::cerr << "Failed to load WAV file: " << SDL_GetError() << std::endl;
      SDL_RWclose(rw);
      SDL_CloseAudioDevice(audioDevice);
      SDL_Quit();
      return 1;
    }
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

    /*  if (bufferIndex >= static_cast<int>(wavLength))
     {
       quit = true; // Audio playback finished
     } */

    // SDL_Delay(1); // Simulate some processing time
  }

  // Clean up and quit
  SDL_FreeWAV(wavBuffer);
  SDL_CloseAudioDevice(audioDevice);
  SDL_Quit();

  return 0;
}

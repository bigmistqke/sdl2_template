#include <mpg123.h>
#include <iostream>

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

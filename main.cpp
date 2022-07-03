#include <cerrno>
#include <fstream>
#include <iostream>
#include <alsa/asoundlib.h>

//Audio File Directory
#define AUDIO_FILE_DIR "/home/user/Documents/ExampleAudio.raw"

//System ID for Audio Device. Set to "default" to use the default system output.
#define AUDIO_DEV "plughw:0,0"

std::fstream audio_file;
unsigned int audio_file_size = 0;
unsigned int audio_file_pos = 0;

snd_pcm_t *audio_dev;
snd_pcm_uframes_t n_frames;

unsigned int buffer_size_bytes;
short *buffer_0 = NULL;
short *buffer_1 = NULL;
bool next_buf = false;

bool stop = false;

void buffer_load(void)
{
  if(audio_file_pos >= audio_file_size)
  {
    stop = true;
    return;
  }
  
  short *buf_in = NULL;
  if(next_buf) buf_in = buffer_1;
  else buf_in = buffer_0;
  
  audio_file.seekg(audio_file_pos);
  audio_file.read((char*) buf_in, buffer_size_bytes);
  audio_file_pos += buffer_size_bytes;
  
  next_buf = !next_buf;
  return;
}

void buffer_play(void)
{
  short *buf_out = NULL;
  if(next_buf) buf_out = buffer_0;
  else buf_out = buffer_1;
  
  int n_return = snd_pcm_writei(audio_dev, buf_out, n_frames);
  if(n_return == -EPIPE) snd_pcm_prepare(audio_dev);
  
  return;
}

void playback(void)
{
  buffer_load();
  while(!stop)
  {
    buffer_play();
    buffer_load();
  }
  
  return;
}

void buffer_malloc(void)
{
  buffer_size_bytes = 4*n_frames;
  buffer_0 = (short*) malloc(buffer_size_bytes);
  buffer_1 = (short*) malloc(buffer_size_bytes);
  return;
}

void buffer_free(void)
{
  free(buffer_0);
  free(buffer_1);
  return;
}

bool open_audio_file(void)
{
  audio_file.open(AUDIO_FILE_DIR, std::ios_base::in);
  if(audio_file.is_open())
  {
    audio_file.seekg(0, audio_file.end);
    audio_file_size = audio_file.tellg();
    audio_file_pos = 0;
    audio_file.seekg(audio_file_pos);
    return true;
  }
  
  return false;
}

bool audio_hw_init(void)
{
  int n_return;
  snd_pcm_hw_params_t *hw_params;
  
  n_return = snd_pcm_open(&audio_dev, AUDIO_DEV, SND_PCM_STREAM_PLAYBACK, 0);
  if(n_return < 0)
  {
    std::cout << "Error opening audio device\n";
    return false;
  }
  
  snd_pcm_hw_params_malloc(&hw_params);
  snd_pcm_hw_params_any(audio_dev, hw_params);
  
  n_return = snd_pcm_hw_params_set_access(audio_dev, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
  if(n_return < 0)
  {
    std::cout << "Error setting access to read/write interleaved\n";
    return false;
  }
  
  n_return = snd_pcm_hw_params_set_format(audio_dev, hw_params, SND_PCM_FORMAT_S16_LE);
  if(n_return < 0)
  {
    std::cout << "Error setting format to signed 16bit little-endian\n";
    return false;
  }
  
  n_return = snd_pcm_hw_params_set_channels(audio_dev, hw_params, 2);
  if(n_return < 0)
  {
    std::cout << "Error setting channels to stereo\n";
    return false;
  }
  
  unsigned int sample_rate = 44100;
  n_return = snd_pcm_hw_params_set_rate_near(audio_dev, hw_params, &sample_rate, 0);
  if(n_return < 0 || sample_rate < 44100)
  {
    std::cout << "Could not set sample rate to 44100 Hz\nAttempting to set sample rate to 48000 Hz\n";
    sample_rate = 48000;
    n_return = snd_pcm_hw_params_set_rate_near(audio_dev, hw_params, &sample_rate, 0);
    if(n_return < 0 || sample_rate < 48000)
    {
      std::cout << "Error setting sample rate\n";
      return false;
    }
  }
  
  n_return = snd_pcm_hw_params(audio_dev, hw_params);
  if(n_return < 0)
  {
    std::cout << "Error setting audio hardware parameters\n";
    return false;
  }
  
  snd_pcm_hw_params_get_period_size(hw_params, &n_frames, 0);
  snd_pcm_hw_params_free(hw_params);
  return true;
}

int main(int argc, char **argv)
{
  if(!audio_hw_init())
  {
    std::cout << "Error code: " << errno << "\nTerminated\n";
    return 0;
  }
  std::cout << "Audio hardware initialized\n";
  
  if(!open_audio_file())
  {
    std::cout << "Error opening audio file\nError code: " << errno << "\nTerminated\n";
    return 0;
  }
  std::cout << "Audio file is open\n";
  
  buffer_malloc();
  
  std::cout << "Playback started\n";
  playback();
  std::cout << "Playback finished\n";
  
  audio_file.close();
  snd_pcm_drain(audio_dev);
  snd_pcm_close(audio_dev);
  buffer_free();
  std::cout << "Terminated\n";
  
  return 0;
}

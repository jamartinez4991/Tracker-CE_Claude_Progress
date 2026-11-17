// Minimal host stub of the Teensy-style AudioStream, for unit-testing the DSP classes only.
#pragma once
#include <cstdint>
#include <cstring>
#include <vector>
#define AUDIO_BLOCK_SAMPLES 128
#define AUDIO_SAMPLE_RATE_EXACT 44117.64706f
#define AUDIO_CORE_RAM
#define AUDIO_CORE_FLASH
struct audio_block_t { int16_t data[AUDIO_BLOCK_SAMPLES]; };
class AudioStream {
public:
  AudioStream(unsigned char n, audio_block_t** q) : ninputs(n) { (void)q; }
  virtual void update() = 0;
  // test hooks
  audio_block_t* in[2] = {nullptr, nullptr};
  audio_block_t* out[2] = {nullptr, nullptr};
  int allocated = 0, released = 0;
  unsigned char ninputs;
  audio_block_t* receiveReadOnly(unsigned i = 0) { return in[i]; }
  audio_block_t* receiveWritable(unsigned i = 0) {
    if (!in[i]) return nullptr; auto* b = new audio_block_t; *b = *in[i]; allocated++; return b; }
  void transmit(audio_block_t* b, unsigned char idx = 0) { out[idx] = b; }
  void release(audio_block_t*) { released++; }
};

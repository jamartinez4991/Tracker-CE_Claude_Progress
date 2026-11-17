#ifndef AUDIOENGINE_AUDIOVOICE_H_
#define AUDIOENGINE_AUDIOVOICE_H_
#include "Audio.h"

class AudioVoice
{
  public:
    enum EnvelopeGeneratorType
    {
        kFilter,
        kWavetablePosition,
        kGranularLength = kWavetablePosition,
        kGranularPosition,
        kPanning,
        kFinetune,
        kEnvelopeGeneratorCount
    };

#ifdef PLATFORM_IMX
    enum VoiceChannels
    {
        kLeft,
        kRight,
        kChannelsCount
    };
#endif

    AudioVoice();
#ifndef PLATFORM_IMX
    AudioPlayMemory* getMemoryPlayer();
    AudioBitDepth* getBitDepth();
    AudioFilterStateVariable* getFilter();
    AudioAmplifier* getAmplifier();
    MonoToStereo* getMonoToStereo();
    AudioAnalyzeRMS* getRMS();
#else
    AudioPlayMemory* getMemoryPlayer(VoiceChannels channel);
    AudioBitDepth* getBitDepth(VoiceChannels channel);
    AudioFilterStateVariable* getFilter(VoiceChannels channel);
    AudioAmplifier* getAmplifier(VoiceChannels channel);
    AudioAmplifier* getPanningAmplifier(VoiceChannels channel);
    AudioAnalyzeRMS* getRMS(VoiceChannels channel);
#endif
    envelopeGenerator* getEnvelopeGenerator(EnvelopeGeneratorType type);
#ifdef MOD_SAMPLE_INSTRUMENT_FX
    // One processor per voice: 1 channel (original Tracker) or paired L/R (i.MX).
    AudioSampleRateReduction* getSampleRateReduction();
    AudioFilterDJ* getDjFilter();
#endif

  private:
#ifdef MOD_SAMPLE_INSTRUMENT_FX
#ifdef PLATFORM_IMX
    static constexpr uint8_t kFxChannels = 2;
#else
    static constexpr uint8_t kFxChannels = 1;
#endif
#endif
#ifndef PLATFORM_IMX
    AudioPlayMemory memoryPlayer;
    AudioBitDepth bitDepth;
    AudioFilterStateVariable filter;
#ifdef MOD_SAMPLE_INSTRUMENT_FX
    // Declared (= constructed = updated) in signal order: filter -> SRR -> DJ -> amplifier.
    AudioSampleRateReduction sampleRateReduction{ kFxChannels };
    AudioFilterDJ djFilter{ kFxChannels };
#endif
    AudioAmplifier amplifier;
    MonoToStereo monoToStereo;
    AudioAnalyzeRMS rms;
#else
    AudioPlayMemory memoryPlayer[VoiceChannels::kChannelsCount];
    AudioBitDepth bitDepth[VoiceChannels::kChannelsCount];
    AudioFilterStateVariable filter[VoiceChannels::kChannelsCount];
#ifdef MOD_SAMPLE_INSTRUMENT_FX
    AudioSampleRateReduction sampleRateReduction{ kFxChannels };
    AudioFilterDJ djFilter{ kFxChannels };
#endif
    AudioAmplifier amplifier[VoiceChannels::kChannelsCount];
    AudioAmplifier panningAmplifier[VoiceChannels::kChannelsCount];
    AudioAnalyzeRMS rms[VoiceChannels::kChannelsCount];
#endif

    envelopeGenerator envelope[kEnvelopeGeneratorCount];
    std::vector<AudioConnection*> connections;
};

#endif /* AUDIOENGINE_AUDIOVOICE_H_ */

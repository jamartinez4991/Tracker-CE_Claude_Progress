#include "AudioVoice.h"

#ifndef PLATFORM_IMX
AudioVoice::AudioVoice()
{
    connections.push_back(new AudioConnection(&memoryPlayer, &bitDepth));
    connections.push_back(new AudioConnection(&bitDepth, &filter));
#ifdef MOD_SAMPLE_INSTRUMENT_FX
    connections.push_back(new AudioConnection(&filter, &sampleRateReduction));
    connections.push_back(new AudioConnection(&sampleRateReduction, &djFilter));
    connections.push_back(new AudioConnection(&djFilter, &amplifier));
#else
    connections.push_back(new AudioConnection(&filter, &amplifier));
#endif
    connections.push_back(new AudioConnection(&amplifier, &monoToStereo));
    connections.push_back(new AudioConnection(&amplifier, &rms));
}
#else
AudioVoice::AudioVoice()
{
    for (int channel = 0; channel < VoiceChannels::kChannelsCount; channel++)
    {
        connections.push_back(new AudioConnection(&memoryPlayer[channel], &bitDepth[channel]));
        connections.push_back(new AudioConnection(&bitDepth[channel], &filter[channel]));
#ifdef MOD_SAMPLE_INSTRUMENT_FX
        // Paired stereo: channel index == input/output index of the shared processors.
        connections.push_back(
          new AudioConnection(&filter[channel], 0, &sampleRateReduction, channel));
        connections.push_back(
          new AudioConnection(&sampleRateReduction, channel, &djFilter, channel));
        connections.push_back(new AudioConnection(&djFilter, channel, &amplifier[channel], 0));
#else
        connections.push_back(new AudioConnection(&filter[channel], &amplifier[channel]));
#endif
        connections.push_back(new AudioConnection(&amplifier[channel], &panningAmplifier[channel]));
        connections.push_back(new AudioConnection(&amplifier[channel], &rms[channel]));
    }
}
#endif

#ifndef PLATFORM_IMX
AudioPlayMemory* AudioVoice::getMemoryPlayer()
{
    return &memoryPlayer;
}
#else
AudioPlayMemory* AudioVoice::getMemoryPlayer(VoiceChannels channel)
{
    return &memoryPlayer[channel];
}
#endif

#ifndef PLATFORM_IMX
AudioBitDepth* AudioVoice::getBitDepth()
{
    return &bitDepth;
}
#else
AudioBitDepth* AudioVoice::getBitDepth(VoiceChannels channel)
{
    return &bitDepth[channel];
}
#endif

#ifndef PLATFORM_IMX
AudioFilterStateVariable* AudioVoice::getFilter()
{
    return &filter;
}
#else
AudioFilterStateVariable* AudioVoice::getFilter(VoiceChannels channel)
{
    return &filter[channel];
}
#endif

#ifndef PLATFORM_IMX
AudioAmplifier* AudioVoice::getAmplifier()
{
    return &amplifier;
}
#else
AudioAmplifier* AudioVoice::getAmplifier(VoiceChannels channel)
{
    return &amplifier[channel];
}
#endif

#ifndef PLATFORM_IMX
MonoToStereo* AudioVoice::getMonoToStereo()
{
    return &monoToStereo;
}
#else
AudioAmplifier* AudioVoice::getPanningAmplifier(VoiceChannels channel)
{
    return &panningAmplifier[channel];
}
#endif

#ifndef PLATFORM_IMX
AudioAnalyzeRMS* AudioVoice::getRMS()
{
    return &rms;
}
#else
AudioAnalyzeRMS* AudioVoice::getRMS(VoiceChannels channel)
{
    return &rms[channel];
}
#endif

#ifdef MOD_SAMPLE_INSTRUMENT_FX
AudioSampleRateReduction* AudioVoice::getSampleRateReduction()
{
    return &sampleRateReduction;
}

AudioFilterDJ* AudioVoice::getDjFilter()
{
    return &djFilter;
}
#endif

envelopeGenerator* AudioVoice::getEnvelopeGenerator(EnvelopeGeneratorType type)
{
    return &envelope[type];
}

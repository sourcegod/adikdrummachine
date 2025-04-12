#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include <portaudio.h>

class AudioDriver {
public:
    AudioDriver();
    ~AudioDriver();

    PaError initialize();
    PaError openStream(int sampleRate, PaStreamCallback* callback, void* userData);
    PaError startStream();
    PaError stopStream();
    PaError closeStream();
    PaError terminate();

private:
    PaStream* stream_;
    bool is_initialized_;
};

#endif // AUDIODRIVER_H

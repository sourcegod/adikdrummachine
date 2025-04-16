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
    bool initAudioDriver(PaStreamCallback *callback, void *userData, int sampleRate, int framesPerBuffer);
    bool startAudioDriver();
    bool stopAudioDriver();
    bool closeAudioDriver(); // Nouvelle fonction

private:
    PaStream* stream_;
    bool is_initialized_;
    PaError lastError_;

};

#endif // AUDIODRIVER_H

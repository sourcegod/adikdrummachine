#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include <portaudio.h>

class AudioDriver {
public:
    AudioDriver();
    ~AudioDriver();

    bool initAudioDriver(PaStreamCallback *callback, void *userData, int sampleRate, int framesPerBuffer);
    bool startAudioDriver();
    bool stopAudioDriver();
    bool closeAudioDriver(); // Nouvelle fonction

private:
    PaStream* stream_;
    PaError lastError_;

};

#endif // AUDIODRIVER_H

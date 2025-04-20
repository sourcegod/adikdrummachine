#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include <portaudio.h>

class AudioDriver {
public:
    AudioDriver();
    ~AudioDriver();

    bool init(PaStreamCallback *callback, void *userData, int sampleRate, int framesPerBuffer);
    bool start();
    bool stop();
    bool close(); // Nouvelle fonction
    PaError getLastError() {  return lastError_; }

private:
    PaStream* stream_;
    PaError lastError_;

};

#endif // AUDIODRIVER_H

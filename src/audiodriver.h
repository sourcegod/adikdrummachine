#ifndef AUDIODRIVER_H
#define AUDIODRIVER_H

#include <portaudio.h>

class AudioDriver {
public:
    AudioDriver();
    ~AudioDriver();

    bool init(int numChannels, int sampleRate, int framesPerBuffer, void *userData);
    // bool init(int numChannels, int sampleRate, int framesPerBuffer, PaStreamCallback *callback, void *userData);
    bool start();
    bool stop();
    bool close(); // Nouvelle fonction
    PaError getLastError() {  return lastError_; }

private:
    PaStream* stream_;
    PaError lastError_;

    static int drumMachineCallback(const void *inputBuffer, 
                                 void *outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo *timeInfo,
                                 PaStreamCallbackFlags statusFlags,
                                 void *userData);


};
//==== End of class AudioDriver ====

#endif // AUDIODRIVER_H

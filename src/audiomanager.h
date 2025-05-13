#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "audiodriver.h"
#include <portaudio.h> // Assure-toi d'inclure portaudio.h si AudioDriver l'utilise directement

namespace adikdrum {

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool init(PaStreamCallback *callback, void *userData, int sampleRate, int framesPerBuffer);
    void close();

private:
    AudioDriver audioDriver_;
};

} // namespace adikdrum
#endif // AUDIOMANAGER_H

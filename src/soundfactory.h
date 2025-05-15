#ifndef SOUNDFACTORY_H
#define SOUNDFACTORY_H

#include <vector>
#include <memory>
#include "soundgenerator.h" // Ajoute cette ligne !
#include "audiosound.h"

namespace adikdrum {

class SoundFactory {
public:
    SoundFactory(int sampleRate, float defaultDuration);
    ~SoundFactory();

    SoundPtr applyEnvelopeToAudioSound(SoundPtr audioSound, float decayRate);
    SoundPtr applyNoiseEnvelopeToAudioSound(SoundPtr audioSound, float decayRate);
    SoundPtr applySquareEnvelopeToAudioSound(SoundPtr audioSound, float duration, float decayRate);
    SoundPtr createEnvelopeForAudioSound(float duration, float decayRate);
    std::vector<float> createEnvelope(int sr, float dur, float decayRate);
    std::vector<float> applyEnvelope(const std::vector<float>& wave, float decayRate);

    SoundPtr generateKick();
    SoundPtr generateSnare();
    SoundPtr generateHiHat(float durationFactor);
    SoundPtr generateKick2();
    SoundPtr generateSnare2();
    SoundPtr generateCymbal(float durationFactor);
    SoundPtr generateTestTone(float frequency = 440.0f, float duration = 0.5f);
    SoundPtr generateBuzzer(float frequency, float duration);
    SoundPtr tone(const std::string& type, float frequency, float length);
private:
    int sampleRate_;
    float defaultDuration_;
    SoundGenerator generator_;
};

//==== End of class SoundFactory ====

} // namespace adikdrum

#endif // SOUNDFACTORY_H


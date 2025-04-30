#ifndef SOUNDFACTORY_H
#define SOUNDFACTORY_H

#include <vector>
#include <memory>
#include "soundgenerator.h" // Ajoute cette ligne !
#include "audiosound.h"


class SoundFactory {
public:
    SoundFactory(int sampleRate, double defaultDuration);
    ~SoundFactory();

    std::shared_ptr<AudioSound> applyEnvelopeToAudioSound(std::shared_ptr<AudioSound> audioSound, double decayRate);
    std::shared_ptr<AudioSound> applyNoiseEnvelopeToAudioSound(std::shared_ptr<AudioSound> audioSound, double decayRate);
    std::shared_ptr<AudioSound> applySquareEnvelopeToAudioSound(std::shared_ptr<AudioSound> audioSound, double duration, double decayRate);
    std::shared_ptr<AudioSound> createEnvelopeForAudioSound(double duration, double decayRate);
    std::vector<double> createEnvelope(int sr, double dur, double decayRate);
    std::vector<double> applyEnvelope(const std::vector<double>& wave, double decayRate);

    std::shared_ptr<AudioSound> generateKick();
    std::shared_ptr<AudioSound> generateSnare();
    std::shared_ptr<AudioSound> generateHiHat(double durationFactor);
    std::shared_ptr<AudioSound> generateKick2();
    std::shared_ptr<AudioSound> generateSnare2();
    std::shared_ptr<AudioSound> generateCymbal(double durationFactor);
    std::shared_ptr<AudioSound> generateTestTone(double frequency, double duration);
    std::shared_ptr<AudioSound> generateBuzzer(double frequency, double duration);

private:
    int sampleRate_;
    double defaultDuration_;
    SoundGenerator generator_;

};

#endif // SOUNDFACTORY_H

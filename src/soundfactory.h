#ifndef SOUNDFACTORY_H
#define SOUNDFACTORY_H

#include <vector>
#include "soundgenerator.h" // Ajoute cette ligne !

class SoundFactory {
public:
    SoundFactory(int sr, double duration);

    std::vector<double> generateKick();
    std::vector<double> generateKick2();
    std::vector<double> generateSnare();
    std::vector<double> generateSnare2();
    std::vector<double> generateHiHat(double durationScale = 0.25);
    std::vector<double> generateCymbal(double durationScale = 3.0);
    std::vector<double> generateTestTone(double frequency);

private:
    SoundGenerator generator;
    int sampleRate;
    double defaultDuration;

    std::vector<double> applyEnvelope(std::vector<double> wave, double decayRate);
    std::vector<double> applyNoiseEnvelope(std::vector<double> wave, double decayRate);
    std::vector<double> applySquareEnvelope(std::vector<double> wave, int sr, double dur, double decayRate);
    std::vector<double> createEnvelope(int sr, double dur, double decayRate);
};

#endif // SOUNDFACTORY_H

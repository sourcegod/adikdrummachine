#ifndef AUDIOSOUND_H
#define AUDIOSOUND_H

#include <vector>
#include <cstddef> // for size_t

class AudioSound {
public:
    size_t startPos, curPos, endPos =0;
    AudioSound(std::vector<double> data, int numChannels);
    ~AudioSound();

    bool isActive() const { return active_; }
    void setActive(bool active);
    std::vector<double>& getRawData() { return rawData_; }
    const double* getData() const { return rawData_.data(); }
    size_t getSize() const { return rawData_.size(); }
    size_t getLength() const { return length_; }
    double getNextSample(); // Pour obtenir l'échantillon suivant
    void resetCurPos() { curPos =0; }   // Pour recommencer la lecture du son
    size_t getCurPos() const { return curPos; }   // Pour recommencer la lecture du son
    int getNumChannels() const { return numChannels_; }
    // std::vector<float> readData(size_t numFrames);
    size_t readData(size_t numFrames);
    bool isFramesRemaining(unsigned long framesRemaining) const { return (endPos - curPos) <= framesRemaining; }
    std::vector<float>& getSoundBuffer() { return soundBuffer_; }
    void applyStaticFadeOutLinear(float fadeOutStartPercent);
    void applyStaticFadeOutExp(float fadeOutStartPercent, float powerFactor); 


private:
    bool active_;
    std::vector<double> rawData_;
    std::vector<float> soundBuffer_;
    int numChannels_;


    int length_;
};

#endif // AUDIOSOUND_H

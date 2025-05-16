#ifndef SIMPLEDELAY_H
#define SIMPLEDELAY_H

#include <vector>
#include <cstddef>  // Pour size_t

class SimpleDelay {
public:
    SimpleDelay(size_t bufferSize, float sampleRate);
    ~SimpleDelay();

    void setDelayTime(float delayTimeMs);
    void setFeedback(float feedback);
    void setGain(float gain);
    void setIsEnabled(bool isEnabled);

    // Fonction process modifiée pour traiter un buffer:
    void process(std::vector<float>& buffer, size_t numFrames, int numChannels);

private:
    std::vector<float> delayBuffer_;
    size_t bufferSize_;
    size_t writeIndex_;
    float delayTimeMs_;
    float feedback_;
    float gain_;
    float sampleRate_;
    bool isEnabled_;
};

#endif // SIMPLEDELAY_H


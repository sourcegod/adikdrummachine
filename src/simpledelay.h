#ifndef SIMPLEDELAY_H
#define SIMPLEDELAY_H

#include <vector>
#include <cstddef>  // Pour size_t

class SimpleDelay {
public:
    SimpleDelay(size_t bufferSize, float sampleRate);
    ~SimpleDelay();

    void setDelayTime(float delayTimeSec);
    void setFeedback(float feedback);
    void setGain(float gain);
    void setActive(bool active);

    // Fonction process modifiée pour traiter un buffer:
    void processData(std::vector<float>& bufData, size_t numFrames, int numChannels);

private:
    std::vector<float> delayBuffer_;
    size_t bufferSize_;
    float sampleRate_;
    size_t writeIndex_;
    float delayTimeSec_;
    float feedback_;
    float gain_;
    bool active_;
};
//==== End of class SimpleDelay ====

#endif // SIMPLEDELAY_H


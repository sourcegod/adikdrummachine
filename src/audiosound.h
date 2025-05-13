#ifndef AUDIOSOUND_H
#define AUDIOSOUND_H

#include <vector>
#include <cstddef> // for size_t
#include <memory>  // Pour std::shared_ptr

namespace adikdrum {

class AudioSound {
public:
    size_t startPos = 0;
    size_t curPos = 0;
    size_t endPos = 0;

    // Constructeur principal
    AudioSound(std::vector<float> data, size_t numChannels = 1, size_t sampleRate = 44100, size_t bitDepth = 16);

    // Constructeur de copie *modifié* pour partager rawData_
    AudioSound(const AudioSound& other)
        : rawData_(other.rawData_), // Partage le vecteur, ne le copie pas
          numChannels_(other.numChannels_),
          sampleRate_(other.sampleRate_),
          bitDepth_(other.bitDepth_),
          length_(other.length_),
          speed_(other.speed_),
          active_(false) {
    startPos = 0;
    curPos = 0;
    endPos = length_;

}

    virtual ~AudioSound();

    virtual bool isActive() const { return active_; }
    virtual void setActive(bool active);
    std::vector<float>& getRawData() { return rawData_; }
    const float* getData() const { return rawData_.data(); }
    size_t getSize() const { return rawData_.size(); }
    size_t getLength() const { return length_; }
    virtual float getNextSample();
    void resetCurPos() { curPos = 0; }
    size_t getCurPos() const { return curPos; }
    void setCurPos(size_t pos) { if (pos < endPos) curPos = pos; }
    size_t getNumChannels() const { return numChannels_; }
    size_t getSampleRate() const { return sampleRate_; }
    size_t getBitDepth() const { return bitDepth_; }
    // virtual size_t readData(std::vector<float>& bufData, size_t numFrames);
    virtual size_t readData(std::vector<float>& buffer, size_t numFrames);
    virtual bool isFramesRemaining(size_t framesRemaining) const { return (endPos - curPos) >= framesRemaining * numChannels_; }
    virtual void applyStaticFadeOutLinear(float fadeOutStartPercent);
    virtual void applyStaticFadeOutExp(float fadeOutStartPercent, float powerFactor);

    void setSpeed(float speed) {
        speed_ = speed;
    }

    float  getSpeed() const { return speed_; }
    bool isFinished() const { return !active_ || curPos >= endPos / numChannels_; }

protected:
    std::vector<float> rawData_;
    size_t numChannels_;
    size_t sampleRate_;
    size_t bitDepth_;
    size_t length_ = 0;
    float speed_ =1.0f;

private:
    bool active_ = false;
};
//==== End of class AudioSound ====


using SoundPtr = std::shared_ptr<AudioSound>;
} // namespace adikdrum

#endif // AUDIOSOUND_H

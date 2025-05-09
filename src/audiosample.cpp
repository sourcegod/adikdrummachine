#include "audiosample.h"
#include <iostream>
#include <vector> // N'oublie pas d'inclure vector ici

namespace adikdrum {

AudioSample::AudioSample(const std::string& fileName)
    : AudioSound(std::vector<float>()), // Appel explicite au constructeur de AudioSound
      audioFile_(),
      currentFileName_(fileName) {
    if (!fileName.empty()) {
        load(fileName);
    }
}

bool AudioSample::load(const std::string& fileName) {
    if (audioFile_.load(fileName)) {
        std::optional<SoundPtr> sound = audioFile_.getSound();
        if (sound.has_value() && sound.value()) {
            rawData_ = sound.value()->getRawData();
            numChannels_ = sound.value()->getNumChannels();
            sampleRate_ = sound.value()->getSampleRate();
            bitDepth_ = sound.value()->getBitDepth();
            length_ = rawData_.size();
            endPos = length_;
            currentFileName_ = fileName;
            return true;
        } else {
            std::cerr << "Error: Could not get AudioSound data from file: " << fileName << std::endl;
            rawData_.clear();
            length_ = 0;
            endPos = 0;
            return false;
        }
    } else {
        std::cerr << "Error loading audio file: " << fileName << std::endl;
        rawData_.clear();
        length_ = 0;
        endPos = 0;
        return false;
    }
}

} // namespace adikdrum

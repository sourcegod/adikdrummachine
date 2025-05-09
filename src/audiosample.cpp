#include "audiosample.h"
#include <iostream>

namespace adikdrum {

AudioSample::AudioSample(const std::string& fileName) : audioFile_(), currentFileName_(fileName) {
    if (!fileName.empty()) {
        load(fileName);
    }
}

bool AudioSample::load(const std::string& fileName) {
    if (audioFile_.load(fileName)) {
        std::optional<SoundPtr> sound = audioFile_.getSound();
        if (sound.has_value() && sound.value()) {
            // Copier les données de l'AudioSound temporaire vers notre AudioSample
            rawData_ = sound.value()->getRawData();
            numChannels_ = sound.value()->getNumChannels().value_or(1);
            sampleRate_ = sound.value()->getSampleRate().value_or(44100);
            bitDepth_ = sound.value()->getBitDepth().value_or(16);
            length_ = rawData_.size();
            endPos = length_;
            currentFileName_ = fileName;
            return true;
        } else {
            std::cerr << "Error: Could not get AudioSound data from file: " << fileName << std::endl;
            // Réinitialiser les données en cas d'échec
            rawData_.clear();
            length_ = 0;
            endPos = 0;
            return false;
        }
    } else {
        std::cerr << "Error loading audio file: " << fileName << std::endl;
        // Réinitialiser les données en cas d'échec
        rawData_.clear();
        length_ = 0;
        endPos = 0;
        return false;
    }
}

} // namespace adikdrum

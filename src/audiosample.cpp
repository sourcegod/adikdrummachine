#include "audiosample.h"
#include <iostream>
#include <vector> // N'oublie pas d'inclure vector ici

namespace adikdrum {

AudioSample::AudioSample(const std::string& filePath)
    : AudioSound(std::vector<float>()), // Appel explicite au constructeur de AudioSound
      audioFile_(),
      filePath_(filePath) {
    if (!filePath.empty()) {
        load(filePath);
    }
}

bool AudioSample::load(const std::string& filePath) {
    if (audioFile_.load(filePath)) {
        std::optional<SoundPtr> sound = audioFile_.getSound();
        if (sound.has_value() && sound.value()) {


            // Au lieu d'assigner directement, on *copie* l'AudioSound
            AudioSound tempSound = *(sound.value());  // Appel au constructeur de copie de AudioSound

            //on assigne les valeurs de tempSound à l'objet courant
            rawData_ = tempSound.getRawData();
            numChannels_ = tempSound.getNumChannels();
            sampleRate_ = tempSound.getSampleRate();
            bitDepth_ = tempSound.getBitDepth();
            length_ = tempSound.getLength();
            endPos = length_;
            filePath_ = filePath;
            return true;
            /*
            rawData_ = sound.value()->getRawData();
            numChannels_ = sound.value()->getNumChannels();
            sampleRate_ = sound.value()->getSampleRate();
            bitDepth_ = sound.value()->getBitDepth();
            length_ = rawData_.size();
            endPos = length_;
            filePath_ = filePath;
            std::cout  << "In AudioSample::load, filePath: " << filePath_ 
                << ",\nnumChannels: " << numChannels_ << ", length: " << length_ << "\n";
            */

            return true;
        } else {
            std::cerr << "Error: Could not get AudioSound data from file: " << filePath << std::endl;
            rawData_.clear();
            length_ = 0;
            endPos = 0;
            return false;
        }
    } else {
        std::cerr << "Error loading audio file: " << filePath << std::endl;
        rawData_.clear();
        length_ = 0;
        endPos = 0;
        return false;
    }
    
}

} // namespace adikdrum

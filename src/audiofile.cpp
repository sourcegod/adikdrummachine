#include "audiofile.h"
#include "audiosound.h"
#include <iostream>
#include <vector>
#include <stdexcept>
#include <sndfile.h>

namespace adikdrum {

AudioFile::AudioFile() = default;

AudioFile::~AudioFile() {
    close();
}

bool AudioFile::load(const std::string& filePath) {
    close(); // Close any previously opened file

    filePath_ = filePath;
    sfInfo_.format = 0; // Initialize format

    sndFile_ = sf_open(filePath_.c_str(), SFM_READ, &sfInfo_);
    if (sndFile_ == nullptr) {
        std::cerr << "Error opening audio file: " << filePath_ << " - " << sf_strerror(nullptr) << std::endl;
        return false;
    }

    // Read all samples into the buffer
    sf_count_t numFrames = sfInfo_.frames;
    samples_.resize(numFrames * sfInfo_.channels);

    sf_count_t framesRead = sf_read_float(sndFile_, samples_.data(), numFrames * sfInfo_.channels);
    if (framesRead < numFrames * sfInfo_.channels) {
        std::cerr << "Warning: Could not read all frames from " << filePath_ << std::endl;
        samples_.resize(framesRead); // Resize to the actual number of samples read
    }

    std::cout << "In AudioFile::Load, filePath: " << filePath_ 
        << ",\nnumChannels: " << sfInfo_.channels << ", numFrames: " << numFrames << ", framesRead: " << framesRead << "\n";

    return true;
}

void AudioFile::close() {
    if (sndFile_ != nullptr) {
        if (sf_close(sndFile_) != 0) {
            std::cerr << "Error closing audio file: " << filePath_ << " - " << sf_strerror(sndFile_) << std::endl;
        }
        sndFile_ = nullptr;
    }
    samples_.clear();
    filePath_.clear();
    numChannels_ = 0;
    sampleRate_ = 0;
    bitDepth_ = 0;
    sfInfo_.format = 0;
}

std::optional<uint32_t> AudioFile::getNumChannels() const {
    if (sndFile_ != nullptr) {
        return sfInfo_.channels;
    }
    return std::nullopt;
}

std::optional<uint32_t> AudioFile::getSampleRate() const {
    if (sndFile_ != nullptr) {
        return sfInfo_.samplerate;
    }
    return std::nullopt;
}

std::optional<uint32_t> AudioFile::getBitDepth() const {
    if (sndFile_ != nullptr) {
        if (sndFile_) {
            if (SF_FORMAT_PCM_S8 == (SF_FORMAT_PCM_S8 & sfInfo_.format)) {
                return 8;
            } else if (SF_FORMAT_PCM_U8 == (SF_FORMAT_PCM_U8 & sfInfo_.format)) {
                return 8;
            } else if (SF_FORMAT_PCM_16 == (SF_FORMAT_PCM_16 & sfInfo_.format)) {
                return 16;
            } else if (SF_FORMAT_PCM_24 == (SF_FORMAT_PCM_24 & sfInfo_.format)) {
                return 24;
            } else if (SF_FORMAT_PCM_32 == (SF_FORMAT_PCM_32 & sfInfo_.format)) {
                return 32;
            } else if (SF_FORMAT_FLOAT == (SF_FORMAT_FLOAT & sfInfo_.format)) {
                return 32;
            } else if (SF_FORMAT_DOUBLE == (SF_FORMAT_DOUBLE & sfInfo_.format)) {
                return 64;
            }
            return 0; // Unknown or unsupported PCM format
        }
    }
    return std::nullopt;
}


std::optional<std::vector<float>> AudioFile::getSamples() const {
    if (sndFile_ != nullptr) {
        return samples_;
    }
    return std::nullopt;
}

std::optional<SoundPtr> AudioFile::getSound() const {
    if (!samples_.empty()) {
        // Créer un shared_ptr vers un nouvel objet AudioSound
        auto sound = std::make_shared<AudioSound>(samples_, getNumChannels().value_or(1));
        return sound;
    }
    return std::nullopt;
}


} // namespace adikdrum

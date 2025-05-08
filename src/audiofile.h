#ifndef AUDIO_FILE_H
#define AUDIO_FILE_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <optional>
#include <sndfile.h> // Include libsndfile header
#include "audiosound.h"

namespace adikdrum {

struct AudioFile {
public:
    AudioFile();
    ~AudioFile();

    bool load(const std::string& filePath);
    void close();

    std::optional<uint32_t> getNumChannels() const;
    std::optional<uint32_t> getSampleRate() const;
    std::optional<uint32_t> getBitDepth() const; // libsndfile retourne la profondeur en bits
    std::optional<std::vector<float>> getSamples() const;
    std::optional<std::shared_ptr<AudioSound>> getSound() const;

private:
    SNDFILE* sndFile_ = nullptr;
    SF_INFO sfInfo_;
    std::string filePath_;
    uint16_t numChannels_ = 0;
    uint32_t sampleRate_ = 0;
    uint16_t bitDepth_ = 0;
    std::vector<float> samples_;
};

} // namespace adikdrum

#endif // AUDIO_FILE_H

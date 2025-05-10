#ifndef AUDIOSAMPLE_H
#define AUDIOSAMPLE_H

#include "audiosound.h"
#include "audiofile.h"
#include <string>
#include <memory>

namespace adikdrum {

class AudioSample : public AudioSound {
public:
    AudioSample(const std::string& filePath = "");
    bool load(const std::string& filePath);
    // Pas besoin de redéfinir le destructeur si la classe dérivée n'alloue pas de ressources spécifiques

private:
    AudioFile audioFile_;
    std::string filePath_;
};

} // namespace adikdrum

#endif // AUDIOSAMPLE_H

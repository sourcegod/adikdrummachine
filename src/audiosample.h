#ifndef AUDIOSAMPLE_H
#define AUDIOSAMPLE_H

#include "audiosound.h"
#include "audiofile.h"
#include <string>
#include <memory>

namespace adikdrum {

class AudioSample : public AudioSound {
public:
    AudioSample(const std::string& fileName = "");
    bool load(const std::string& fileName);
    // Pas besoin de redéfinir le destructeur si la classe dérivée n'alloue pas de ressources spécifiques

private:
    AudioFile audioFile_;
    std::string currentFileName_;
};

} // namespace adikdrum

#endif // AUDIOSAMPLE_H

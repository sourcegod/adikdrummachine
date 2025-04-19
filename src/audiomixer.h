#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H

#include <vector>
#include <array>
#include <memory> // Pour std::shared_ptr
#include "audiosound.h"

class AudioMixer {
public:
    AudioMixer(int numChannels);
    ~AudioMixer();

    void play(int channel, std::shared_ptr<AudioSound> sound); // Prend un shared_ptr
    void pause(int channel);
    void stop(int channel);
    void setVolume(int channel, float volume);
    float getVolume(int channel) const;
    bool isChannelActive(int channel) const;
    void setChannelActive(int channel, bool active);
    std::shared_ptr<AudioSound> getSound(int channel) const; // Retourne un shared_ptr
    void setGlobalVolume(float volume); // Nouvelle fonction
    float getGlobalVolume() const;     // Nouvelle fonction

private:
    struct ChannelInfo {
        bool active;
        float volume;
        std::shared_ptr<AudioSound> sound; // shared_ptr vers l'objet AudioSound
    };

    std::array<ChannelInfo, 17> channels_;
    float globalVolume_; // Variable pour le volume global
    static const int metronomeChannel_ = 0;
};

#endif // AUDIOMIXER_H

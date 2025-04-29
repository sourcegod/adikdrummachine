#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H

#include <vector>
#include <cstddef>  // Pour size_t
#include <memory> // Pour std::shared_ptr
#include "audiosound.h"

struct ChannelInfo {
    std::shared_ptr<AudioSound> sound; // shared_ptr vers l'objet AudioSound
    bool active_;
    float volume;
    float pan; // Ajouter cette ligne

    bool reserved;
    size_t startPos; // Position de début de la lecture (sera 0)
    size_t curPos;   // Position de lecture actuelle
    size_t endPos;   // Position de fin de la lecture (taille du buffer)

    bool isPlaying() const { return active_ && sound && curPos < endPos; }
    bool isActive() const { return active_; }
    void setActive(bool active) { active_ = active; }
    bool muted;
    ChannelInfo() 
      : sound(nullptr), 
      active_(false), volume(1.0f), pan(0.0f),
      reserved(false), 
      startPos(0), curPos(0), endPos(0), 
      muted(false) {}

};

class AudioMixer {
public:
    AudioMixer(size_t numChannels);
    ~AudioMixer();

    bool init(int sampleRate = 44100, int channels = 2, int bits = 16);
    void close();
    void play(size_t channel, std::shared_ptr<AudioSound> sound); // Prend un shared_ptr
    void pause(size_t channel);
    void stop(size_t channel);
    float getVolume(size_t channel) const;
    void setVolume(size_t channel, float volume);
    float getGlobalVolume() const;     // Nouvelle fonction
    void setGlobalVolume(float volume); // Nouvelle fonction

    bool isChannelActive(size_t channel) const;
    void setChannelActive(size_t channel, bool active);
    std::shared_ptr<AudioSound> getSound(size_t channel) const; // Retourne un shared_ptr
    void reserveChannel(size_t channel, bool reserved); // Nouvelle fonction pour réserver un canal
    bool isChannelReserved(size_t channel) const; // Nouvelle fonction pour vérifier si un canal est réservé
    bool isChannelPlaying(size_t channel) const;

    size_t getChannelCurPos(size_t channel) const;
    void setChannelCurPos(size_t channel, size_t pos);
    size_t getChannelEndPos(size_t channel) const;
    std::vector<ChannelInfo>& getChannelList();
    void setChannelMuted(size_t channelIndex, bool muted);
    bool isChannelMuted(size_t channelIndex) const;
    void resetMute();
    void setChannelPan(size_t channelIndex, float panValue);
    float getChannelPan(size_t channelIndex) const;

    void fadeInLinear(size_t channelIndex, std::vector<float>& bufData, unsigned long durationFrames, int outputNumChannels);
    void fadeOutLinear(size_t channelIndex, std::vector<float>& bufData, unsigned long durationFrames, int outputNumChannels);
    ChannelInfo getChannelInfo(size_t channelIndex) { return channelList_[channelIndex]; }
    void mixSoundData(std::vector<float>& outputBuffer, size_t framesPerBuffer, size_t outputNumChannels);

private:
    std::vector<ChannelInfo> channelList_;
    float globalVolume_; // Variable pour le volume global
    size_t numChannels_;

    static const int metronomeChannel_ = 0;
};

#endif // AUDIOMIXER_H

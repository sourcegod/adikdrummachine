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

    bool init(int sampleRate = 44100, int channels = 2, int bits = 16);
    void close();
    void play(int channel, std::shared_ptr<AudioSound> sound); // Prend un shared_ptr
    void pause(int channel);
    void stop(int channel);
    float getVolume(int channel) const;
    void setVolume(int channel, float volume);
    float getGlobalVolume() const;     // Nouvelle fonction
    void setGlobalVolume(float volume); // Nouvelle fonction

    bool isChannelActive(int channel) const;
    void setChannelActive(int channel, bool active);
    std::shared_ptr<AudioSound> getSound(int channel) const; // Retourne un shared_ptr
    void reserveChannel(int channel, bool reserved); // Nouvelle fonction pour réserver un canal
    bool isChannelReserved(int channel) const; // Nouvelle fonction pour vérifier si un canal est réservé
    bool isChannelPlaying(int channel) const;

    // Nouvelle interface pour accéder aux membres de ChannelInfo de manière sécurisée
    size_t getChannelCurPos(int channel) const;
    void setChannelCurPos(int channel, size_t pos);
    size_t getChannelEndPos(int channel) const;

private:
    unsigned int numChannels_;
    struct ChannelInfo {
        bool active;
        float volume;
        std::shared_ptr<AudioSound> sound; // shared_ptr vers l'objet AudioSound
        bool reserved;
        size_t startPos; // Position de début de la lecture (sera 0)
        size_t curPos;   // Position de lecture actuelle
        size_t endPos;   // Position de fin de la lecture (taille du buffer)

        bool isPlaying() const { return active && sound && curPos < endPos; }

    };

    std::array<ChannelInfo, 18> channels_;
    float globalVolume_; // Variable pour le volume global
    static const int metronomeChannel_ = 0;
};

#endif // AUDIOMIXER_H

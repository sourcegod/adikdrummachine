#ifndef AUDIOMIXER_H
#define AUDIOMIXER_H

#include <vector>
#include <array> // Pour une taille fixe de canaux

class AudioMixer {
public:
    AudioMixer(int numChannels);
    ~AudioMixer();

    void play(int channel, int soundIndex);
    void pause(int channel);
    void stop(int channel);
    void setVolume(int channel, float volume);
    float getVolume(int channel) const;
    bool isChannelActive(int channel) const;

private:
    struct ChannelInfo {
        bool active;
        float volume;
        int soundIndex; // Index du son à jouer
        // Peut-être un pointeur vers l'itérateur de lecture du son ?
    };

    std::array<ChannelInfo, 17> channels_; // 17 canaux audio
    static const int metronomeChannel_ = 0; // Canal réservé au métronome (par exemple)
};

#endif // AUDIOMIXER_H

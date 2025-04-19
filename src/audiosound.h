#ifndef AUDIOSOUND_H
#define AUDIOSOUND_H

#include <vector>

class AudioSound {
public:
    AudioSound(std::vector<double> data);
    ~AudioSound();

    bool isActive() const;
    void setActive(bool active);
    std::vector<double>& getRawData();
    int getLength() const;
    double getNextSample(); // Pour obtenir l'échantillon suivant
    void resetPlayhead();   // Pour recommencer la lecture du son

private:
    bool active_;
    std::vector<double> rawData_;
    int length_;
    int playhead_; // Indice de lecture courant
};

#endif // AUDIOSOUND_H

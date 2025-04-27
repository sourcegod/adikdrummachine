#ifndef AUDIOSOUND_H
#define AUDIOSOUND_H

#include <vector>
#include <cstddef> // for size_t

class AudioSound {
public:
    // AudioSound(std::vector<double> data);
    AudioSound(std::vector<double> data, int numChannels);
    ~AudioSound();

    bool isActive() const { return active_; }
    void setActive(bool active);
    std::vector<double>& getRawData() { return rawData_; }
    const double* getData() const { return rawData_.data(); }
    size_t getSize() const { return rawData_.size(); }
    // int getLength() const { return length_; }
    double getNextSample(); // Pour obtenir l'échantillon suivant
    void resetPlayhead();   // Pour recommencer la lecture du son
    int getNumChannels() const { return numChannels_; }


private:
    bool active_;
    std::vector<double> rawData_;
    int numChannels_;


    int length_;
    int playhead_; // Indice de lecture courant
};

#endif // AUDIOSOUND_H

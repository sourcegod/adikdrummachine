#ifndef DRUMPLAYER_H
#define DRUMPLAYER_H

#include <vector>

class DrumPlayer {
public:

    std::vector<bool> playing;
    bool isPlaying;
    bool isClicking; // Nouvelle variable pour activer/désactiver le métronome
    int currentStep;
    int bpm;
    double secondsPerStep;
    std::vector<std::vector<double>> drumSounds_; // Stocke la liste des sons
    std::vector<double>::iterator* currentSound_; // Pointeur vers un tableau d'itérateurs
    int clickStep; // Nouvelle variable pour le pas du métronome


    DrumPlayer(int numSounds, int initialBpm, 
        const std::vector<std::vector<double>>& sounds);
    ~DrumPlayer();
    void playSound(int soundIndex);
    void stopAllSounds();
    void playMetronome(); // Nouvelle fonction pour jouer le métronome

    double softClip(double x);
    void setBpm(int newBpm);


private:
    int sampleRate_; // Ajoute une variable pour stocker le sample rate (nécessaire pour setBpm)
    bool isFirstBeat_; // Pour savoir si c'est le premier temps de la mesure
    int beatCounter_; // Nouvelle variable pour compter les temps


};

#endif // DRUMPLAYER_H

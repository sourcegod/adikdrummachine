#ifndef ADIKDRUM_H
#define ADIKDRUM_H

#include "audiodriver.h"
#include "drumplayer.h"
#include "audiomixer.h"
#include "audiosound.h"
#include <vector>
#include <string>

class AdikDrum {
public:
    struct DrumMachineData {
        DrumPlayer* player;
        AudioMixer* mixer;
        double sampleRate;
    };

    std::string helpText;
    std::pair<int, int> cursorPos;



    AdikDrum();
    ~AdikDrum();

    bool initApp();
    void closeApp();
    void run(); // Fonction pour la boucle principale (gestion du clavier)
    void loadSounds();
    const std::vector<std::shared_ptr<AudioSound>>& getDrumSounds() const;
    void demo();
    void loadPattern();

    void displayMessage(const std::string& message); 
    void displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<int, int> cursor);
    void selectStep();
    void unselectStep();
    void moveCursorUp();
    void moveCursorDown();
    void moveCursorRight();
    void moveCursorLeft();

private:
    AudioDriver audioDriver_;
    int sampleRate_;
    AudioMixer mixer_;
    std::vector<std::shared_ptr<AudioSound>> drumSounds_; // Membre public pour stocker les sons
    std::shared_ptr<AudioSound> soundClick1_;
    std::shared_ptr<AudioSound> soundClick2_;
    int numSounds_;
    int numSteps_;
    int initialBpm_;
    DrumPlayer drumPlayer_; // Note: il faut Déclarer drumPlayer_ APRÈS numSounds_ et numSteps_, pour l'ordre d'initialisation des membres

    DrumMachineData drumData_;

};


#endif // ADIKDRUM_H

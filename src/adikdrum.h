#ifndef ADIKDRUM_H
#define ADIKDRUM_H

#include "audiodriver.h"
#include "drumplayer.h"
#include "audiomixer.h"
#include "audiosound.h"
#include "uiapp.h" // Inclure l'interface UIApp
#include <vector>
#include <string>
#include <sstream>
#include <map>

class AdikDrum {
public:
    struct DrumMachineData {
        DrumPlayer* player;
        AudioMixer* mixer;
        double sampleRate;
    };

    int numSounds_;
    int numSteps_;
    std::string helpText;
    std::pair<int, int> cursorPos;
    std::map<char, int> keyToSoundMap;
    UIApp* uiApp_; // Pointeur vers l'objet UIApp


    AdikDrum(UIApp* uiApp); // Constructeur prend un pointeur UIApp
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
    void playPause();
    void toggleClick();
    void stopAllSounds();
    void toggleMute();
    void resetMute();
    void changeVolume(float deltaVolume);
    void changeBpm(float deltaBpm);
    void changePan(float deltaPan);
    void playKey(char key);        
    void triggerLastSound();
    void playCurrentSound();

private:
    AudioDriver audioDriver_;
    int sampleRate_;
    AudioMixer mixer_;
    std::vector<std::shared_ptr<AudioSound>> drumSounds_; // Membre public pour stocker les sons
    std::shared_ptr<AudioSound> soundClick1_;
    std::shared_ptr<AudioSound> soundClick2_;
    int initialBpm_;
    DrumPlayer drumPlayer_; // Note: il faut Déclarer drumPlayer_ APRÈS numSounds_ et numSteps_, pour l'ordre d'initialisation des membres

    DrumMachineData drumData_;
    std::string msgText_;

};
//==== End of class AdikDrum ====


#endif // ADIKDRUM_H

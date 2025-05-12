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

namespace adikdrum {

class AdikDrum {
public:
    struct DrumMachineData {
        DrumPlayer* player;
        AudioMixer* mixer;
        double sampleRate;
    };

    UIApp* uiApp_; // Pointeur vers l'objet UIApp
    std::string helpText;
    std::pair<size_t, size_t> cursorPos;


    AdikDrum(UIApp* uiApp); // Constructeur prend un pointeur UIApp
    ~AdikDrum();

    bool initApp();
    void closeApp();
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
    auto& getPattern() { return drumPlayer_.pattern_; }
    size_t getNumSounds() const { return numSounds_; }
    size_t getNumSteps() const { return numSteps_; }
    void changeSpeed(float speed);

private:
    AudioDriver audioDriver_;
    int sampleRate_;
    AudioMixer mixer_;
    size_t numSounds_;
    size_t numSteps_;
    std::vector<std::shared_ptr<AudioSound>> drumSounds_; // Membre public pour stocker les sons
    std::shared_ptr<AudioSound> soundClick1_;
    std::shared_ptr<AudioSound> soundClick2_;
    int initialBpm_;
    DrumPlayer drumPlayer_; // Note: il faut Déclarer drumPlayer_ APRÈS numSounds_ et numSteps_, pour l'ordre d'initialisation des membres
    std::string msgText_;

    DrumMachineData drumData_;

};
//==== End of class AdikDrum ====


} // namespace adikdrum

#endif // ADIKDRUM_H

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
    std::string helpText_;
    bool helpDisplayed_ = false; // Pour savoir si l'aide est affichée

    std::pair<size_t, size_t> cursorPos;


    AdikDrum(UIApp* uiApp); // Constructeur prend un pointeur UIApp
    ~AdikDrum();

    bool initApp();
    void closeApp();
    void loadSounds();
    const std::vector<SoundPtr>& getDrumSounds() const;
    void demo(int numSound=16);
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
    void playKey(int soundIndex);        
    void playKeyPad(int soundIndex);
    void triggerLastSound();
    void playCurrentSound();
    auto& getPattern() { return drumPlayer_.pattern_; }
    auto& getCurPattern() { return drumPlayer_.curPattern_; }
    auto& getDrumPlayer() { return drumPlayer_; }
    size_t getNumSounds() const { return numSounds_; }
    size_t getNumSteps() const { return numSteps_; }
    void changeSpeed(float speed);
    void genTones();
    void toggleDelay();
    void changeShiftPad(size_t deltaShiftPad);
    void changeBar(int delta);
    void gotoStart();
    void gotoEnd();
    void setUIApp(UIApp* uiApp);
    
    // Partie Enregistrement
    void toggleRecord();
    void recordSound(size_t soundIndex); 
    void deleteLastPlayedStep(); // Renommée pour être plus générique
    void clearCurrentSound();
    void clearLastPlayedSound();
    void clearPattern();

    void toggleHelp(); // Pour basculer l'affichage de l'aide
    bool isHelpDisplayed() const { return helpDisplayed_; }
    void showStatus();
    const std::string& getMsgText() const { return msgText_; }
    void setPlayQuantizeResolution(size_t reso);
    void setRecQuantizeResolution(size_t reso);
    void quantizePlayedSteps();
    void genStepsFromSound();
    void quantizeStepsFromSound();
    void saveData();
    void loadData();


    void test();


private:
    AudioDriver audioDriver_;
    int sampleRate_;
    AudioMixer mixer_;
    size_t numSounds_;
    size_t numSteps_;
    std::vector<SoundPtr> drumSounds_; // Membre public pour stocker les sons
    SoundPtr soundClick1_;
    SoundPtr soundClick2_;
    int initialBpm_;
    DrumPlayer drumPlayer_; // Note: il faut Déclarer drumPlayer_ APRÈS numSounds_ et numSteps_, pour l'ordre d'initialisation des membres
    DrumMachineData drumData_;
    std::string msgText_;
    std::string previousMsgText_; 


    size_t shiftPadIndex_ =0;

};

//==== End of class AdikDrum ====


} // namespace adikdrum

#endif // ADIKDRUM_H

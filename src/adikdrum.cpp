/*
 *  File: adikdrum.cpp
 *  Drum Machine Engine in C++
 *  Compile: g++ adikdrum.cpp -o adikdrum -lportaudio -lpthread
 *  Date: Sat, 12/04/2025
 *  Author: Coolbrother
 *  */
//----------------------------------------

#include "adikdrum.h"
#include "consoleuiapp.h" // Inclure l'en-tête de ConsoleUIApp
#include "audiodriver.h" // Inclure le header de AudioDriver
#include "soundfactory.h" // Inclure le header de SoundFactory
#include "drumplayer.h"
#include "audiomixer.h"
#include "constants.h"

#include <iostream>
#include <string>
#include <sstream> // for osstringstream
#include <vector>
#include <cmath>
#include <random>
#include <portaudio.h>
#include <algorithm>
#include <map>
#include <utility> // Pour utiliser std::pair
// for performance checking
#include <thread>
#include <chrono>

//----------------------------------------


volatile int callbackCounter =0;
void beep() {
    std::cout << '\a' << std::flush;
}

//----------------------------------------

static int drumMachineCallback(const void* inputBuffer, void* outputBuffer,
                             unsigned long framesPerBuffer,
                             const PaStreamCallbackTimeInfo* timeInfo,
                             PaStreamCallbackFlags statusFlags,
                             void* userData) {
    // Note: Casting parameters in void here, to avoid compiler warnings: inused parameter.
     (void)inputBuffer;
     (void)timeInfo;
     (void)statusFlags;
    AdikDrum::DrumMachineData* data = static_cast<AdikDrum::DrumMachineData*>(userData);
    if (data && data->mixer) {
        float* out = static_cast<float*>(outputBuffer);
        static unsigned long frameCounter = 0;
        unsigned long samplesPerStep = static_cast<unsigned long>(data->sampleRate * data->player->secondsPerStep);
        const int outputNumChannels = 2; // Assumons stéréo pour l'instant
        std::vector<float> bufData(framesPerBuffer * outputNumChannels, 0.0f);

        if (frameCounter >= samplesPerStep) {
            frameCounter = 0;
            if (data->player->isPlaying()) {
                data->player->clickStep = data->player->currentStep;
                if (data->player->isClicking() && data->player->clickStep % 4 == 0) {
                  data->player->playMetronome();
                }
                data->player->playPattern();
                data->player->currentStep = (data->player->currentStep + 1) % data->player->getNumSteps();
            } else if (data->player->isClicking()) {
                if (data->player->clickStep % 4 == 0) {
                    data->player->playMetronome();
                }
                data->player->clickStep = (data->player->clickStep + 1) % data->player->getNumSteps();
            }
        }

        // Mixer les sons en utilisant la fonction dédiée
        data->mixer->mixSoundData(bufData, framesPerBuffer, outputNumChannels);

        // Copie du buffer de mixage vers le buffer de sortie PortAudio
        for (unsigned long i = 0; i < framesPerBuffer * outputNumChannels; ++i) {
          // Note: il est recommandé de convertir la sortie en static_cast float, pour éviter des comportements inattendus de convertion de types implicites.  
          out[i] = static_cast<float>(data->player->hardClip(bufData[i] * data->mixer->getGlobalVolume() * GLOBAL_GAIN));
        
        }

        if (data->player->isPlaying() || data->player->isClicking()) {
            frameCounter += framesPerBuffer;
        }
        return paContinue;
    }
    return paContinue;
}

//----------------------------------------

AdikDrum::AdikDrum(UIApp* uiApp)
    : uiApp_(uiApp),
      cursorPos({0, 0}),
      sampleRate_(44100),
      mixer_(32),
      numSounds_(16),
      numSteps_(16),
      drumPlayer_(numSounds_, numSteps_),
      msgText_("") // Initialisation optionnelle

{
      std::cout << "AdikDrum::Constructor - numSounds_: " << numSounds_ << ", numSteps_: " << numSteps_ << std::endl;
      helpText = "Appuyez sur les touches suivantes pour contrôler l'application:\n"
               "  Q: Quitter l'application\n"
               "  Entrée: Activer le pas à la position du curseur\n"
               "  Backspace: Désactiver le pas à la position du curseur\n"
               "  Espace: Activer/désactiver la lecture\n"
               "  c: Démarrer/arrêter le métronome\n"
               "  p: Lancer la démo des sons\n"
               "  Ctrl+p: Charger un nouveau pattern aléatoire\n"
               "  v: Arrêter tous les sons en cours\n"
               "  x: Muter/démuter le son courant (ligne du curseur)\n"
               "  X: Réinitialiser le mute de tous les canaux\n"
               "  +: Augmenter le volume global\n"
               "  -: Diminuer le volume global\n"
               "  (: Diminuer le BPM\n"
               "  ): Augmenter le BPM\n"
               "  [: Déplacer le panoramique du son courant vers la gauche\n"
               "  ]: Déplacer le panoramique du son courant vers la droite\n"
               "  Flèche Haut: Déplacer le curseur vers le haut et jouer le son\n"
               "  Flèche Bas: Déplacer le curseur vers le bas et jouer le son\n"
               "  Flèche Droite: Déplacer le curseur vers la droite\n"
               "  Flèche Gauche: Déplacer le curseur vers la gauche\n"
               "  Touches [q-k, a-i]: Jouer le son correspondant\n";

}
//----------------------------------------


AdikDrum::~AdikDrum() {
    // closeApp();
}
//----------------------------------------

bool AdikDrum::initApp() {
    const int numChannelsMixer = 32; // Clarifier le nom pour le mixer
    const int sampleRate = 44100;
    const int framesPerBuffer = 256; // Nouvelle variable pour la taille du buffer
    const double defaultDuration = 0.1;
    SoundFactory soundFactory(sampleRate, defaultDuration);

    // Générer les sons du métronome
    std::shared_ptr<AudioSound> soundClick1 = soundFactory.generateBuzzer(880.0, 50); // Son aigu
    std::shared_ptr<AudioSound> soundClick2 = soundFactory.generateBuzzer(440.0, 50); // Son grave

    float fadeOutStartPercentage = 0.1f; // Appliquer le fondu à partir d'un pourcentage de la longueur
    soundClick1->applyStaticFadeOutLinear(fadeOutStartPercentage);
    soundClick2->applyStaticFadeOutLinear(fadeOutStartPercentage);
 
    // global structure for now
    mixer_ = AudioMixer(numChannelsMixer);
    drumData_.player = &drumPlayer_;
    drumData_.mixer = &mixer_;
    drumData_.sampleRate = sampleRate;
    drumPlayer_.setMixer(mixer_); // Assigner le mixer à player
    loadSounds(); // charger les sons
    drumPlayer_.drumSounds_ = this->getDrumSounds();

    // Assigner les sons du métronome à DrumPlayer
    drumPlayer_.soundClick1_ = soundClick1;
    drumPlayer_.soundClick2_ = soundClick2;
    // drumPlayer_.pattern_ = pattern; // Assign the global pattern to the player

    const int numOutputChannels = 2; // Définir explicitement le nombre de canaux de sortie
    if (!audioDriver_.init(numOutputChannels, sampleRate, framesPerBuffer, drumMachineCallback, &drumData_)) {
        std::cerr << "Erreur lors de l'initialisation de l'AudioDriver." << std::endl;
        return 1;
    }

    if (!audioDriver_.start()) {
        std::cerr << "Erreur lors du démarrage de l'AudioDriver." << std::endl;
        audioDriver_.stop();
        return 1;
    }

    // Tester les sons
    demo();

    std::cout << "AdikDrum initialisé et démarré." << std::endl;
    // std::cin.get();

    return true;
}
//----------------------------------------

void AdikDrum::closeApp() {
    audioDriver_.stop();
    // audioDriver_.close(); // not nessary cause it managing by the AudioDriver's destructor
    std::cout << "AdikDrum fermé." << std::endl;

}
//----------------------------------------

void AdikDrum::loadSounds() {
    const int sampleRate = 44100;
    const double defaultDuration = 0.1;
    SoundFactory soundFactory(sampleRate, defaultDuration);
    drumSounds_.clear(); // S'assurer que le vecteur est vide avant de charger

     drumSounds_.push_back(soundFactory.generateKick());
     drumSounds_.push_back(soundFactory.generateSnare());
    drumSounds_.push_back(soundFactory.generateHiHat(0.25));
    drumSounds_.push_back(soundFactory.generateKick2());
    drumSounds_.push_back(soundFactory.generateSnare2());
    drumSounds_.push_back(soundFactory.generateCymbal(3.0));
    drumSounds_.push_back(soundFactory.generateTestTone(440.0, defaultDuration));
    drumSounds_.push_back(soundFactory.generateTestTone(550.0, defaultDuration));
    drumSounds_.push_back(soundFactory.generateTestTone(220.0, defaultDuration)); // Exemple pour RimShot
    drumSounds_.push_back(soundFactory.generateTestTone(330.0, defaultDuration)); // Exemple pour HandClap
    drumSounds_.push_back(soundFactory.generateHiHat(0.5)); // Exemple pour HiHatOpen
    drumSounds_.push_back(soundFactory.generateTestTone(110.0, defaultDuration)); // Exemple pour LowTom
    drumSounds_.push_back(soundFactory.generateTestTone(165.0, defaultDuration)); // Exemple pour MidTom
    drumSounds_.push_back(soundFactory.generateTestTone(275.0, defaultDuration)); // Exemple pour HiTom
    drumSounds_.push_back(soundFactory.generateTestTone(660.0, 0.1)); // Exemple pour CowBell
    drumSounds_.push_back(soundFactory.generateTestTone(440.0, 0.3)); // Exemple pour Tambourine
    
    float fadeOutStartPercentage = 0.3f; // Appliquer le fondu à partir d'un pourcentage de la longueur
    // float expFadeOutStartPercentage = 0.8f; // Commencer le fondu à 60% pour les clics
    // float exponentialPower = 3.0f; // Facteur de puissance pour le fondu exponentiel

    for (auto&  sound : drumSounds_) {
      // std::cout << "voici len: " << sound->getLength() << " et pourcentage de début de fadeout: " << fadeOutStartPercentage << std::endl;
      // Appliquer un fondu linéaire par défaut
      sound->applyStaticFadeOutLinear(fadeOutStartPercentage);
      
      // std::cout << "Appliquer un fondu exponentiel au clic." << std::endl;
      // sound->applyStaticFadeOutExp(expFadeOutStartPercentage, exponentialPower);

    }
      

}
//----------------------------------------

const std::vector<std::shared_ptr<AudioSound>>& AdikDrum::getDrumSounds() const {
    return drumSounds_;
}
//----------------------------------------

void AdikDrum::demo() {
    // Tester les sons
    msgText_ = "Demo en train de jouer";
    displayMessage(msgText_);
    for (size_t i = 0; i < drumSounds_.size(); ++i) {
        drumPlayer_.playSound(i);
        long long sleepDurationMs = static_cast<long long>(drumPlayer_.drumSounds_[i]->getSize() * 1000.0 / sampleRate_ * 1.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
    }
    msgText_ = "Demo terminée.";
    displayMessage(msgText_);

}
//----------------------------------------

void AdikDrum::loadPattern() {
    msgText_ = "Chargement d'un pattern de démonstration...";
    displayMessage(msgText_);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 1);

    for (size_t i = 0; i < drumPlayer_.pattern_.size(); ++i) {
        for (int j = 0; j < drumPlayer_.numSteps_; ++j) {
            drumPlayer_.pattern_[i][j] = (distrib(gen) == 1);
        }
    }
    msgText_ = "Pattern de démonstration chargé.";
    displayMessage(msgText_);
    drumPlayer_.resetMute();
    displayGrid(drumPlayer_.pattern_, cursorPos);
}
//----------------------------------------

void AdikDrum::displayMessage(const std::string& message) {
    if (uiApp_) {
        uiApp_->displayMessage(message);
    } else {
        std::cerr << "Erreur: UIApp n'est pas initialisé." << std::endl;
    }
}
//----------------------------------------

void AdikDrum::displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<int, int> cursor) {
    if (uiApp_) {
    auto numSounds = getNumSounds();
    auto numSteps = getNumSteps();
        uiApp_->displayGrid(grid, cursor, numSounds, numSteps);
    } else {
        std::cerr << "Erreur: UIApp n'est pas initialisé." << std::endl;
    }
}
//----------------------------------------



/*
void AdikDrum::displayMessage(const std::string& message) {
    std::cout << message << std::endl;
}
//----------------------------------------

void AdikDrum::displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<int, int> cursor) {
    std::ostringstream oss;
    oss << "  ";
    for (int i = 0; i < numSteps_; ++i) {
        oss << (i + 1) % 10 << " ";
    }
    oss << std::endl;
    for (int i = 0; i < numSounds_; ++i) {
        oss << (i + 1) % 10 << " ";
        for (int j = 0; j < numSteps_; ++j) {
            if (cursor.first == j && cursor.second == i) {
                oss << "x ";
            } else if (grid[i][j]) {
                oss << "# ";
            } else {
                oss << "- ";
            }
        }
        oss << std::endl;
    }
    oss << std::endl;
    displayMessage(oss.str());
}
//----------------------------------------
*/

void AdikDrum::selectStep() {
    drumPlayer_.pattern_[cursorPos.second][cursorPos.first] = true;
    msgText_ = "Step " + std::to_string(cursorPos.first + 1) + " on sound " + std::to_string(cursorPos.second + 1) + " activated and playing.";
    displayMessage(msgText_);
    drumPlayer_.playSound(cursorPos.second);
    displayGrid(drumPlayer_.pattern_, cursorPos);
}
//----------------------------------------

void AdikDrum::unselectStep() {
    drumPlayer_.pattern_[cursorPos.second][cursorPos.first] = false;
    msgText_ = "Step " + std::to_string(cursorPos.first + 1) + " on sound " + std::to_string(cursorPos.second + 1) + " deactivated.";
    displayMessage(msgText_);
    displayGrid(drumPlayer_.pattern_, cursorPos);
}
//----------------------------------------

void AdikDrum::moveCursorUp() {
    if (cursorPos.second > 0) {
        cursorPos.second--;
        displayGrid(drumPlayer_.pattern_, cursorPos);
        msgText_ = "Cursor up, playing sound " + std::to_string(cursorPos.second + 1);
        displayMessage(msgText_);
        drumPlayer_.playSound(cursorPos.second);
    } else {
        beep();
    }
}
//----------------------------------------

void AdikDrum::moveCursorDown() {
    if (cursorPos.second < numSounds_ - 1) {
        cursorPos.second++;
        displayGrid(drumPlayer_.pattern_, cursorPos);
        msgText_ = "Cursor down, playing sound " + std::to_string(cursorPos.second + 1);
        displayMessage(msgText_);
        drumPlayer_.playSound(cursorPos.second);
    } else {
        beep();
    }
}
//----------------------------------------

void AdikDrum::moveCursorRight() {
    if (cursorPos.first < numSteps_ - 1) {
        cursorPos.first++;
        displayGrid(drumPlayer_.pattern_, cursorPos);
        msgText_ = "Cursor right, step " + std::to_string(cursorPos.first + 1);
        displayMessage(msgText_);
    } else {
        beep();
        displayMessage("Reached the end (right).");
    }
}
//----------------------------------------

void AdikDrum::moveCursorLeft() {
    if (cursorPos.first > 0) {
        cursorPos.first--;
        displayGrid(drumPlayer_.pattern_, cursorPos);
        msgText_ = "Cursor left, step " + std::to_string(cursorPos.first + 1);
        displayMessage(msgText_);
    } else {
        beep();
        displayMessage("Reached the beginning (left).");
    }
}
//----------------------------------------

void AdikDrum::playPause() {
    drumPlayer_.togglePlay();
    msgText_ = std::string("Play: ") + (drumPlayer_.isPlaying() ? "ON" : "OFF");
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::toggleClick() {
    drumPlayer_.toggleClick();
    msgText_ = std::string("Metronome: ") + (drumPlayer_.isClicking() ? "ON" : "OFF");
    displayMessage(msgText_);
    if (drumPlayer_.isClicking()) {
        drumPlayer_.startClick();
    } else {
        drumPlayer_.stopClick();
    }
}
//----------------------------------------

void AdikDrum::stopAllSounds() {
    drumPlayer_.stopAllSounds();
    msgText_ = "All sounds stopped.";
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::toggleMute() {
    int currentSoundIndex = cursorPos.second;
    bool currentMuted = drumPlayer_.isSoundMuted(currentSoundIndex);
    drumPlayer_.setSoundMuted(currentSoundIndex, !currentMuted);
    msgText_ = "Son " + std::to_string(currentSoundIndex) +
               " (canal " + std::to_string(currentSoundIndex + 1) +
               ") est maintenant " + (currentMuted ? "démuté" : "muté") + ".";
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::resetMute() {
    drumPlayer_.resetMute();
    msgText_ = "Tous les sons ont été démutés.";
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::changeVolume(float deltaVolume) {
    float currentVolume = mixer_.getGlobalVolume();
    mixer_.setGlobalVolume(std::clamp(currentVolume + deltaVolume, 0.0f, 1.0f));
    msgText_ = "Volume global: " + std::to_string(static_cast<int>(mixer_.getGlobalVolume() * 10)) + "/10";
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::changeBpm(float deltaBpm) {
    auto bpm = drumPlayer_.getBpm();
    drumPlayer_.setBpm(bpm + deltaBpm);
    msgText_ = "BPM réglé à " + std::to_string(drumPlayer_.getBpm());
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::changePan(float deltaPan) {
    int currentChannelIndex = cursorPos.second + 1;
    float currentPan = mixer_.getChannelPan(currentChannelIndex);
    mixer_.setChannelPan(currentChannelIndex, std::clamp(currentPan + deltaPan, -1.0f, 1.0f));
    msgText_ = "Pan du canal " + std::to_string(currentChannelIndex) +
               " réglé à " + std::to_string(mixer_.getChannelPan(currentChannelIndex));
    displayMessage(msgText_);
}
//----------------------------------------
void AdikDrum::playKey(char key) {
    auto it = KEY_TO_SOUND_MAP.find(key);
    if (it != KEY_TO_SOUND_MAP.end()) {
        int soundIndex = it->second;
        drumPlayer_.playSound(soundIndex);
    }
}

void AdikDrum::triggerLastSound() {
    drumPlayer_.playLastSound();
    // msgText_ = "Rejouer le dernier son sur le canal 17.";
    // displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::playCurrentSound() {
    size_t currentSoundIndex = static_cast<size_t>(cursorPos.second);
    drumPlayer_.playSound(currentSoundIndex);
    // msgText_ = "Jouer le son " + std::to_string(currentSoundIndex + 1) + " (canal " + std::to_string(currentSoundIndex + 1) + ").";
    // displayMessage(msgText_);
}
//----------------------------------------
//==== End of class AdikDrum ====


int main() {
    AdikDrum adikDrumApp(nullptr); // Créer AdikDrum sans UIApp pour l'instant
    ConsoleUIApp consoleUI(adikDrumApp); // Créer ConsoleUIApp en passant une référence à AdikDrum
    adikDrumApp.uiApp_ = &consoleUI; // Assigner l'UIApp à AdikDrum
    if (!adikDrumApp.initApp()) {
        return false;
    }

    if (consoleUI.init()) {
        consoleUI.run();
        consoleUI.close();
    }
    adikDrumApp.closeApp();

    return 0;
}

/*
int main() {
    AdikDrum adikDrumApp;
    if (adikDrumApp.initApp()) {
        adikDrumApp.run();
        adikDrumApp.closeApp();
    }
    return 0;
}
*/
//----------------------------------------


/*
 *  File: adikdrum.cpp
 *  Drum Machine Engine in C++
 *  Compile: g++ adikdrum.cpp -o adikdrum -lportaudio -lpthread
 *  Date: Sat, 12/04/2025
 *  Author: Coolbrother
 *  */
//----------------------------------------

#include "adikdrum.h"

#include "adikcuiapp.h" // Inclure l'en-tête de ConsoleUIApp
#include "audiodriver.h" // Inclure le header de AudioDriver
#include "drumplayer.h"
#include "audiomixer.h"
#include "constants.h"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <utility> // Pour utiliser std::pair
#include <random>

// for performance checking
#include <thread>
#include <chrono>

//----------------------------------------


volatile int callbackCounter =0;

void beep() {
    std::cout << '\a' << std::flush;
}
//----------------------------------------

namespace adikdrum {
/*
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
*/

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
    mixer_ = AudioMixer(numChannelsMixer);

    // Générer les sons du métronome
    SoundPtr soundClick1 = mixer_.genTone("buzzer", 880.0, 50); // Son aigu
    SoundPtr soundClick2 = mixer_.genTone("buzzer", 440.0, 50); // Son aigu

    float fadeOutStartPercentage = 0.1f; // Appliquer le fondu à partir d'un pourcentage de la longueur
    soundClick1->applyStaticFadeOutLinear(fadeOutStartPercentage);
    soundClick2->applyStaticFadeOutLinear(fadeOutStartPercentage);
 
    // global structure for now
    drumData_.player = &drumPlayer_;
    drumData_.mixer = &mixer_;
    drumData_.sampleRate = sampleRate;
    drumPlayer_.setMixer(mixer_); // Assigner le mixer à player
    loadSounds(); // charger les sons
    // genTones();
    drumPlayer_.drumSounds_ = this->getDrumSounds();

    // Assigner les sons du métronome à DrumPlayer
    drumPlayer_.soundClick1_ = soundClick1;
    drumPlayer_.soundClick2_ = soundClick2;

    const int numOutputChannels = 2; // Définir explicitement le nombre de canaux de sortie
    if (!audioDriver_.init(numOutputChannels, sampleRate, framesPerBuffer, &drumData_)) {
    // if (!audioDriver_.init(numOutputChannels, sampleRate, framesPerBuffer, drumMachineCallback, &drumData_)) {
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
    auto soundCount = SOUND_LIST.size();
    drumSounds_.clear();
    drumSounds_.resize(soundCount); // Redimensionner drumSounds_ en fonction du nombre de fichiers à charger

    for (size_t i = 0; i < soundCount; ++i) {
        std::string filePath = MEDIA_DIR + "/" + SOUND_LIST[i]; // Construire le chemin complet du fichier
        SoundPtr sound = mixer_.loadSound(filePath); // Charger le fichier

        if (sound->getLength() > 0) {
            drumSounds_[i] = sound;
            std::cout << "Loaded " << SOUND_LIST[i] << " at index " << i << std::endl;
        } else {
            std::cerr << "Error loading " << filePath << ". Loading default sound instead." << std::endl;
        }
    }

    /*
    float fadeOutStartPercentage = 0.3f;
    for (auto& sound : drumSounds_) {
        if (sound) {
            sound->applyStaticFadeOutLinear(fadeOutStartPercentage);
        }
    }
    */


}
//----------------------------------------


void AdikDrum::genTones() {
    const float defaultFrequency = 440.0;
    const float defaultDuration = 0.1;
    drumSounds_.clear(); // S'assurer que le vecteur est vide avant de charger

    drumSounds_.push_back(mixer_.genTone("kick"));
    drumSounds_.push_back(mixer_.genTone("snare"));
    drumSounds_.push_back(mixer_.genTone("hihat", defaultFrequency, 0.25));
    drumSounds_.push_back(mixer_.genTone("kick2"));
    drumSounds_.push_back(mixer_.genTone("snare2"));
    drumSounds_.push_back(mixer_.genTone("cymbal", defaultFrequency, 3.0));
    drumSounds_.push_back(mixer_.genTone("sine", 440.0));
    drumSounds_.push_back(mixer_.genTone("sine", 550.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 220.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 330.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("hihat", defaultFrequency, 0.5));
    drumSounds_.push_back(mixer_.genTone("sine", 110.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 165.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 165.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 175.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 440.0, 0.3));
    
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


const std::vector<SoundPtr>& AdikDrum::getDrumSounds() const {
    return drumSounds_;
}
//----------------------------------------

void AdikDrum::demo() {
    // Tester les sons
    msgText_ = "Demo en train de jouer";
    displayMessage(msgText_);
    for (size_t i = 0; i < NUM_SOUNDS; ++i) {
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
    
    // Récupère le pattern courant
    auto curPattern = drumPlayer_.curPattern_;

    // Vérifie si le pattern est valide avant de générer des données
    if (curPattern) {
        // Génère les données du pattern directement dans l'objet AdikPattern
        curPattern->genData();
    } else {
        std::cerr << "Erreur: curPattern_ n'est pas initialisé dans DrumPlayer." << std::endl;
        msgText_ = "Erreur: Impossible de charger le pattern de démonstration.";
        displayMessage(msgText_);
        return;
    }

    msgText_ = "Pattern de démonstration chargé.";
    displayMessage(msgText_);
    drumPlayer_.resetMute();

    // Affiche la première barre du pattern (index 0) par défaut.
    // Vous pourriez vouloir afficher la barre courante si vous en avez une.
    displayGrid(curPattern->getPatternBar(0), cursorPos);
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
        int soundIndex = it->second + shiftPadIndex_;
        drumPlayer_.playSound(soundIndex);
    }
}
//----------------------------------------

void AdikDrum::playKeyPad(char key) {
    auto it = KEYPAD_TO_SOUND_MAP.find(key);
    if (it != KEYPAD_TO_SOUND_MAP.end()) {
        int soundIndex = it->second + shiftPadIndex_;
        drumPlayer_.playSound(soundIndex);
    }
}
//----------------------------------------


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


void AdikDrum::changeSpeed(float speed) {
    int currentChannelIndex =  drumPlayer_.getLastSoundIndex() + 1;
    mixer_.setSpeed(currentChannelIndex, std::clamp(mixer_.getChannelList()[currentChannelIndex].speed + speed, 0.25f, 4.0f));
    std::string msgText = "Vitesse du canal " + std::to_string(currentChannelIndex) + " réglée à " + std::to_string(mixer_.getChannelList()[currentChannelIndex].speed);
    displayMessage(msgText);
}
//----------------------------------------

void AdikDrum::toggleDelay() {
    int currentChannelIndex =  drumPlayer_.getLastSoundIndex() + 1;
    bool active = mixer_.isDelayActive(currentChannelIndex);
    mixer_.setDelayActive(currentChannelIndex, !active);
    msgText_ = "Délai du Canal (" + std::to_string(currentChannelIndex + 1) +
               ") est maintenant " + (active ? "désactivé" : "activé") + ".";

    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::changeShiftPad(size_t deltaShiftPad) {
    // Note: converti le résultat en int pour que le compilateur ne converti pas un nombre négatif en un grand nombre unsigned, du fait que le type est size_t
    int tempVal = shiftPadIndex_ + deltaShiftPad;
    shiftPadIndex_ = std::clamp(tempVal, 0, 32);
    msgText_ = "Shift Pad: " + std::to_string(shiftPadIndex_) + "/32";
    displayMessage(msgText_);
}
//----------------------------------------



//==== End of class AdikDrum ====

} // namespace adikdrum

/*
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
*/
//----------------------------------------


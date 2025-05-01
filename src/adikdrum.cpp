/*
 *  File: adikdrum.cpp
 *  Drum Machine Engine in C++
 *  Compile: g++ adikdrum.cpp -o adikdrum -lportaudio -lpthread
 *  Date: Sat, 12/04/2025
 *  Author: Coolbrother
 *  */
//----------------------------------------

#include "adikdrum.h"
#include <iostream>
#include <string>
#include <sstream> // for osstringstream
#include <vector>
#include <cmath>
#include <random>
#include <portaudio.h>
#include <algorithm>
#include <map>
#include <termios.h>
#include <unistd.h>
#include <utility> // Pour utiliser std::pair
// for performance checking
#include <thread>
#include <chrono>

#include "audiodriver.h" // Inclure le header de AudioDriver
#include "soundfactory.h" // Inclure le header de SoundFactory
#include "drumplayer.h"
#include "audiomixer.h"

//----------------------------------------

const double PI = 3.14159265358979323846;
const int NUM_SOUNDS = 16; // Notre constante globale pour le nombre de sons
const float GLOBAL_GAIN = 0.2f;
int NUM_STEPS = 16;

volatile int callbackCounter =0;
termios oldTerm; // pour gérer le terminal
                 //
// Mapping des touches et des sons
std::map<char, int> keyToSoundMap = {
    {'q', 0}, {'s', 1}, {'d', 2}, {'f', 3},
    {'g', 4}, {'h', 5}, {'j', 6}, {'k', 7},
    {'a', 8}, {'z', 9}, {'e', 10}, {'r', 11},
    {'t', 12}, {'y', 13}, {'u', 14}, {'i', 15}
};

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

// Fonction pour initialiser le terminal
termios initTermios(int echo) {
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~(ICANON);
    if (echo == 0) {
        newt.c_lflag &= ~(ECHO);
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    return oldt;
}
//----------------------------------------

// Fonction pour restaurer les paramètres du terminal
void resetTermios(termios oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}
//----------------------------------------

AdikDrum::AdikDrum()
    : cursorPos({0, 0}),
      sampleRate_(44100),
      mixer_(18),
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


void AdikDrum::run() {
    oldTerm = initTermios(0);
    msgText_ = "Le clavier est initialisé.";
    displayMessage(msgText_);

    // displayGrid(pattern, cursorPos); // Affiche la grille au démarrage
    displayGrid(drumPlayer_.pattern_, cursorPos);

    char key;
    while (read(STDIN_FILENO, &key, 1) == 1) {
        if (key == 'Q') break;

        if (key == '\n') { // Touche Enter
            selectStep();
        } else if (key == 127) { // Touche Backspace
            unselectStep();
        } else if (key == ' ') { // Touche Espace
            playPause(); // Appelle la nouvelle fonction
        } else if (key == 'c') { // Touche 'c'
            toggleClick(); // Appelle la nouvelle fonction

        } else if (key == 'p') {
            demo();
        } else if (key == 16) { // Ctrl+p
            loadPattern();
        } else if (key == 'v') {
            stopAllSounds();
        } else if (key == 'x') {
            toggleMute();
        } else if (key == 'X') {
            resetMute();
        } else if (key == '+') { // Touche '+'
            changeVolume(0.1f); // Augmenter le volume
        } else if (key == '-') { // Touche '-'
            changeVolume(-0.1f); // Diminuer le volume

        } else if (key == '(') { // Touche '('
            changeBpm(-5.0f);    // Diminuer le BPM
        } else if (key == ')') { // Touche ')'
            changeBpm(5.0f);     // Augmenter le BPM

        } else if (key == '[') { // Touche '['
            changePan(-0.1f);
        } else if (key == ']') { // Touche ']'
            changePan(0.1f);
        } else if (keyToSoundMap.count(key)) {
            playKey(key); // Gérer les autres touches pour jouer des sons

        } else if (key == '\033') { // Code d'échappement
            read(STDIN_FILENO, &key, 1); // Lit '['
            read(STDIN_FILENO, &key, 1); // Lit le code de la flèche
            if (key == 'A') { // Flèche haut
                moveCursorUp();
            } else if (key == 'B') { // Flèche bas
                moveCursorDown();
            } else if (key == 'C') { // Flèche droite
                moveCursorRight();
            } else if (key == 'D') { // Flèche gauche
                moveCursorLeft();
            } // End arrow keys conditions

        } // End key == \033
    } // End the while Loop

    // Resets the terminal
    resetTermios(oldTerm);
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
    if (keyToSoundMap.count(key)) {
        int soundIndex = keyToSoundMap[key];
        drumPlayer_.playSound(soundIndex);
    }
}
//----------------------------------------
//==== End of class AdikDrum ====


int main() {
    AdikDrum adikDrumApp;
    if (adikDrumApp.initApp()) {
        adikDrumApp.run();
        adikDrumApp.closeApp();
    }
    return 0;
}
//----------------------------------------


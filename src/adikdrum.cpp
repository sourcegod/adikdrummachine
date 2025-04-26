/*
 *  File: adikdrum.cpp
 *  Drum Machine Engine in C++
 *  Compile: g++ adikdrum.cpp -o adikdrum -lportaudio -lpthread
 *  Date: Sat, 12/04/2025
 *  Author: Coolbrother
 *  */


#include "adikdrum.h"
#include <iostream>
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

const double PI = 3.14159265358979323846;
const int NUM_SOUNDS = 16; // Notre constante globale pour le nombre de sons
const float GLOBAL_GAIN = 0.8f;
int NUM_STEPS = 16;

std::pair<int, int> cursor_pos = {0, 0}; // {x, y}
std::vector<std::vector<bool>> pattern(NUM_SOUNDS, std::vector<bool>(NUM_STEPS, false));
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

struct DrumMachineData {
    DrumPlayer* player;
    AudioMixer* mixer;
    double sampleRate;
} drumData;


static int drumMachineCallback(const void* inputBuffer, void* outputBuffer,
                                 unsigned long framesPerBuffer,
                                 const PaStreamCallbackTimeInfo* timeInfo,
                                 PaStreamCallbackFlags statusFlags,
                                 void* userData) {
    DrumMachineData* data = static_cast<DrumMachineData*>(userData);
    // AdikDrum::DrumMachineData* data = static_cast<AdikDrum::DrumMachineData*>(userData);
    if (data && data->mixer) {
        float* out = static_cast<float*>(outputBuffer);
        static unsigned long frameCounter = 0;
        unsigned long samplesPerStep = static_cast<unsigned long>(data->sampleRate * data->player->secondsPerStep);

        // std::cout << "je suis dans le callback" << callbackCounter << " fois " << std::endl;
        if (frameCounter >= samplesPerStep) {
            frameCounter = 0;

            if (data->player->isPlaying) {
                data->player->clickStep = data->player->currentStep;
                if (data->player->isClicking && data->player->clickStep % 4 == 0) {
                  // beep();  
                  data->player->playMetronome();
                }
                data->player->playPattern();
                data->player->currentStep = (data->player->currentStep + 1) % data->player->getNumSteps();

            } else if (data->player->isClicking) {
                if (data->player->clickStep % 4 == 0) {
                    data->player->playMetronome();
                }
                data->player->clickStep = (data->player->clickStep + 1) % data->player->getNumSteps();
            }
        }

        // Mixer les sons
        
        for (unsigned long i = 0; i < framesPerBuffer; ++i) {
            double leftMix = 0.0;
            double rightMix = 0.0;
            for (auto& chan : data->mixer->getChannelList()) {
                if (chan.isActive() && !chan.muted && chan.sound) {
                    size_t curPos = chan.curPos;
                    size_t endPos = chan.endPos;
                    if (curPos < endPos) {
                        float panValue = chan.pan;
                        float rightGain = std::max(0.0f, 1.0f - panValue); // Pan -1 -> 0 (Left), 0 -> 1 (Right)
                        float leftGain = std::max(0.0f, 1.0f + panValue);  // Pan -1 -> 1 (Left), 0 -> 0 (Right)

                        float sample = chan.sound->getRawData()[curPos] * chan.volume;
                        leftMix += sample * leftGain;
                        rightMix += sample * rightGain;
                        chan.curPos++;
                    } else {
                        chan.setActive(false);
                    }
                }
            }
            *out++ = static_cast<float>(data->player->hardClip(leftMix * data->mixer->getGlobalVolume() * GLOBAL_GAIN));   // Left channel
            *out++ = static_cast<float>(data->player->hardClip(rightMix * data->mixer->getGlobalVolume() * GLOBAL_GAIN));  // Right channel
        }


        /*
        for (unsigned long i = 0; i < framesPerBuffer; ++i) {
            double mixedSample = 0.0;
            for (auto& chan: data->mixer->getChannelList()) {
                // if (chan.isActive()) {
                if (chan.isActive() && !chan.muted) { // Vérifier si le canal n'est pas muté
                    auto sound = chan.sound;
                    if (sound) {
                        size_t curPos = chan.curPos;
                        size_t endPos = chan.endPos;
                        if (curPos < endPos) {
                            mixedSample += (sound->getRawData()[curPos] * chan.volume);
                          chan.curPos++;
                        } else {
                            chan.setActive(false);
                        }

                    }
                }
            }
            *out++ = static_cast<float>(data->player->hardClip(mixedSample * data->mixer->getGlobalVolume() * GLOBAL_GAIN));
        }
        */

        if (data->player->isPlaying || data->player->isClicking) {
            frameCounter += framesPerBuffer;
        }
        return paContinue;
    }
        
    
    // beep();
    // callbackCounter++;
    return paContinue;
}

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

// Fonction pour restaurer les paramètres du terminal
void resetTermios(termios oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

void displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<int, int> cursor) {
    // std::cout << "\033[2J\033[H";
    std::cout << "  ";
    for (int i = 0; i < NUM_STEPS; ++i) {
        std::cout << (i + 1) % 10 << " ";
    }
    std::cout << std::endl;
    for (int i = 0; i < NUM_SOUNDS; ++i) {
        std::cout << (i + 1) % 10 << " ";
        for (int j = 0; j < NUM_STEPS; ++j) {
            if (cursor.first == j && cursor.second == i) {
                std::cout << "x ";
            } else if (grid[i][j]) {
                std::cout << "# "; // Affiche '#' si le pas est activé
            } else {
                std::cout << "- "; // Affiche '-' si le pas est désactivé
            }
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}



AdikDrum::AdikDrum()
    : sampleRate_(44100),
      numSounds_(16),
      numSteps_(16),
      mixer_(18),
      drumPlayer_(numSounds_, numSteps_) 
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
               "  Flèche Haut: Déplacer le curseur vers le haut et jouer le son\n"
               "  Flèche Bas: Déplacer le curseur vers le bas et jouer le son\n"
               "  Flèche Droite: Déplacer le curseur vers la droite\n"
               "  Flèche Gauche: Déplacer le curseur vers la gauche\n"
               "  Touches [q-k, a-i]: Jouer le son correspondant\n";

}


AdikDrum::~AdikDrum() {
    // closeApp();
}

bool AdikDrum::initApp() {
    const int numChannels = 32;
    const int sampleRate = 44100;
    const double defaultDuration = 0.1;
    SoundFactory soundFactory(sampleRate, defaultDuration);
   
    // Générer les sons du métronome
    std::shared_ptr<AudioSound> soundClick1 = soundFactory.generateBuzzer(880.0, 50); // Son aigu
    std::shared_ptr<AudioSound> soundClick2 = soundFactory.generateBuzzer(440.0, 50); // Son grave

    // global structure for now
    mixer_ = AudioMixer(numChannels);
    drumData.player = &drumPlayer_;
    drumData.mixer = &mixer_;
    drumData.sampleRate = sampleRate;
    drumPlayer_.setMixer(mixer_); // Assigner le mixer à player
    loadSounds(); // charger les sons
    drumPlayer_.drumSounds_ = this->getDrumSounds();

    // Assigner les sons du métronome à DrumPlayer
    drumPlayer_.soundClick1_ = soundClick1;
    drumPlayer_.soundClick2_ = soundClick2;
    drumPlayer_.pattern_ = pattern; // Assign the global pattern to the player
                                         


  if (!audioDriver_.init(drumMachineCallback, &drumData, sampleRate, 256)) {
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

void AdikDrum::closeApp() {
    audioDriver_.stop();
    // audioDriver_.close(); // not nessary cause it managing by the AudioDriver's destructor
    std::cout << "AdikDrum fermé." << std::endl;

}

void AdikDrum::run() {
    std::cout << helpText << std::endl; // Afficher le texte d'aide au début
    // initialiser le clavier
    oldTerm = initTermios(0);
    std::cout << "Le clavier est initialisé." << std::endl;


    // displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action
    char key;
    while (read(STDIN_FILENO, &key, 1) == 1) {

        if (key == 'Q') break;

        if (key == '\n') { // Touche Enter
            drumPlayer_.pattern_[cursor_pos.second][cursor_pos.first] = true; // Active toujours le pas
            std::cout << "Step " << cursor_pos.first + 1 << " on sound " << cursor_pos.second + 1 << " activated and playing." << std::endl;
            drumPlayer_.playSound(cursor_pos.second); // Utilise l'index directement

        } else if (key == 127) { // Touche Backspace (code ASCII 127)
            drumPlayer_.pattern_[cursor_pos.second][cursor_pos.first] = false;
            std::cout << "Step " << cursor_pos.first + 1 << " on sound " << cursor_pos.second + 1 << " deactivated." << std::endl;
        } else if (key == ' ') { // Touche Espace
            drumPlayer_.isPlaying = !drumPlayer_.isPlaying;
            std::cout << "Play: " << (drumPlayer_.isPlaying ? "ON" : "OFF") << std::endl;
        } else if (key == 'c') {
            drumPlayer_.isClicking = !drumPlayer_.isClicking;
            if (drumPlayer_.isClicking)
              drumPlayer_.startClick();
            else
              drumPlayer_.stopClick();
            std::cout << "Metronome: " << (drumPlayer_.isClicking ? "ON" : "OFF") << std::endl;

        } else if (key == 'p') {
            demo();
            std::cout << "Playing demo" << std::endl;
        } else if (key == 16) { // Ctrl+p (code ASCII 16) - Peut varier selon le terminal
            loadPattern();
            displayGrid(drumPlayer_.pattern_, cursor_pos);

        } else if (key == 'v') {
            drumPlayer_.stopAllSounds();
            std::cout << "All sounds stopped." << std::endl;
       
        } else if (key == 'x') {
            int currentSoundIndex = cursor_pos.second;
            bool currentMuted = drumPlayer_.isSoundMuted(currentSoundIndex);
            drumPlayer_.setSoundMuted(currentSoundIndex, !currentMuted);
        } else if (key == 'X') {
            drumPlayer_.resetMute(); // Réinitialiser le mute via DrumPlayer (qui appelle aussi AudioMixer)

        } else if (key == '+') {
            float currentVolume = drumData.mixer->getGlobalVolume();
            drumData.mixer->setGlobalVolume(std::min(1.0f, currentVolume + 0.1f));
            std::cout << "Volume global: " << static_cast<int>(drumData.mixer->getGlobalVolume() * 10) << "/10" << std::endl;
        } else if (key == '-') {
            float currentVolume = drumData.mixer->getGlobalVolume();
            drumData.mixer->setGlobalVolume(std::max(0.0f, currentVolume - 0.1f));
            std::cout << "Volume global: " << static_cast<int>(drumData.mixer->getGlobalVolume() * 10) << "/10" << std::endl;

        } else if (key == '(') {
            auto bpm = drumPlayer_.getBpm();  
            if (bpm > 5) {
              bpm -=5;  
                drumPlayer_.setBpm(bpm);
                std::cout << "BPM decreased to " << bpm << std::endl;
            } else {
                beep();
                std::cout << "Minimum BPM reached." << std::endl;
            }
        } else if (key == ')') {
            auto bpm = drumPlayer_.getBpm();  
            if (bpm < 800) {
              bpm +=5;  
              drumPlayer_.setBpm(bpm);
                std::cout << "BPM increased to " << bpm << std::endl;
            } else {
                beep();
                std::cout << "Maximum BPM reached." << std::endl;
            }

        } else if (key == '[') {
            int currentChannelIndex = cursor_pos.second;
            float currentPan = drumData.mixer->getChannelPan(currentChannelIndex);
            drumData.mixer->setChannelPan(currentChannelIndex, std::max(-1.0f, currentPan - 0.1f));
        } else if (key == ']') {
            int currentChannelIndex = cursor_pos.second;
            float currentPan = drumData.mixer->getChannelPan(currentChannelIndex);
            drumData.mixer->setChannelPan(currentChannelIndex, std::min(1.0f, currentPan + 0.1f));

        } else if (keyToSoundMap.count(key)) {
            int soundIndex = keyToSoundMap[key];
            drumPlayer_.playSound(soundIndex); // Utilise l'index directement

        } else if (key == '\033') { // Code d'échappement pour les séquences de touches spéciales (comme les flèches)
            read(STDIN_FILENO, &key, 1); // Lit le caractère '['
            read(STDIN_FILENO, &key, 1); // Lit le code de la flèche
            if (key == 'A') { // Flèche haut
                if (cursor_pos.second > 0) {
                    cursor_pos.second--;
                    displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action

                } else {
                    beep();
                }
                drumPlayer_.playSound(cursor_pos.second); // Utilise l'index directement
                std::cout << "Cursor up, playing sound " << cursor_pos.second << std::endl;
            } else if (key == 'B') { // Flèche bas
                if (cursor_pos.second < NUM_SOUNDS - 1) {
                    cursor_pos.second++;
                    displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action
                } else {
                    beep();
                }
                drumPlayer_.playSound(cursor_pos.second); // Utilise l'index directement
                std::cout << "Cursor down, playing sound " << cursor_pos.second << std::endl;

            } else if (key == 'C') { // Flèche droite
                if (cursor_pos.first < NUM_STEPS - 1) {
                    cursor_pos.first++;
                    displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action
                    std::cout << "Cursor right, step " << cursor_pos.first << std::endl;
                } else {
                    beep();
                    std::cout << "Reached the end (right)." << std::endl;
                }

            } else if (key == 'D') { // Flèche gauche
                if (cursor_pos.first > 0) {
                    cursor_pos.first--;
                    displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action
                    std::cout << "Cursor left, step " << cursor_pos.first << std::endl;
                } else {
                    beep();
                    std::cout << "Reached the beginning (left)." << std::endl;
                }

            }
        }
    
        // beep();
    } // End the while Loop

    // Resets the terminal
    resetTermios(oldTerm);

}

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
        

}

const std::vector<std::shared_ptr<AudioSound>>& AdikDrum::getDrumSounds() const {
    return drumSounds_;
}

void AdikDrum::demo() {
    // Tester les sons
    for (int i = 0; i < drumSounds_.size(); ++i) {
        drumData.player->playSound(i);
        long long sleepDurationMs = static_cast<long long>(drumData.player->drumSounds_[i]->getLength() * 1000.0 / sampleRate_ * 1.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
    }

}

void AdikDrum::loadPattern() {
    std::cout << "Chargement d'un pattern de démonstration..." << std::endl;
    // Exemple de pattern aléatoire
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 1); // 0 ou 1

    // std::cout << "pattern.size: " << drumPlayer_.pattern_.size() << std::endl;
    // std::cout << "numSteps: " << drumPlayer_.numSteps_ << std::endl;
    for (size_t i = 0; i < drumPlayer_.pattern_.size(); ++i) {
        for (int j = 0; j < drumPlayer_.numSteps_; ++j) {
            drumPlayer_.pattern_[i][j] = (distrib(gen) == 1); // Assigne aléatoirement true ou false
        }
    }
    std::cout << "Pattern de démonstration chargé." << std::endl;
    drumPlayer_.resetMute(); // Réinitialiser le mute lors du chargement d'un nouveau pattern

    // Afficher le pattern chargé en utilisant displayGrid
}

int main() {
    AdikDrum adikDrumApp;
    if (adikDrumApp.initApp()) {
        adikDrumApp.run();
        adikDrumApp.closeApp();
    }
    return 0;
}

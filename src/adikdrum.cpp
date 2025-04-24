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
            data->player->currentStep = (data->player->currentStep + 1) % NUM_STEPS;

        } else if (data->player->isClicking) {
            if (data->player->clickStep % 4 == 0) {
                data->player->playMetronome();
            }
            data->player->clickStep = (data->player->clickStep + 1) % NUM_STEPS;
        }
    }

    // Mixer les sons
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        double mixedSample = 0.0;
        for (auto& chan: data->mixer->getChannelList()) {
            if (chan.isActive()) {
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

    if (data->player->isPlaying || data->player->isClicking) {
        frameCounter += framesPerBuffer;
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
      initialBpm_(120),
      mixer_(18),
      drumPlayer_(numSounds_, numSteps_, initialBpm_) {
}

AdikDrum::~AdikDrum() {
    // closeApp();
}

bool AdikDrum::initApp() {
    const int numChannels = 18;
    //  AudioMixer mixer(numChannels);
    const int sampleRate = 44100;
    const double defaultDuration = 0.1;
   
    // /*
    SoundFactory soundFactory(sampleRate, defaultDuration);
    // std::vector<std::shared_ptr<AudioSound>> drumSounds;

    /* 
    drumSounds.push_back(soundFactory.generateKick());
    drumSounds.push_back(soundFactory.generateSnare());
    drumSounds.push_back(soundFactory.generateHiHat(0.25));
    drumSounds.push_back(soundFactory.generateKick2());
    drumSounds.push_back(soundFactory.generateSnare2());
    drumSounds.push_back(soundFactory.generateCymbal(3.0));
    drumSounds.push_back(soundFactory.generateTestTone(440.0, defaultDuration));
    drumSounds.push_back(soundFactory.generateTestTone(550.0, defaultDuration));
    drumSounds.push_back(soundFactory.generateTestTone(220.0, defaultDuration)); // Exemple pour RimShot
    drumSounds.push_back(soundFactory.generateTestTone(330.0, defaultDuration)); // Exemple pour HandClap
    drumSounds.push_back(soundFactory.generateHiHat(0.5)); // Exemple pour HiHatOpen
    drumSounds.push_back(soundFactory.generateTestTone(110.0, defaultDuration)); // Exemple pour LowTom
    drumSounds.push_back(soundFactory.generateTestTone(165.0, defaultDuration)); // Exemple pour MidTom
    drumSounds.push_back(soundFactory.generateTestTone(275.0, defaultDuration)); // Exemple pour HiTom
    drumSounds.push_back(soundFactory.generateTestTone(660.0, 1)); // Exemple pour CowBell
    drumSounds.push_back(soundFactory.generateTestTone(440.0, 3)); // Exemple pour Tambourine
    */    
    
    // Générer les sons du métronome
    std::shared_ptr<AudioSound> soundClick1 = soundFactory.generateBuzzer(880.0, 50); // Son aigu
    std::shared_ptr<AudioSound> soundClick2 = soundFactory.generateBuzzer(440.0, 50); // Son grave

    // global structure for now
    drumData.player = &drumPlayer_;
    drumData.mixer = &mixer_;
    drumData.player->setMixer(mixer_); // Assigner le mixer à player
    loadSounds(); // charger les sons
    drumData.player->drumSounds_ = this->getDrumSounds();
    drumData.sampleRate = sampleRate;

    // Assigner les sons du métronome à DrumPlayer
    drumData.player->soundClick1_ = soundClick1;
    drumData.player->soundClick2_ = soundClick2;
    drumData.player->pattern_ = pattern; // Assign the global pattern to the player
                                         
    // */


    // /*
    if (!audioDriver_.init(drumMachineCallback, &drumData, sampleRate, 256)) {
        std::cerr << "Erreur lors de l'initialisation de l'AudioDriver." << std::endl;
        return 1;
    }
    // */


    // /*
    if (!audioDriver_.start()) {
        std::cerr << "Erreur lors du démarrage de l'AudioDriver." << std::endl;
        audioDriver_.stop();
        return 1;
    }
    // */
    
    // Tester les sons
    demo();
    // initialiser le clavier
    oldTerm = initTermios(0);
    std::cout << "Le clavier est initialisé." << std::endl;


    std::cout << "AdikDrum initialisé et démarré." << std::endl;
    // std::cin.get();


    return true;
}

void AdikDrum::closeApp() {
    resetTermios(oldTerm);
    audioDriver_.stop();
    audioDriver_.close();
    std::cout << "AdikDrum fermé." << std::endl;

}

void AdikDrum::run() {

    try {

      // displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action
      char key;
      while (read(STDIN_FILENO, &key, 1) == 1) {
      // while (true) {
           // beep();              
           // key = std::cin.get();
           // std::cout << "voici key: " << key << std::endl;
           // continue;

          if (key == 'X') break;

          if (key == '\n') { // Touche Enter
              drumData.player->pattern_[cursor_pos.second][cursor_pos.first] = true; // Active toujours le pas
              std::cout << "Step " << cursor_pos.first + 1 << " on sound " << cursor_pos.second + 1 << " activated and playing." << std::endl;
              drumData.player->playSound(cursor_pos.second); // Utilise l'index directement

          } else if (key == 127) { // Touche Backspace (code ASCII 127)
              drumData.player->pattern_[cursor_pos.second][cursor_pos.first] = false;
              std::cout << "Step " << cursor_pos.first + 1 << " on sound " << cursor_pos.second + 1 << " deactivated." << std::endl;
          } else if (key == ' ') { // Touche Espace
              drumData.player->isPlaying = !drumData.player->isPlaying;
              std::cout << "Play: " << (drumData.player->isPlaying ? "ON" : "OFF") << std::endl;
          } else if (key == 'p') {
            demo();
            std::cout << "Playing demo" << std::endl;
          } else if (key == 'v') {
              drumData.player->stopAllSounds();
              std::cout << "All sounds stopped." << std::endl;
          } else if (key == 'c') {
              drumData.player->isClicking = !drumData.player->isClicking;
              if (drumData.player->isClicking)
                drumData.player->startClick();
              else
                drumData.player->stopClick();
              std::cout << "Metronome: " << (drumData.player->isClicking ? "ON" : "OFF") << std::endl;
          
          } else if (key == '+') {
              float currentVolume = drumData.mixer->getGlobalVolume();
              drumData.mixer->setGlobalVolume(std::min(1.0f, currentVolume + 0.1f));
              std::cout << "Volume global: " << static_cast<int>(drumData.mixer->getGlobalVolume() * 10) << "/10" << std::endl;
          } else if (key == '-') {
              float currentVolume = drumData.mixer->getGlobalVolume();
              drumData.mixer->setGlobalVolume(std::max(0.0f, currentVolume - 0.1f));
              std::cout << "Volume global: " << static_cast<int>(drumData.mixer->getGlobalVolume() * 10) << "/10" << std::endl;

          } else if (key == '(') {
              if (drumData.player->bpm > 5) {
                  drumData.player->setBpm(drumData.player->bpm - 5);
                  std::cout << "BPM decreased to " << drumData.player->bpm << std::endl;
              } else {
                  beep();
                  std::cout << "Minimum BPM reached." << std::endl;
              }
          } else if (key == ')') {
              if (drumData.player->bpm < 800) {
                  drumData.player->setBpm(drumData.player->bpm + 5);
                  std::cout << "BPM increased to " << drumData.player->bpm << std::endl;
              } else {
                  beep();
                  std::cout << "Maximum BPM reached." << std::endl;
              }

          } else if (keyToSoundMap.count(key)) {
              int soundIndex = keyToSoundMap[key];
              drumData.player->playSound(soundIndex); // Utilise l'index directement

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
                  drumData.player->playSound(cursor_pos.second); // Utilise l'index directement
                  std::cout << "Cursor up, playing sound " << cursor_pos.second << std::endl;
              } else if (key == 'B') { // Flèche bas
                  if (cursor_pos.second < NUM_SOUNDS - 1) {
                      cursor_pos.second++;
                      displayGrid(pattern, cursor_pos); // Affiche la grille après chaque action
                  } else {
                      beep();
                  }
                  drumData.player->playSound(cursor_pos.second); // Utilise l'index directement
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
      }

    } catch (const std::runtime_error& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
    }

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
    drumSounds_.push_back(soundFactory.generateTestTone(660.0, 1)); // Exemple pour CowBell
    drumSounds_.push_back(soundFactory.generateTestTone(440.0, 3)); // Exemple pour Tambourine
        

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

int main() {
    AdikDrum adikDrumApp;
    if (adikDrumApp.initApp()) {
        adikDrumApp.run();
        adikDrumApp.closeApp();
    }
    return 0;
}

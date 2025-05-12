#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <map>
#include <vector>
#include <string>

namespace adikdrum { // Ajout du namespace pour entourer les constantes

// Constantes mathématiques
const double PI = 3.14159265358979323846;

// Configuration de la map clavier -> son
const std::map<char, int> KEY_TO_SOUND_MAP = {
    {'q', 0}, {'s', 1}, {'d', 2}, {'f', 3},
    {'g', 4}, {'h', 5}, {'j', 6}, {'k', 7},
    {'a', 8}, {'z', 9}, {'e', 10}, {'r', 11},
    {'t', 12}, {'y', 13}, {'u', 14}, {'i', 15},
    {'1', 16}, {'2', 17}, {'3', 18}, {'4', 19}, {'5', 20},
};

// Configuration des fichiers audio
const std::string MEDIA_DIR = "./media";
const std::vector<std::string> SOUND_LIST = {
    "tr707/35_BassDrum2.wav",
    "tr707/36_BassDrum1.wav",
    "tr707/37_RimShot.wav",
    "tr707/38_Snare1.wav",
    "tr707/39_HandClap.wav",
    "tr707/40_Snare2.wav",
    "tr707/41_LowTom.wav",
    "tr707/42_HhC.wav",
    "tr707/43_MedTom.wav",
    "tr707/44_HhO.wav",
    "tr707/45_HiTom.wav",
    "tr707/46_Crash.wav",
    "tr707/47_Ride.wav",
    "tr707/48_Tamb.wav",
    "tr707/49_CowBell.wav",
    "a440.wav",
    "funky.wav",
    "drumloop.wav",
    "latin.wav",
    "singing.wav",
    "rhodes.wav",
};

// Autres constantes globales
const int NUM_SOUNDS = 16;
const int NUM_STEPS = 16;
const int INITIAL_BPM = 120;
const int MIXER_CHANNELS = 32;
const int SAMPLE_RATE = 44100;

const float GLOBAL_GAIN = 0.2f;

} // namespace adikdrum

#endif // CONSTANTS_H


#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <map>
#include <vector>
#include <string>

// Constantes mathématiques
const double PI = 3.14159265358979323846;

// Configuration de la map clavier -> son
const std::map<char, int> KEY_TO_SOUND_MAP = {
    {'q', 0}, {'s', 1}, {'d', 2}, {'f', 3},
    {'g', 4}, {'h', 5}, {'j', 6}, {'k', 7},
    {'a', 8}, {'z', 9}, {'e', 10}, {'r', 11},
    {'t', 12}, {'y', 13}, {'u', 14}, {'i', 15}
};

// Autres constantes globales si nécessaire
const int NUM_SOUNDS = 16;
const int NUM_STEPS = 16;
const int INITIAL_BPM = 120;
const int MIXER_CHANNELS = 32;
const int SAMPLE_RATE = 44100;

const float GLOBAL_GAIN = 0.2f;

const std::string MEDIA_DIR = "./media";
const std::vector<std::string> SOUND_LIST = {
        "a440.wav",
        "drumloop.wav",
        "funky.wav",
        "latin.wav",
        "singing.wav",
        "rhodes.wav",
    };


#endif // CONSTANTS_H

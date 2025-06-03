#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <map>
#include <vector>
#include <string>
#include <ncurses.h>

namespace adikdrum { // Ajout du namespace pour entourer les constantes

// Constantes mathématiques
const double PI = 3.14159265358979323846;

// Configuration de la map clavier -> son
const std::map<char, int> KEY_TO_SOUND_MAP = {
    {'q', 0}, {'s', 1}, {'d', 2}, {'f', 3},
    {'g', 4}, {'h', 5}, {'j', 6}, {'k', 7},
    {'a', 8}, {'z', 9}, {'e', 10}, {'r', 11},
    {'t', 12}, {'y', 13}, {'u', 14}, {'i', 15},
};

// /*
const std::map<char, int> KEYPAD_TO_SOUND_MAP = {
    {'1', 0}, {'2', 1}, {'3', 2}, {'4', 3},
    {'5', 4}, {'6', 5}, {'7', 6}, {'8', 7},
    // {KEY_PLUS, 11}, {KEY_MINUS, 12},

};
// */

/*
// Définition de la map pavé numérique -> son (si tu l'utilises)
const std::map<int, int> KEYPAD_TO_SOUND_MAP = {
    // Exemple pour ncurses (KEY_C1, KEY_C2, etc. sont des macros ncurses pour le pavé numérique)
    // Tu devras t'assurer que ces macros sont définies ou que tu utilises les valeurs numériques brutes
    // #include <ncurses.h> pour KEY_C1, KEY_C2 etc.
    {KEY_C1, 0}, {KEY_C2, 1}, {KEY_C3, 2}, {KEY_C4, 3},
    {KEY_C5, 4}, {KEY_C6, 5}, {KEY_C7, 6}, {KEY_C8, 7},
    {KEY_C9, 8}, {KEY_C0, 9}, {KEY_C_PERIOD, 10}, {KEY_C_PLUS, 11},
    {KEY_C_MINUS, 12}, {KEY_C_MULTIPLY, 13}, {KEY_C_DIVIDE, 14}, {KEY_C_ENTER, 15}
    // Note: KEY_C1 à KEY_C9 sont souvent 49 à 57 (chiffres ASCII) sur certains terminaux
    // Si tu veux les chiffres du pavé numérique spécifiquement, utilise les macros ncurses
    // ou gère les chiffres ASCII si tu n'utilises pas keypad(stdscr, TRUE) pour stdscr.
    // Pour l'instant, je les laisse avec KEY_C* comme si tu utilisais le pavé numérique Ncurses.
};
*/


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


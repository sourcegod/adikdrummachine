#include "consoleuiapp.h"
#include "adikdrum.h"
#include <iostream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <sstream>

// Fonction pour initialiser le terminal
termios ConsoleUIApp::initTermios(int echo) {
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
void ConsoleUIApp::resetTermios(termios oldt) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}
//----------------------------------------


ConsoleUIApp::ConsoleUIApp(AdikDrum& adikDrum) : adikDrum_(adikDrum) {}

bool ConsoleUIApp::init() {
    oldTerm = initTermios(0); // Réutilise la fonction d'initialisation du terminal
    if (tcgetattr(STDIN_FILENO, &oldTerm) == -1) {
        perror("tcgetattr");
        return false;
    }
    

    return true;
}

void ConsoleUIApp::run() {
    std::string msg = "Le clavier est initialisé.";
    displayMessage(msg);
    auto& pattern = adikDrum_.getPattern();
    auto numSounds = adikDrum_.getNumSounds();
    auto numSteps = adikDrum_.getNumSteps();
    displayGrid(pattern, adikDrum_.cursorPos, numSounds, numSteps); // Affiche la grille au démarrage

    char key;
    while (read(STDIN_FILENO, &key, 1) == 1) {
        if (key == 'Q') break;

        if (key == '\n') { // Touche Enter
            adikDrum_.selectStep();
        } else if (key == 127) { // Touche Backspace
            adikDrum_.unselectStep();
        } else if (key == ' ') { // Touche Espace
            adikDrum_.playPause();
        } else if (key == 'c') { // Touche 'c'
            adikDrum_.toggleClick();
        } else if (key == 'p') { // Touche 'p'
            adikDrum_.demo();
        } else if (key == 16) { // Ctrl+p
            adikDrum_.loadPattern();
        } else if (key == 'v') { // Touche 'v'
            adikDrum_.stopAllSounds();
        } else if (key == 'x') { // Touche 'x'
            adikDrum_.toggleMute();
        } else if (key == 'X') { // Touche 'X'
            adikDrum_.resetMute();
        } else if (key == '+') { // Touche '+'
            adikDrum_.changeVolume(0.1f);
        } else if (key == '-') { // Touche '-'
            adikDrum_.changeVolume(-0.1f);
        } else if (key == '(') { // Touche '('
            adikDrum_.changeBpm(-5.0f);
        } else if (key == ')') { // Touche ')'
            adikDrum_.changeBpm(5.0f);
        } else if (key == '[') { // Touche '['
            adikDrum_.changePan(-0.1f);
        } else if (key == ']') { // Touche ']'
            adikDrum_.changePan(0.1f);
        } else if (adikDrum_.keyToSoundMap.count(key)) {
            adikDrum_.playKey(key); // Gérer les autres touches pour jouer des sons
        } else if (key == 'l') { // Touche 'l'
            adikDrum_.triggerLastSound(); // Déclenche la relecture du dernier son
        } else if (key == 'm') { // Touche 'm'
            adikDrum_.playCurrentSound(); // Joue le son courant
        } else if (key == '\033') { // Code d'échappement
            read(STDIN_FILENO, &key, 1); // Lit '['
            read(STDIN_FILENO, &key, 1); // Lit le code de la flèche
            if (key == 'A') { // Flèche haut
                adikDrum_.moveCursorUp();
            } else if (key == 'B') { // Flèche bas
                adikDrum_.moveCursorDown();
            } else if (key == 'C') { // Flèche droite
                adikDrum_.moveCursorRight();
            } else if (key == 'D') { // Flèche gauche
                adikDrum_.moveCursorLeft();
            } // End arrow keys conditions
        } // End main key handling
    } // End the while Loop

}

void ConsoleUIApp::close() {
    // Resets the terminal
    resetTermios(oldTerm);
}

void ConsoleUIApp::displayMessage(const std::string& message) {
    std::cout << message << std::endl;
}

void ConsoleUIApp::displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<size_t, size_t> cursor, size_t numSounds, size_t numSteps) {
    std::ostringstream oss;
    oss << "  ";
    for (size_t i = 0; i < numSteps; ++i) {
        oss << (i + 1) % 10 << " ";
    }
    oss << std::endl;
    for (size_t i = 0; i < numSounds; ++i) {
        oss << (i + 1) % 10 << " ";
        for (size_t j = 0; j < numSteps; ++j) {
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

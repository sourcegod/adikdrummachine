/*
 * File: adiktui.cpp
 * Test on ncurses library
 * Compile: g++ adiktui.cpp -o adiktui -lncurses
 * Date: Sun, 18/05/2025
 * Author: Coolbrother
 * */

#include "adiktui.h"
#include "adikdrum.h"
#include "constants.h"

#include <iostream>
#include <iomanip>
#include <vector>

namespace adikdrum {

AdikTUI::AdikTUI(AdikDrum* adikDrum) 
    : adikDrum_(adikDrum),
    screenWidth_(0), 
    screenHeight_(0), 
    messageWindow_(nullptr), 
    gridWindow_(nullptr) {
}
//----------------------------------------

AdikTUI::~AdikTUI() {
    close();
}
//----------------------------------------

bool AdikTUI::init() {
    // Initialiser ncurses
    if (initscr() == NULL) {
        std::cerr << "Erreur lors de l'initialisation de ncurses." << std::endl;
        return false;
    }
    cbreak();             // Désactiver la mise en mémoire tampon de ligne, passer les frappes directement au programme
    noecho();             // Ne pas afficher les caractères saisis
    keypad(stdscr, TRUE); // Activer les touches spéciales comme les flèches
    getmaxyx(stdscr, screenHeight_, screenWidth_); // Obtenir les dimensions de l'écran
    curs_set(0); // Rendre le curseur invisible

    createWindows();

    return true;
}
//----------------------------------------

void AdikTUI::createWindows() {
     // Calculer les dimensions des fenêtres. Laisser de l'espace pour les bordures.
    int messageHeight = 3; // Exemple : 3 lignes pour les messages
    int gridHeight = screenHeight_ - messageHeight - 2; // -2 pour les bordures
    int gridWidth = screenWidth_ - 2;

    // Créer une fenêtre pour les messages en haut de l'écran
    messageWindow_ = newwin(messageHeight, screenWidth_, 0, 0);
    if (messageWindow_ == NULL) {
        std::cerr << "Erreur lors de la création de la fenêtre de message." << std::endl;
        endwin();
        exit(1); // Utiliser exit pour une gestion des erreurs plus robuste dans init
    }
    box(messageWindow_, 0, 0); // Dessiner une bordure
    wrefresh(messageWindow_);

    // Créer une fenêtre pour la grille
    gridWindow_ = newwin(gridHeight, gridWidth, messageHeight, 0);
    if (gridWindow_ == NULL)
    {
        std::cerr << "Erreur lors de la création de la fenêtre de grille." << std::endl;
        endwin();
        exit(1);
    }
    box(gridWindow_, 0, 0);
    wrefresh(gridWindow_);
}
//----------------------------------------

void AdikTUI::destroyWindows() {
    if (messageWindow_) {
        delwin(messageWindow_);
        messageWindow_ = nullptr;
    }
    if (gridWindow_) {
        delwin(gridWindow_);
        gridWindow_ = nullptr;
    }
}
//----------------------------------------

void AdikTUI::run() {
    std::string msg = "Le clavier est initialisé.";
    displayMessage(msg);

    // Assurez-vous que adikDrum_ est valide avant d'y accéder
    if (!adikDrum_) {
        displayMessage("Erreur: AdikDrum n'est pas initialisé.");
        return;
    }

    // Récupérer le pattern courant depuis drumPlayer_ (via AdikDrum)
    // Assurez-vous que curPattern_ est valide avant d'appeler getPatternBar
    auto curPattern = adikDrum_->getDrumPlayer().curPattern_;
    const auto& pattern = curPattern ? curPattern->getPatternBar(curPattern->getCurrentBar()) : std::vector<std::vector<bool>>();

    auto numSounds = adikDrum_->getNumSounds();
    auto numSteps = adikDrum_->getNumSteps();
    displayGrid(pattern, adikDrum_->cursorPos, numSounds, numSteps);

    int key; // Changed to int to properly handle ncurses special keys
    while ((key = getch()) != 'Q') { // 'Q' pour quitter
        switch (key) {
            case '\n': // Entrée
                adikDrum_->selectStep();
                break;
            case KEY_BACKSPACE: // Backspace
                adikDrum_->unselectStep();
                break;
            case ' ': // Espace
            case '0': // '0'
                adikDrum_->playPause();
                break;
            case 'c': // Metronome
                adikDrum_->toggleClick();
                break;
            case 'p': // Demo
                adikDrum_->demo();
                break;
            case 16: // Ctrl+p (assumant que 16 est le code ASCII pour Ctrl+p)
                adikDrum_->loadPattern();
                break;
            case 'v': // Stop all sounds
            case '.': // '.'
                adikDrum_->stopAllSounds();
                break;
            case 'x': // Mute current sound
                adikDrum_->toggleMute();
                break;
            case 'X': // Reset all mute
                adikDrum_->resetMute();
                break;
            case '+': // Increase volume
                adikDrum_->changeVolume(0.1f);
                break;
            case '-': // Decrease volume
                adikDrum_->changeVolume(-0.1f);
                break;
            case '(': // Decrease BPM
                adikDrum_->changeBpm(-5.0f);
                break;
            case ')': // Increase BPM
                adikDrum_->changeBpm(5.0f);
                break;
            case '[': // Pan left
                adikDrum_->changePan(-0.1f);
                break;
            case ']': // Pan right
                adikDrum_->changePan(0.1f);
                break;
            case '{': // Decrease speed
                adikDrum_->changeSpeed(-0.25f);
                break;
            case '}': // Increase speed
                adikDrum_->changeSpeed(0.25f);
                break;
            case 'D': // Toggle Delay
                adikDrum_->toggleDelay();
                break;
            case 'l': // 'l'
            case '9': // '9' (as per user's updated run function)
                adikDrum_->triggerLastSound();
                break;
            case 'm': // 'm'
                adikDrum_->playCurrentSound();
                break;
            case '/': // '/'
                adikDrum_->changeShiftPad(-8);
                break;
            case '*': // '*'
                adikDrum_->changeShiftPad(8);
                break;
            case '<': // Go to start of pattern
                adikDrum_->gotoStart();
                break;
            case '>': // Go to end of pattern
                adikDrum_->gotoEnd();
                break;
            case KEY_UP:
                adikDrum_->moveCursorUp();
                break;
            case KEY_DOWN:
                adikDrum_->moveCursorDown();
                break;
            case KEY_LEFT:
                adikDrum_->moveCursorLeft();
                break;
            case KEY_RIGHT:
                adikDrum_->moveCursorRight();
                break;
            case KEY_PPAGE: // PageUp pour mesure précédente
                adikDrum_->changeBar(-1);
                break;
            case KEY_NPAGE: // PageDown pour mesure suivante
                adikDrum_->changeBar(1);
                break;
            default:
                // Vérifier si la touche correspond à un son
                // Note: KEY_TO_SOUND_MAP est dans constants.h
                auto it = KEY_TO_SOUND_MAP.find(key);
                if (it != KEY_TO_SOUND_MAP.end()) {
                    adikDrum_->playKey(key);
                } else if (KEYPAD_TO_SOUND_MAP.count(key)) { // Check for keypad keys
                    adikDrum_->playKeyPad(key);
                } else {
                    displayMessage(std::string("Touche pressée : ") + (char)key);
                }
                break;
        }
        // Re-get the pattern after potential changes by AdikDrum functions
        const auto& updatedPattern = adikDrum_->getCurPattern() ? adikDrum_->getCurPattern()->getPatternBar(adikDrum_->getCurPattern()->getCurrentBar()) : std::vector<std::vector<bool>>();
        displayGrid(updatedPattern, adikDrum_->cursorPos, numSounds, numSteps);
    }
}

/*
void AdikTUI::run() {
    std::string msg = "Le clavier est initialisé.";
    displayMessage(msg);

    // Récupérer le pattern courant depuis drumPlayer_ (via AdikDrum)
    // Assurez-vous que curPattern_ est valide avant d'appeler getPatternBar
    auto curPattern = adikDrum_.getDrumPlayer().curPattern_;
    const auto& pattern = curPattern ? curPattern->getPatternBar(curPattern->getCurrentBar()) : std::vector<std::vector<bool>>();

    auto numSounds = adikDrum_.getNumSounds();
    auto numSteps = adikDrum_.getNumSteps();
    displayGrid(pattern, adikDrum_.cursorPos, numSounds, numSteps);

    int key;
    while ((key = getch()) != 'Q') {
        // beep();
        switch (key) {
            case '\n': // Enter
                adikDrum_.selectStep();
                break;
            case 127: // Backspace
                adikDrum_.unselectStep();
                break;
            case ' ': // Space
            case '0': // '0'
                adikDrum_.playPause();
                break;
            case 'c':
                adikDrum_.toggleClick();
                break;
            case 'p':
                adikDrum_.demo();
                break;
            case 16: // Ctrl+p
                adikDrum_.loadPattern();
                break;
            case 'v': // 'v'
            case '.': // '.'
                adikDrum_.stopAllSounds();
                break;
            case 'x':
                adikDrum_.toggleMute();
                break;
            case 'X':
                adikDrum_.resetMute();
                break;
            case '+':
                adikDrum_.changeVolume(0.1f);
                break;
            case '-':
                adikDrum_.changeVolume(-0.1f);
                break;
            case '(':
                adikDrum_.changeBpm(-5.0f);
                break;
            case ')':
                adikDrum_.changeBpm(5.0f);
                break;
            case '[':
                adikDrum_.changePan(-0.1f);
                break;
            case ']':
                adikDrum_.changePan(0.1f);
                break;
            case '{':
                 adikDrum_.changeSpeed(-0.25f);
                 break;
            case '}':
                adikDrum_.changeSpeed(0.25f);
                break;
            case 'D':
                adikDrum_.toggleDelay();
                break;
            case 'l':  // 'l'
            case '9':  // '.'
                adikDrum_.triggerLastSound();
                break;
            case 'm':
                adikDrum_.playCurrentSound();
                break;
            case '/': // '/'
                adikDrum_.changeShiftPad(-8);
                break;
            case '*': // '*'
                adikDrum_.changeShiftPad(8);
                break;
            case '<': // Go to start of pattern
                adikDrum_.gotoStart();
                break;
            case '>': // Go to end of pattern
                adikDrum_.gotoEnd();
                break;

            case KEY_UP:
                adikDrum_.moveCursorUp();
                break;
            case KEY_DOWN:
                adikDrum_.moveCursorDown();
                break;
            case KEY_LEFT:
                adikDrum_.moveCursorLeft();
                break;
            case KEY_RIGHT:
                adikDrum_.moveCursorRight();
                break;

            case KEY_PPAGE: // PageUp pour mesure précédente
                adikDrum_.changeBar(-1);
                break;
            case KEY_NPAGE: // PageDown pour mesure suivante
                adikDrum_.changeBar(1);
                break;
            default:
                if (KEY_TO_SOUND_MAP.count(key)) {
                    adikDrum_.playKey(key);
                } else if (KEYPAD_TO_SOUND_MAP.count(key)) {
                    adikDrum_.playKeyPad(key);
                } else {
                    displayMessage(std::string("Touche pressée : ") + (char)key);
                }
                break;

        } // End of switch

        // Re-get the pattern after potential changes by AdikDrum functions
        const auto& curPattern = adikDrum_.getCurPattern();
        const auto& updatedPattern = curPattern ? curPattern->getPatternBar(curPattern->getCurrentBar()) : std::vector<std::vector<bool>>();
        displayGrid(updatedPattern, adikDrum_.cursorPos, numSounds, numSteps);
        // displayGrid(pattern, adikDrum_.cursorPos, numSounds, numSteps); // Update grid after each input

    } // End of while loop

}
//----------------------------------------
*/


void AdikTUI::close() {
    destroyWindows();
    endwin(); // Nettoyer ncurses
}
//----------------------------------------

void AdikTUI::displayMessage(const std::string& message) {
    if (messageWindow_ == nullptr) return;

    werase(messageWindow_); // Effacer la fenêtre
    // Note: For more accessibility with the Screen Reader, you must erase the screen, Refresh the screen and pause for 50 ms, and after, print the right message 
    wrefresh(messageWindow_);       // Rafraîchir la fenêtre pour afficher les modifications
     napms(50); // Pause de 50ms  
    wmove(messageWindow_, 1, 0); // Déplace le curseur au  début de la ligne
    // wclrtoeol(messageWindow_); // Efface du curseur jusqu'à la fin de la ligne.
    box(messageWindow_, 0, 0);
    mvwprintw(messageWindow_, 1, 0, message.c_str()); // Afficher le message
    wmove(messageWindow_, 1, 0); // Déplace le curseur au  début de la ligne
    wrefresh(messageWindow_);       // Rafraîchir la fenêtre pour afficher les modifications

     // napms(500); // pause de 500ms  
     // beep();

}
//----------------------------------------

void AdikTUI::displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<size_t, size_t> cursor, size_t numSounds, size_t numSteps) {
    return;
    if (gridWindow_ == nullptr) return;

    werase(gridWindow_);
    box(gridWindow_, 0, 0);

    // Calculer les dimensions des cellules
    int gridHeight = getmaxy(gridWindow_) - 2; // -2 pour la bordure
    int gridWidth  = getmaxx(gridWindow_) - 2;
    float cellHeight = (float)gridHeight / numSounds;
    float cellWidth  = (float)gridWidth  / numSteps;

     // Gestion des erreurs : Vérifier les dimensions nulles
    if (cellHeight <= 0 || cellWidth <= 0) {
        mvwprintw(gridWindow_, 1, 1, "Grille trop petite pour être affichée.");
        wrefresh(gridWindow_);
        return;
    }

    // Afficher la grille
    for (size_t i = 0; i < numSounds; ++i) {
        for (size_t j = 0; j < numSteps; ++j) {
            int y = 1 + static_cast<int>(i * cellHeight); // +1 pour la bordure
            int x = 1 + static_cast<int>(j * cellWidth);

            // S'assurer que nous n'écrivons pas en dehors des limites de la fenêtre.
            if (y < gridHeight && x < gridWidth) {
                 if (grid[i][j]) {
                    wattron(gridWindow_, A_STANDOUT); // Mettre en évidence les cellules actives
                    mvwprintw(gridWindow_, y, x, "██");
                    wattroff(gridWindow_, A_STANDOUT);
                }
                else
                {
                     mvwprintw(gridWindow_, y, x, "  ");
                }
            }
           
        }
    }

    // Afficher le curseur
    int cursorY = 1 + static_cast<int>(cursor.first * cellHeight);
    int cursorX = 1 + static_cast<int>(cursor.second * cellWidth);
     if (cursorY < gridHeight && cursorX < gridWidth)
        {
             wattron(gridWindow_, A_REVERSE);
             mvwprintw(gridWindow_, cursorY, cursorX, "><");
             wattroff(gridWindow_, A_REVERSE);
        }

    wrefresh(gridWindow_);
}
//----------------------------------------

} // namespace adikdrum

int main() {
    adikdrum::AdikDrum adikDrumApp(nullptr); // Créer AdikDrum sans UIApp pour l'instant
    adikdrum::AdikTUI ui(&adikDrumApp); // Créer AdikTUI et passer une référence à AdikDrum
    adikDrumApp.setUIApp(&ui); // Assigner l'UIApp à AdikDrum via la nouvelle méthode

    if (!adikDrumApp.initApp()) {
        // Gérer l'erreur d'initialisation de l'application
        std::cerr << "Erreur: L'initialisation de l'application a échoué." << std::endl;
        return 1; // Retourner un code d'erreur
    }

    if (ui.init()) {
        ui.run();
        ui.close(); // Fermer correctement l'interface utilisateur
    }
    else
    {
       std::cerr << "Erreur: L'initialisation de l'interface utilisateur a échoué." << std::endl;
       adikDrumApp.closeApp();
       return 1;
    }

    adikDrumApp.closeApp(); // Fermer correctement l'application AdikDrum

    return 0; // Retourner un code de succès
}

/*
int main() {
    adikdrum::AdikTUI ui;
    if (!ui.init()) {
        std::cerr << "Échec de l'initialisation de l'interface utilisateur." << std::endl;
        return 1;
    }
    ui.run();
    ui.close();
    return 0;
}
*/


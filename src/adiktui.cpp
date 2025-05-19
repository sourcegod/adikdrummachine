/*
 * File: adiktui.cpp
 * Test on ncurses library
 * Compile: g++ adiktui.cpp -o adiktui -lncurses
 * Date: Sun, 18/05/2025
 * Author: Coolbrother
 * */

#include "adiktui.h"
#include "adikdrum.h"

#include <iostream>
#include <iomanip>
#include <vector>

namespace adikdrum {

AdikTUI::AdikTUI(AdikDrum& adikDrum) 
        : adikDrum_(adikDrum),
        screenWidth_(0), screenHeight_(0), messageWindow_(nullptr), gridWindow_(nullptr) {
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
    int ch;
    while ((ch = getch()) != 'q') { // 'q' pour quitter
        switch (ch) {
            case char('p'):
            adikDrum_.demo();
            break;
            case KEY_UP:
                displayMessage("Flèche haut pressée");
                break;
            case KEY_DOWN:
                displayMessage("Flèche bas pressée");
                break;
            case KEY_LEFT:
                displayMessage("Flèche gauche pressée");
                break;
            case KEY_RIGHT:
                displayMessage("Flèche droite pressée");
                break;
            default:
                displayMessage(std::string("Touche pressée : ") + (char)ch);
                break;
        }
    }
}
//----------------------------------------

void AdikTUI::close() {
    destroyWindows();
    endwin(); // Nettoyer ncurses
}
//----------------------------------------

void AdikTUI::displayMessage(const std::string& message) {
    if (messageWindow_ == nullptr) return;

    // werase(messageWindow_); // Effacer la fenêtre
    wmove(messageWindow_, 1, 0); // Déplace le curseur au  début de la ligne
    wclrtoeol(messageWindow_); // Efface du curseur jusqu'à la fin de la ligne.
    box(messageWindow_, 0, 0);
    mvwprintw(messageWindow_, 1, 0, message.c_str()); // Afficher le message
    wmove(messageWindow_, 1, 0); // Déplace le curseur au  début de la ligne
     wrefresh(messageWindow_);       // Rafraîchir la fenêtre pour afficher les modifications

     napms(500); // pause de 500ms  
     beep();

}
//----------------------------------------

void AdikTUI::displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<size_t, size_t> cursor, size_t numSounds, size_t numSteps) {
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
    AdikDrum adikDrumApp(nullptr); // Créer AdikDrum sans UIApp pour l'instant
    AdikTUI textUI(adikDrumApp); // Créer AdikTUIApp en passant une référence à AdikDrum
    adikDrumApp.uiApp_ = &textUI; // Assigner l'UIApp à AdikDrum
    if (!adikDrumApp.initApp()) {
        return false; // Changer le code de retour en cas d'erreur
    }

    if (textUI.init()) {
        textUI.run();
        textUI.close();
    }
    adikDrumApp.closeApp();

    return 0;
}
//----------------------------------------

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


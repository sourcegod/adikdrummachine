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
#include "adikcommands.h"

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

    if (!adikDrum_) {
        displayMessage("Erreur: AdikDrum n'est pas initialisé.");
        return;
    }

    auto numSounds = adikDrum_->getNumSounds();
    auto numSteps = adikDrum_->getNumSteps();

    int key;
    while ((key = getch()) != 'Q') {
        // adikDrum_->update();
        if (key == 27) { // Code ASCII pour ESC (Escape)
            if (currentUIMode_ != UIMode::NORMAL) {
                currentUIMode_ = UIMode::NORMAL;
                noecho();    // Désactive l'écho (Ncurses) si on était en mode commande
                curs_set(0); // Masque le curseur (Ncurses) si on était en mode commande
                clearCommandInputLine(); // Nettoie la ligne de commande si on était en mode commande
                displayMessage("Retour au mode : " + getUIModeName(currentUIMode_));
            } else {
                beep(); // Déjà en mode NORMAL, émet un son
                displayMessage("Déjà en mode : " + getUIModeName(currentUIMode_));
            }
            continue; // Important: Traite la touche et passe à la prochaine itération
        } else if (key == '\t') { // Touche Tab
            int nextMode = static_cast<int>(currentUIMode_) + 1;
            // Assurez-vous que UIMode::NUM_MODES est bien la dernière entrée de votre enum UIMode
            if (nextMode < static_cast<int>(UIMode::NUM_MODES)) {
                currentUIMode_ = static_cast<UIMode>(nextMode);
                displayMessage("Mode : " + getUIModeName(currentUIMode_));
            } else {
                beep(); // Dernier mode, émet un son
                displayMessage("Déjà au dernier mode (" + getUIModeName(currentUIMode_) + ").");
            }
            continue; // Important: Traite la touche et passe à la prochaine itération
                      // pour éviter qu'elle soit traitée par le switch des modes.
        } else if (key == KEY_BTAB) { // Touche Shift+Tab
            int prevMode = static_cast<int>(currentUIMode_) - 1;
            if (prevMode >= static_cast<int>(UIMode::NORMAL)) { // UIMode::NORMAL est censé être le premier
                currentUIMode_ = static_cast<UIMode>(prevMode);
                displayMessage("Mode : " + getUIModeName(currentUIMode_));
            } else {
                beep(); // Premier mode, émet un son
                displayMessage("Déjà au premier mode (" + getUIModeName(currentUIMode_) + ").");
            }
            continue; // Important: Traite la touche et passe à la prochaine itération
        }

        switch (currentUIMode_) {
            case UIMode::NORMAL: {
                switch (key) {
                    case ':': // Bascule en mode commande
                        currentUIMode_ = UIMode::COMMAND_INPUT;
                        commandInputBuffer_.clear();
                        commandCursorPos_ = 0;
                        echo();
                        curs_set(1);
                        drawCommandInputLine();
                        break;
                    // Ajoutez ici d'autres raccourcis clavier qui ne sont PAS des déclencheurs de sons
                    // et qui doivent fonctionner en mode NORMAL.
                    case '\n': adikDrum_->selectStep(); break;
                    case KEY_BACKSPACE: adikDrum_->unselectStep(); break;
                    case ' ': adikDrum_->playPause(); break;
                    case 'c': adikDrum_->toggleClick(); break;
                    case 'p': adikDrum_->demo(); break;
                    case 'v':
                    case '.': adikDrum_->stopAllSounds(); break;
                    case 'x': adikDrum_->toggleMute(); break;
                    case 'X': adikDrum_->resetMute(); break;
                    case '+': adikDrum_->changeVolume(0.1f); break;
                    case '-': adikDrum_->changeVolume(-0.1f); break;
                    case '(': adikDrum_->changeBpm(-5.0f); break;
                    case ')': adikDrum_->changeBpm(5.0f); break;
                    case '[': adikDrum_->changePan(-0.1f); break;
                    case ']': adikDrum_->changePan(0.1f); break;
                    case '{': adikDrum_->changeSpeed(-0.25f); break;
                    case '}': adikDrum_->changeSpeed(0.25f); break;
                    case 'D': adikDrum_->toggleDelay(); break;
                    case '<': adikDrum_->gotoStart(); break;
                    case '>': adikDrum_->gotoEnd(); break;
                    case 4: adikDrum_->clearCurrentSound(); break; // Ctrl+D
                    case 8: adikDrum_->toggleHelp(); break; // Ctrl+H
                    case 11: adikDrum_->clearLastPlayedSound(); break; // Ctrl+K
                    case 16: adikDrum_->loadPattern(); break; // Ctrl+P
                    case 18: adikDrum_->toggleRecord(); break; // Ctrl+R
                    case 20: adikDrum_->test(); break; // Ctrl+T
                    case 21: adikDrum_->showStatus(); break; // Ctrl+U

                    case KEY_UP: adikDrum_->moveCursorUp(); break;
                    case KEY_DOWN: adikDrum_->moveCursorDown(); break;
                    case KEY_LEFT: adikDrum_->moveCursorLeft(); break;
                    case KEY_RIGHT: adikDrum_->moveCursorRight(); break;
                    case KEY_PPAGE: adikDrum_->changeBar(-1); break;
                    case KEY_NPAGE: adikDrum_->changeBar(1); break;
                    case KEY_DC: adikDrum_->deleteLastPlayedStep(); break;

                    default: // Si la touche n'est pas un raccourci de mode NORMAL
                        // Tente de la gérer comme une touche de son.
                        // handleKeySound(key);
                        break;
                }
                break;
            } // Fin du case UIMode::NORMAL

            case UIMode::KEY_SOUND: {
                // Ce mode n'est pas activé par une touche spécifique dans `run()`.
                // Il est géré comme le `default` du mode `NORMAL`.
                // Cependant, si tu avais une touche pour passer *explicitement* en mode KEY_SOUND,
                // alors la logique de handleKeySound(key) irait ici.
                // Pour l'instant, c'est le "catch-all" du mode NORMAL.
                handleKeySound(key); // Appelle la fonction de gestion des sons
                break;
            }

            case UIMode::COMMAND_INPUT: {
                handleCommandInput(key);
                break;
            }
        } // Fin du switch (currentUIMode_)

        // --- Section de rafraîchissement de l'affichage principal ---
        // Ne rafraîchit que si l'aide n'est pas affichée et qu'on est en mode NORMAL ou KEY_SOUND
        // (le mode KEY_SOUND étant implicitement le 'default' de NORMAL ici,
        // cette condition suffit pour rafraîchir l'affichage normal).
        if (!adikDrum_->isHelpDisplayed() && currentUIMode_ != UIMode::COMMAND_INPUT) {
            const auto& updatedPattern = adikDrum_->getDrumPlayer().curPattern_ ? adikDrum_->getDrumPlayer().curPattern_->getPatternBar(adikDrum_->getDrumPlayer().curPattern_->getCurrentBar()) : std::vector<std::vector<bool>>();
            // displayGrid(updatedPattern, adikDrum_->cursorPos, numSounds, numSteps);
            // displayMessage(adikDrum_->getMsgText());
        }
    } // Fin de la boucle while
}
//----------------------------------------

void AdikTUI::close() {
    destroyWindows();
    endwin(); // Nettoyer ncurses
}
//----------------------------------------

void AdikTUI::displayMessage(const std::string& message) {
    if (messageWindow_ == nullptr) return;

    werase(messageWindow_); // Effacer tout le contenu de la fenêtre de message
    wrefresh(messageWindow_); // Rafraîchir pour vider l'écran (important pour le lecteur d'écran)
    napms(50); // Petite pause pour s'assurer que l'effacement est perçu

    box(messageWindow_, 0, 0); // Redessine la bordure

    // Calcule la hauteur de la fenêtre pour trouver l'avant-dernière ligne
    int max_y, max_x;
    getmaxyx(messageWindow_, max_y, max_x);

    // Positionne le message sur l'avant-dernière ligne (max_y - 2 pour la bordure inférieure et une ligne au-dessus)
    // Commence à la colonne 1 pour être à l'intérieur de la bordure gauche.
    mvwprintw(messageWindow_, max_y - 2, 1, "%s", message.c_str());

    // Déplace le curseur texte au début de la ligne du message pour l'accessibilité
    wmove(messageWindow_, max_y - 2, 1);

    wrefresh(messageWindow_); // Rafraîchit pour afficher le message et positionner le curseur
    // napms(500); // pause de 500ms (commenté, car potentiellement gênant pour l'utilisateur)
    // beep(); // (commenté, à activer si vous voulez un feedback sonore)
}
//----------------------------------------

void AdikTUI::displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<size_t, size_t> cursor, size_t numSounds, size_t numSteps) {
    return;
    if (gridWindow_ == nullptr) return;

    werase(gridWindow_); // Efface le contenu précédent de la fenêtre
    box(gridWindow_, 0, 0); // Dessine la bordure de la fenêtre

    // Lignes et colonnes de départ pour l'affichage de la grille à l'intérieur de la bordure
    int displayStartY = 1;
    int displayStartX = 1;

    // Afficher la grille
    for (size_t i = 0; i < numSounds; ++i) { // Itère sur les lignes (sons)
        for (size_t j = 0; j < numSteps; ++j) { // Itère sur les colonnes (pas)
            char charToDisplay;

            if (i == cursor.second && j == cursor.first) {
                // Si c'est la position du curseur
                charToDisplay = 'X';
            } else if (grid[i][j]) {
                // Si le pad est activé
                charToDisplay = '#';
            } else {
                // Si le pad est inactif
                charToDisplay = '-';
            }
            // Afficher le caractère, suivi d'un espace pour la clarté
            // Chaque élément prend 2 colonnes (caractère + espace)
            mvwprintw(gridWindow_, displayStartY + i, displayStartX + (j * 2), "%c ", charToDisplay);
        }
    }

    // Repositionner le curseur du terminal après l'affichage de la grille
    // Le curseur est sur `cursorPos.second` (ligne) et `cursorPos.first` (colonne).
    // Chaque élément de la grille prend 2 caractères (caractère + espace), donc on multiplie la colonne par 2.
    // On ajoute displayStartY et displayStartX pour tenir compte des bordures.
    wmove(gridWindow_, displayStartY + cursor.second, displayStartX + (cursor.first * 2));

    wrefresh(gridWindow_); // Met à jour l'affichage de la fenêtre et repositionne le curseur
}
//----------------------------------------

std::string AdikTUI::getUIModeName(UIMode mode) {
    switch (mode) {
        case UIMode::NORMAL: return "NORMAL";
        case UIMode::KEY_SOUND: return "KEY SOUND";
        case UIMode::COMMAND_INPUT: return "COMMAND INPUT";
        default: return "INCONNU";
    }
}
//----------------------------------------

void AdikTUI::printKeyCode(int key) {
    // Affiche le code de la touche pressée
    mvprintw(0, 0, "Key Code: %d (Char: '%c')  ", key, key);
    refresh(); // Rafraîchit l'écran pour montrer le code

}
//----------------------------------------

void AdikTUI::clearCommandInputLine() {
    if (messageWindow_) {
        // Efface la ligne du message, où la commande est affichée
        werase(messageWindow_); // Efface la ligne entière de messageWindow_
        wrefresh(messageWindow_);
    }
}
//----------------------------------------

void AdikTUI::drawCommandInputLine() {
    if (messageWindow_) {
        // Redessine la ligne de commande sur messageWindow_
        werase(messageWindow_); // Efface d'abord tout contenu précédent
        mvwprintw(messageWindow_, 0, 0, ":%s", commandInputBuffer_.c_str());
        wmove(messageWindow_, 0, 1 + commandCursorPos_); // Déplace le curseur Ncurses
        wrefresh(messageWindow_);
    }
}
//----------------------------------------

void AdikTUI::executeCommand(const CommandInput& cmd) {
    if (cmd.commandName.empty()) {
        displayMessage("Commande vide. Annulé.");
        return;
    }

    // Recherche de la commande dans la COMMAND_MAP
    auto it = COMMAND_MAP.find(cmd.commandName);

    if (it != COMMAND_MAP.end()) {
        // Commande trouvée, exécute l'action associée
        try {
            it->second.action(adikDrum_, cmd.args);
            // Si la commande s'exécute sans erreur, on peut afficher un message de succès
            // ou laisser la fonction du drum mettre à jour le messageText.
            // Si la fonction du drum met à jour msgText_, on peut faire:
            displayMessage(adikDrum_->getMsgText());
        } catch (const std::invalid_argument& e) {
            displayMessage("Erreur d'argument pour '" + cmd.commandName + "': " + e.what());
        } catch (const std::out_of_range& e) {
            displayMessage("Argument hors de portée pour '" + cmd.commandName + "': " + e.what());
        } catch (const std::exception& e) {
            displayMessage("Erreur lors de l'exécution de '" + cmd.commandName + "': " + e.what());
        }
    } else {
        // Commande non trouvée
        displayMessage("Commande inconnue: " + cmd.commandName);
    }
}
//----------------------------------------

void AdikTUI::handleCommandInput(int key) {
    switch (key) {
        case KEY_BACKSPACE: // Gère la touche Retour arrière
        case 127:           // Code ASCII pour Backspace
        case '\b':          // Code ASCII pour Backspace
            if (commandCursorPos_ > 0) {
                commandInputBuffer_.erase(commandInputBuffer_.begin() + commandCursorPos_ - 1);
                commandCursorPos_--;
            }
            break;

        case KEY_LEFT: // Gère la flèche gauche
            if (commandCursorPos_ > 0) {
                commandCursorPos_--;
            }
            break;

        case KEY_RIGHT: // Gère la flèche droite
            if (commandCursorPos_ < commandInputBuffer_.length()) {
                commandCursorPos_++;
            }
            break;

        case '\n': // Gère la touche Entrée (validation)
        case '\r':
            {
                // Utilise la fonction parseCommandString qui est maintenant globale dans adikcommands.h
                CommandInput cmd = parseCommandString(commandInputBuffer_);
                executeCommand(cmd); // Exécute la commande
            }
            // Réinitialise le mode d'interface
            currentUIMode_ = UIMode::NORMAL;
            noecho();    // Désactive l'écho (Ncurses)
            curs_set(0); // Masque le curseur (Ncurses)
            // Nettoie la ligne de commande, puis réaffiche le message normal du drum
            clearCommandInputLine();
            displayMessage(adikDrum_->getMsgText()); // Utilise getMsgText() si msgText_ est privé
            break;

        case 27: // Code ASCII pour ESC (Escape)
            // Quitte le mode commande sans valider
            currentUIMode_ = UIMode::NORMAL;
            noecho();    // Désactive l'écho (Ncurses)
            curs_set(0); // Masque le curseur (Ncurses)
            clearCommandInputLine(); // Nettoie la ligne de commande
            displayMessage("Mode commande annulé.");
            beep(); // Joue un son pour indiquer l'annulation
            break;

        default:
            // Si c'est un caractère imprimable, l'ajouter au buffer
            if (key >= 32 && key <= 126) { // Caractères ASCII imprimables
                commandInputBuffer_.insert(commandInputBuffer_.begin() + commandCursorPos_, static_cast<char>(key));
                commandCursorPos_++;
            }
            break;
    }
    // Redessine toujours le champ après chaque touche en mode commande
    drawCommandInputLine();
}
//----------------------------------------

void AdikTUI::handleKeySound(int key) {
    switch(key) {
        case '0': adikDrum_->playPause(); break;
        case 'l':
        case '9': adikDrum_->triggerLastSound(); break;
        case 'm': adikDrum_->playCurrentSound(); break;
        case '.': adikDrum_->stopAllSounds(); break;
        case '-': adikDrum_->changeShiftPad(-8); break;
        case '+': adikDrum_->changeShiftPad(8); break;
        
        default:

            int soundIndex = -1;

            // Tente de trouver le son dans la map des touches principales
            auto it = KEY_TO_SOUND_MAP.find(key);
            if (it != KEY_TO_SOUND_MAP.end()) {
                soundIndex = it->second;
            } else {
                // Si non trouvé, tente de trouver le son dans la map du pavé numérique
                auto it_num = KEYPAD_TO_SOUND_MAP.find(key);
                if (it_num != KEYPAD_TO_SOUND_MAP.end()) {
                    soundIndex = it_num->second;
                }
            }

            if (soundIndex != -1) {
                if (adikDrum_->getDrumPlayer().isRecording()) {
                    adikDrum_->recordSound(soundIndex);
                } else {
                    adikDrum_->playKey(soundIndex);
                }
            }
        break;
    } // Fin du switch


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
//----------------------------------------


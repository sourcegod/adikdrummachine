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
    displayMessage(msg); // Affiche le message initial

    if (!adikDrum_) {
        displayMessage("Erreur: AdikDrum n'est pas initialisé.");
        return;
    }

    // Ces variables sont utilisées pour l'affichage de la grille
    auto numSounds = adikDrum_->getNumSounds();
    auto numSteps = adikDrum_->getNumSteps();

    int key;
    // Utilise wgetch pour lire spécifiquement de gridWindow_ en mode normal,
    // car c'est là que le curseur de navigation est pertinent.
    // Cependant, pour le mode commande, on lira de messageWindow_ ou stdscr.
    // Pour simplifier l'exemple, je vais garder getch() qui lit de stdscr par défaut.
    // Si tu utilises des sous-fenêtres pour les entrées, il faut être vigilant.
    // Le plus simple pour gérer les modes est souvent de lire de stdscr.
    while ((key = getch()) != 'Q') { // 'Q' pour quitter
        // Mettre à jour l'état de l'application (playback, etc.) en continu
        // adikDrum_->update();

        // Gérer les entrées clavier selon le mode de l'UI
        switch (currentUIMode_) {
            case UIMode::NORMAL: {
                // --- GESTION DES TOUCHES EN MODE NORMAL ---
                switch (key) {
                    case ':': // Bascule en mode commande
                        currentUIMode_ = UIMode::COMMAND_INPUT;
                        commandInputBuffer_.clear();
                        commandCursorPos_ = 0;
                        echo();      // Active l'écho temporairement pour la saisie (Ncurses)
                        curs_set(1); // Affiche le curseur (Ncurses)
                        drawCommandInputLine(); // Affiche le prompt vide sur messageWindow_
                        break;
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
                    case '9': // '9'
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
                    case 4: // Ctrl+D
                        adikDrum_->clearCurrentSound();
                        break;
                    case 8: // Ctrl+H
                        adikDrum_->toggleHelp();
                        break;
                    case 11: // Ctrl+K
                        adikDrum_->clearLastPlayedSound();
                        break;
                    case 16: // Ctrl+P
                        adikDrum_->loadPattern();
                        break;
                    case 18: // Ctrl+R
                        adikDrum_->toggleRecord();
                        break;
                    case 20: // Ctrl+T
                        adikDrum_->test();
                        break;
                    case 21: // Ctrl+U
                        adikDrum_->showStatus();
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
                    case KEY_PPAGE: // PageUp
                        adikDrum_->changeBar(-1);
                        break;
                    case KEY_NPAGE: // PageDown
                        adikDrum_->changeBar(1);
                        break;
                    case KEY_DC: // Delete
                        adikDrum_->deleteLastPlayedStep();
                        break;
                    default: {
                        int soundIndex = -1;
                        auto it = KEY_TO_SOUND_MAP.find(key);
                        if (it != KEY_TO_SOUND_MAP.end()) {
                            soundIndex = it->second;
                        } else {
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
                    }
                } // Fin du switch (key) en mode NORMAL
                break;
            } // Fin du case UIMode::NORMAL

            case UIMode::COMMAND_INPUT: {
                // --- GESTION DES TOUCHES EN MODE SAISIE DE COMMANDE ---
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
                            CommandInput cmd = parseCommandString(commandInputBuffer_);
                            executeCommand(cmd); // Exécute la commande
                        }
                        // Réinitialise le mode d'interface
                        currentUIMode_ = UIMode::NORMAL;
                        noecho();    // Désactive l'écho (Ncurses)
                        curs_set(0); // Masque le curseur (Ncurses)
                        // Nettoie la ligne de commande, puis réaffiche le message normal du drum
                        clearCommandInputLine();
                        // displayMessage(adikDrum_->msgText_);
                        break;

                    case 27: // Code ASCII pour ESC (Escape)
                        // Quitte le mode commande sans valider
                        currentUIMode_ = UIMode::NORMAL;
                        noecho();    // Désactive l'écho (Ncurses)
                        curs_set(0); // Masque le curseur (Ncurses)
                        clearCommandInputLine(); // Nettoie la ligne de commande
                        displayMessage("Mode commande annulé.");
                        break;

                    default:
                        // Si c'est un caractère imprimable, l'ajouter au buffer
                        if (key >= 32 && key <= 126) { // Caractères ASCII imprimables
                            commandInputBuffer_.insert(commandInputBuffer_.begin() + commandCursorPos_, static_cast<char>(key));
                            commandCursorPos_++;
                        }
                        break;
                }
                drawCommandInputLine(); // Redessine le champ après chaque touche en mode commande
                break;
            } // Fin du case UIMode::COMMAND_INPUT
        } // Fin du switch (currentUIMode_)

        // --- Section de rafraîchissement de l'affichage principal ---
        // Cette section est exécutée à chaque tour de boucle, mais son contenu dépend du mode UI.
        if (!adikDrum_->isHelpDisplayed() && currentUIMode_ == UIMode::NORMAL) {
            // Seule la grille est mise à jour si l'aide n'est pas affichée et que nous sommes en mode normal.
            // Le message a déjà été mis à jour par displayMessage si une action normale a eu lieu.
            const auto& updatedPattern = adikDrum_->getDrumPlayer().curPattern_ ? adikDrum_->getDrumPlayer().curPattern_->getPatternBar(adikDrum_->getDrumPlayer().curPattern_->getCurrentBar()) : std::vector<std::vector<bool>>();
            displayGrid(updatedPattern, adikDrum_->cursorPos, numSounds, numSteps);
            // displayMessage(adikDrum_->msgText_); // Non nécessaire si displayMessage est appelé après chaque action.
                                                  // Il est déjà appelé quand on quitte le mode commande aussi.
        }
        // Si l'aide est affichée, le contenu de gridWindow_ et messageWindow_ est déjà géré par toggleHelp()
        // et n'a pas besoin d'être rafraîchi ici.
        // Si on est en mode COMMAND_INPUT, drawCommandInputLine() gère le messageWindow_.
        // gridWindow_ n'a pas besoin d'être rafraîchi non plus pendant la saisie.
    } // Fin de la boucle while
}

/*
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
            case 4: // Ctrl+D (ASCII value for Ctrl+D) - Pour effacer toutes les occurrences d'un son
                adikDrum_->clearCurrentSound();
                break;
            case 8: // Ctrl+H
                adikDrum_->toggleHelp();
                break;
            case 11: // Ctrl+K (ASCII value for Ctrl+K) - Efface toutes les occurrences du DERNIER SON JOUÉ
                adikDrum_->clearLastPlayedSound();
                break;
            case 16: // Ctrl+p (assumant que 16 est le code ASCII pour Ctrl+p)
                adikDrum_->loadPattern();
                break;
            case 18: // Ctrl+R (ASCII value for Ctrl+R)
                adikDrum_->toggleRecord();
                break;
            case 20: // Ctrl+T
                adikDrum_->test();
                break;
            case 21: // Ctrl+U
                adikDrum_->showStatus();
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
            case KEY_DC: // Touche Delete
                adikDrum_->deleteLastPlayedStep();
                break;

            default:
                // Gérer les touches de lecture/enregistrement de sons (Q, S, D, F, pavé numérique, etc.)
                int soundIndex = -1;
                // Vérifier les touches de clavier mappées
                auto it = KEY_TO_SOUND_MAP.find(key);
                if (it != KEY_TO_SOUND_MAP.end()) {
                    soundIndex = it->second;
                } else {
                    // Vérifier les touches du pavé numérique mappées
                    // Note: Ncurses peut retourner des valeurs différentes pour les touches du pavé numérique.
                    // Assurez-vous que KEYPAD_TO_SOUND_MAP est compatible avec les valeurs de 'ch'.
                    auto it_num = KEYPAD_TO_SOUND_MAP.find(key); // 'ch' est déjà un int pour les touches spéciales
                    if (it_num != KEYPAD_TO_SOUND_MAP.end()) {
                        soundIndex = it_num->second;
                    }
                }

                if (soundIndex != -1) {
                    if (adikDrum_->getDrumPlayer().isRecording()) {
                        adikDrum_->recordSound(soundIndex);
                    } else {
                        adikDrum_->playKey(soundIndex); // Joue le son normalement
                    }
                }
                break;

        } // End of switch

        //
        // On rafraîchit la grille et le message normal uniquement si l'aide n'est PAS affichée.
        // if (!adikDrum_->isHelpDisplayed()) {
        //    const auto& updatedPattern = adikDrum_->getCurPattern() ? adikDrum_->getCurPattern()->getPatternBar(adikDrum_->getCurPattern()->getCurrentBar()) : std::vector<std::vector<bool>>();
        //    displayGrid(updatedPattern, adikDrum_->cursorPos, numSounds, numSteps);
        //    displayMessage(adikDrum_->msgText_);
        // } else {
            // Si l'aide est affichée, s'assurer que le message de l'aide reste à l'écran
            // displayMessage(adikDrum_->msgText_); est déjà appelé dans AdikDrum::toggleHelp()
            // lorsque helpDisplayed_ passe à true. Si l'aide est déjà affichée,
            // il n'y a pas besoin de la redessiner à chaque frame.
            // On peut juste s'assurer que le curseur est masqué ou repositionné si nécessaire.
            // Pour l'instant, on se contente de l'affichage sur les 2 lignes du bas.
        // }
        // 


    } // End of While Loop

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

/*
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
*/

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


void AdikTUI::clearCommandInputLine() {
    if (messageWindow_) {
        // Efface la ligne du message, où la commande est affichée
        werase(messageWindow_); // Efface la ligne entière de messageWindow_
        wrefresh(messageWindow_);
    }
}

void AdikTUI::drawCommandInputLine() {
    if (messageWindow_) {
        // Redessine la ligne de commande sur messageWindow_
        werase(messageWindow_); // Efface d'abord tout contenu précédent
        mvwprintw(messageWindow_, 0, 0, ":%s", commandInputBuffer_.c_str());
        wmove(messageWindow_, 0, 1 + commandCursorPos_); // Déplace le curseur Ncurses
        wrefresh(messageWindow_);
    }
}

// --- La fonction executeCommand (à implémenter plus en détail) ---
void AdikTUI::executeCommand(const CommandInput& cmd) {
    if (cmd.commandName.empty()) {
        displayMessage("Commande vide. Annulé.");
        return;
    }

    // Exemple simple: affiche la commande et ses args
    std::string msg = "Exécution commande: " + cmd.commandName;
    for (const auto& arg : cmd.args) {
        msg += " '" + arg + "'";
    }
    displayMessage(msg); // Utilise ta fonction existante displayMessage

    /*
    // TODO: Ici, tu mettras la logique pour mapper cmd.commandName
    // aux fonctions de adikDrum_ et passer les arguments.
    // Utilise un switch ou une map de std::function pour dispatcher les commandes.
    if (cmd.commandName == "bpm" && cmd.args.size() == 1) {
        try {
            float bpm = std::stof(cmd.args[0]);
            adikDrum_->setBpm(bpm);
            displayMessage("BPM réglé à " + std::to_string(bpm));
        } catch (const std::invalid_argument& e) {
            displayMessage("Erreur: Argument BPM invalide. " + std::string(e.what()));
        } catch (const std::out_of_range& e) {
            displayMessage("Erreur: BPM hors de portée. " + std::string(e.what()));
        }
    } else if (cmd.commandName == "quantplayreso" && cmd.args.size() == 1) {
        try {
            int reso = std::stoi(cmd.args[0]);
            adikDrum_->setPlayQuantizeResolution(reso);
            displayMessage("Résolution de quantification réglée à " + std::to_string(reso));
        } catch (const std::invalid_argument& e) {
            displayMessage("Erreur: Argument de résolution invalide. " + std::string(e.what()));
        } catch (const std::out_of_range& e) {
            displayMessage("Erreur: Résolution hors de portée. " + std::string(e.what()));
        }
    }
    // ... ajoute d'autres commandes ici ...
    else {
        displayMessage("Commande inconnue: " + cmd.commandName);
    }
    */

}

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


#ifndef ADIKTUI_H
#define ADIKTUI_H

#include "uiapp.h"
#include "adikdrum.h"
#include "adikcommands.h"

#include <ncurses.h>
#include <string>
#include <vector>
#include <utility>

namespace adikdrum {

enum class UIMode {
    NORMAL,        // Mode normal de navigation et de lecture
    KEY_SOUND, 

    COMMAND_INPUT,  // Mode de saisie de commande (après avoir tapé ':')
    NUM_MODES // Un "compteur" de modes, toujours en dernière position

};

std::string getUIModeName(UIMode mode);

class AdikTUI : public UIApp {
public:
    // Le constructeur prend maintenant un pointeur vers AdikDrum
    AdikTUI(AdikDrum* adikDrum);
    virtual ~AdikTUI();

    virtual bool init() override;
    virtual void run() override;
    virtual void close() override;
    virtual void displayMessage(const std::string& message) override;
    virtual void displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<size_t, size_t> cursor, size_t numSounds, size_t numSteps) override;

private:
    AdikDrum* adikDrum_; // Pointeur vers l'instance de AdikDrum
    int screenWidth_;
    int screenHeight_;
    WINDOW* messageWindow_;
    WINDOW* gridWindow_;
    void createWindows();
    void destroyWindows();


    // fonctions pour le mode command
    UIMode currentUIMode_ = UIMode::NORMAL; // Le mode d'interface actuel
    std::string commandInputBuffer_;      // Le buffer pour la saisie de commande
    size_t commandCursorPos_ = 0;         // Position du curseur dans le buffer de commande

    // Fonctions utilitaires internes pour la gestion de la ligne de commande
    void clearCommandInputLine();
    void drawCommandInputLine();

    // Une fonction pour gérer l'exécution des commandes (à implémenter plus tard)
    void executeCommand(const CommandInput& cmd);
    void handleCommandInput(int key);
    void handleKeySound(int key);

};

} // namespace adikdrum

#endif // ADIKTUI_H

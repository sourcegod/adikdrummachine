#ifndef CONSOLEUIAPP_H
#define CONSOLEUIAPP_H

#include "uiapp.h"
#include "adikdrum.h" // Inclure AdikDrum pour pouvoir l'utiliser
#include <termios.h>

class ConsoleUIApp : public UIApp {
public:
    ConsoleUIApp(AdikDrum& adikDrum);
    ~ConsoleUIApp() override = default;

    termios initTermios(int echo);
    void resetTermios(termios oldt);

    bool init() override;
    void run() override;
    void close() override;
    void displayMessage(const std::string& message) override;
    void displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<size_t, size_t> cursor, size_t numSounds, size_t numSteps) override;

private:
    AdikDrum& adikDrum_;
    termios oldTerm;
};

#endif // CONSOLEUIAPP_H

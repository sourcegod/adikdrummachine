#ifndef ADIKCUIAPP_H
#define ADIKCUIAPP_H

#include "uiapp.h"
#include "adikdrum.h" // Inclure AdikDrum pour pouvoir l'utiliser
#include <termios.h>

namespace adikdrum {

class AdikCUIApp : public UIApp {
public:
    AdikCUIApp(AdikDrum& adikDrum);
    ~AdikCUIApp() override = default;

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
//==== End of class AdikCUIApp ====

} // namespace adikdrum
#endif // ADIKCUIAPP_H

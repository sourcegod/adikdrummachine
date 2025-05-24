#ifndef ADIKTUI_H
#define ADIKTUI_H

#include "uiapp.h"
#include "adikdrum.h"

#include <ncurses.h>
#include <string>
#include <vector>
#include <utility>

namespace adikdrum {

class AdikTUI : public UIApp {
public:

    // Le constructeur prend maintenant un pointeur vers AdikDrum
    AdikTUI(AdikDrum* adikDrum);
    // AdikTUI(AdikDrum& adikDrum); 
    virtual ~AdikTUI();

    virtual bool init() override;
    virtual void run() override;
    virtual void close() override;
    virtual void displayMessage(const std::string& message) override;
    virtual void displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<size_t, size_t> cursor, size_t numSounds, size_t numSteps) override;

private:
    AdikDrum* adikDrum_; // Pointeur vers l'instance de AdikDrum
    // AdikDrum& adikDrum_;
    int screenWidth_;
    int screenHeight_;
    WINDOW* messageWindow_;
    WINDOW* gridWindow_;
    void createWindows();
    void destroyWindows();
};

} // namespace adikdrum

#endif // ADIKTUI_H

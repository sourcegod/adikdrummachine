#ifndef ADIKTUI_H
#define ADIKTUI_H

#include "uiapp.h"
#include <ncurses.h>
#include <string>
#include <vector>
#include <utility>

namespace adikdrum {

class AdikTUI : public UIApp {
public:
    AdikTUI();
    virtual ~AdikTUI();

    virtual bool init() override;
    virtual void run() override;
    virtual void close() override;
    virtual void displayMessage(const std::string& message) override;
    virtual void displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<size_t, size_t> cursor, size_t numSounds, size_t numSteps) override;

private:
    int screenWidth_;
    int screenHeight_;
    WINDOW* messageWindow_;
    WINDOW* gridWindow_;
    void createWindows();
    void destroyWindows();
};

} // namespace adikdrum

#endif // ADIKTUI_H

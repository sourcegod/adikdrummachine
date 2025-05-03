#ifndef UIAPP_H
#define UIAPP_H

#include <string>
#include <vector>
#include <utility>

class UIApp {
public:
    virtual ~UIApp() = default;

    virtual bool init() = 0;
    virtual void run() = 0;
    virtual void close() = 0;
    virtual void displayMessage(const std::string& message) = 0;
    virtual void displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<size_t, size_t> cursor, size_t numSounds, size_t numSteps) = 0;
};
//==== End of class UIApp ====

#endif // UIAPP_H

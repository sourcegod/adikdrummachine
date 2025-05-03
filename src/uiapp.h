#ifndef UIAPP_H
#define UIAPP_H

#include <string>

class UIApp {
public:
    virtual ~UIApp() = default;

    virtual bool init() = 0;
    virtual void run() = 0;
    virtual void close() = 0;
    virtual void displayMessage(const std::string& message) = 0;
    virtual void displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<int, int> cursor) = 0;
};

#endif // UIAPP_H

#ifndef ADIKPATTERN_H
#define ADIKPATTERN_H

#include <iostream>
#include <vector>
#include <string>

// Déclaration de la classe AdikPattern
class AdikPattern {
public:
    // Constructeur par défaut
    AdikPattern();

    // Constructeur avec initialisation du nombre de barres
    AdikPattern(size_t numBarres);

    // Fonction pour définir le nombre de barres
    void setBar(size_t numBarres);

    // Fonction pour obtenir le nombre de barres
    size_t getBar() const;

    // Fonction pour définir la longueur (nombre de pas) d'une barre spécifique
    void setBarLength(size_t barIndex, size_t length);

    // Fonction pour obtenir la longueur (nombre de pas) d'une barre spécifique
    size_t getBarLength(size_t barIndex) const;

    // Fonction pour accéder au pattern 2D d'une barre spécifique (numSounds x numSteps)
    std::vector<std::vector<bool>>& getPatternBar(size_t barIndex);

     // Fonction pour accéder au pattern 2D d'une barre spécifique (version const)
    const std::vector<std::vector<bool>>& getPatternBar(size_t barIndex) const;

    // Fonction pour afficher le pattern de batterie
    void displayPattern() const;

    // Fonction pour obtenir le nombre de sons par barre (fixe)
    size_t getNumSoundsPerBar() const { return numSoundsPerBar_; }
    std::vector<std::vector<std::vector<bool>>>& getPatData() { return patData_; }
    const std::vector<std::vector<std::vector<bool>>>& getPatData() const { return patData_; }
    size_t getCurrentBar() const { return currentBar_; }
    void setCurrentBar(size_t newBarIndex);
    void genData();

private:
    size_t numBarres_; // Nombre de barres dans le pattern
    size_t currentBar_;
    size_t currentStep_;
    size_t numSoundsPerBar_; // Nombre de sons par barre (fixe, par exemple 16)
    std::vector<std::vector<std::vector<bool>>> patData_; // Structure pour stocker le pattern de batterie [barre][son][pas]
};

#endif // ADIKPATTERN_H

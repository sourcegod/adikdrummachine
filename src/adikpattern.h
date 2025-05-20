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

    // Fonction pour définir la longueur d'une barre spécifique
    void setBarLength(size_t barIndex, size_t length);

    // Fonction pour obtenir la longueur d'une barre spécifique
    size_t getBarLength(size_t barIndex) const;

    // Fonction pour accéder au pattern d'une barre spécifique
    std::vector<bool>& getPatternBar(size_t barIndex);

     // Fonction pour accéder au pattern d'une barre spécifique (version const)
    const std::vector<bool>& getPatternBar(size_t barIndex) const;

    // Fonction pour afficher le pattern de batterie
    void displayPattern() const;

private:
    size_t numBarres_; // Nombre de barres dans le pattern
    std::vector<std::vector<bool>> pattern_; // Structure pour stocker le pattern de batterie
};

#endif // ADIKPATTERN_H

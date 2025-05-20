#include "adikpattern.h"

// Constructeur par défaut
AdikPattern::AdikPattern() : numBarres_(0) {}

// Constructeur avec initialisation du nombre de barres
AdikPattern::AdikPattern(size_t numBarres) : numBarres_(numBarres) {
    pattern_.resize(numBarres);
}

// Fonction pour définir le nombre de barres
void AdikPattern::setBar(size_t numBarres) {
    if (numBarres == 0) {
        numBarres = 1; // Correction : nombre minimal de barres est 1
    }
    numBarres_ = numBarres;
    pattern_.resize(numBarres); // Redimensionne le vecteur pattern_
}

// Fonction pour obtenir le nombre de barres
size_t AdikPattern::getBar() const {
    return numBarres_;
}

// Fonction pour définir la longueur d'une barre spécifique
void AdikPattern::setBarLength(size_t barIndex, size_t length) {
    if (barIndex >= numBarres_) {
        barIndex = numBarres_ - 1; // Correction : ajuste l'index au maximum valide
    }
    if (numBarres_ > 0)
    {
        pattern_[barIndex].resize(length, false); // Redimensionne la barre à la longueur spécifiée
    }
}

// Fonction pour obtenir la longueur d'une barre spécifique
size_t AdikPattern::getBarLength(size_t barIndex) const {
    if (barIndex >= numBarres_) {
        barIndex = numBarres_ - 1; // Correction : ajuste l'index au maximum valide
    }
    if (numBarres_ > 0)
    {
       return pattern_[barIndex].size();
    }
    else
    {
        return 0;
    }
}

// Fonction pour accéder au pattern d'une barre spécifique
std::vector<bool>& AdikPattern::getPatternBar(size_t barIndex) {
    if (barIndex >= numBarres_) {
         barIndex = numBarres_ - 1; // Correction : ajuste l'index au maximum valide
    }
    return pattern_[barIndex];
}

// Fonction pour accéder au pattern d'une barre spécifique (version const)
const std::vector<bool>& AdikPattern::getPatternBar(size_t barIndex) const{
    if (barIndex >= numBarres_) {
         barIndex = numBarres_ - 1; // Correction : ajuste l'index au maximum valide
    }
    return pattern_[barIndex];
}

// Fonction pour afficher le pattern de batterie
void AdikPattern::displayPattern() const {
    if (pattern_.empty()) {
        std::cout << "Le pattern de batterie est vide." << std::endl;
        return;
    }

    for (size_t i = 0; i < numBarres_; ++i) {
        std::cout << "Barre " << i + 1 << ": ";
        if (pattern_[i].empty())
        {
            std::cout << "[Barre Vide]" << std::endl;
            continue;
        }
        for (bool note : pattern_[i]) {
            std::cout << (note ? "1 " : "0 ");
        }
        std::cout << std::endl;
    }
}

/*
// Fonction main pour tester la classe AdikPattern
int main() {
    try {
        // Création d'un objet AdikPattern avec 2 barres
        AdikPattern AdikPattern(2);

        // Définition de la longueur des barres
        AdikPattern.setBarLength(0, 8); // La première barre a 8 pas
        AdikPattern.setBarLength(1, 16); // La deuxième barre a 16 pas

        // Accès et modification du pattern de la première barre
        std::vector<bool>& bar1Pattern = AdikPattern.getPatternBar(0);
        bar1Pattern[0] = true; // Place une note au premier pas
        bar1Pattern[3] = true; // Place une note au quatrième pas
        bar1Pattern[7] = true;

        // Accès et modification du pattern de la deuxième barre
        std::vector<bool>& bar2Pattern = AdikPattern.getPatternBar(1);
        bar2Pattern[0] = true;
        bar2Pattern[4] = true;
        bar2Pattern[8] = true;
        bar2Pattern[12] = true;

        // Affichage du pattern
        AdikPattern.displayPattern();

        // Test de la fonction setBar pour modifier le nombre de barres
        AdikPattern.setBar(4); // Change le nombre de barres à 4
        std::cout << "Nombre de barres après modification : " << AdikPattern.getBar() << std::endl;

        // Affichage du pattern après modification du nombre de barres
        AdikPattern.displayPattern(); // Affiche le pattern (les barres 3 et 4 seront vides pour l'instant)

       // Définition de la longueur des nouvelles barres.
        AdikPattern.setBarLength(2, 8);
        AdikPattern.setBarLength(3, 8);

        // Affichage du pattern après avoir défini la longueur des nouvelles barres
        AdikPattern.displayPattern();

    } catch (const std::exception& e) {
        std::cerr << "Erreur: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
*/


#include "adikpattern.h"
#include <algorithm> // Pour std::clamp
#include <random>    // Pour la génération aléatoire

// Constructeur par défaut
AdikPattern::AdikPattern()
    : numBarres_(1), numSoundsPerBar_(16) { // Initialisation par défaut : 1 barre, 16 sons
    currentBar_ =0;
    patData_.resize(numBarres_);
    for (size_t i = 0; i < numBarres_; ++i) {
        patData_[i].resize(numSoundsPerBar_);
        for (size_t j = 0; j < numSoundsPerBar_; ++j) {
            patData_[i][j].assign(16, false); // Par défaut 16 pas par son
        }
    }
}

// Constructeur avec initialisation du nombre de barres
AdikPattern::AdikPattern(size_t numBarres)
    : numBarres_(numBarres), numSoundsPerBar_(16) { // Initialisation par défaut : 16 sons
    if (numBarres_ == 0) numBarres_ = 1; // Assure au moins 1 barre
    currentBar_ =0;
    patData_.resize(numBarres_);
    for (size_t i = 0; i < numBarres_; ++i) {
        patData_[i].resize(numSoundsPerBar_);
        for (size_t j = 0; j < numSoundsPerBar_; ++j) {
            patData_[i][j].assign(16, false); // Par défaut 16 pas par son
        }
    }
}

// Fonction pour définir le nombre de barres
void AdikPattern::setBar(size_t numBarres) {
    if (numBarres == 0) {
        numBarres = 1; // Correction : nombre minimal de barres est 1
    }
    size_t oldNumBarres = numBarres_;
    numBarres_ = numBarres;
    patData_.resize(numBarres_); // Redimensionne le vecteur patData_
    // Initialise les barres nouvellement ajoutées
    for (size_t i = oldNumBarres; i < numBarres_; ++i) {
        patData_[i].resize(numSoundsPerBar_);
        for (size_t j = 0; j < numSoundsPerBar_; ++j) {
            patData_[i][j].assign(16, false); // Par défaut 16 pas pour les nouveaux sons dans les nouvelles barres
        }
    }
}

// Fonction pour obtenir le nombre de barres
size_t AdikPattern::getBar() const {
    return numBarres_;
}

// Fonction pour définir la longueur (nombre de pas) d'une barre spécifique
void AdikPattern::setBarLength(size_t barIndex, size_t length) {
    if (numBarres_ == 0) { // Gère le cas où aucune barre n'existe encore
        setBar(1); // Crée au moins une barre
        barIndex = 0;
    } else if (barIndex >= numBarres_) {
        barIndex = numBarres_ - 1; // Ajuste l'index au maximum valide
    }

    if (patData_[barIndex].empty()) { // S'assure que numSoundsPerBar_ est configuré pour cette barre
        patData_[barIndex].resize(numSoundsPerBar_);
    }

    for (size_t j = 0; j < numSoundsPerBar_; ++j) {
        patData_[barIndex][j].resize(length, false); // Redimensionne chaque son de la barre à la longueur spécifiée
    }
}

// Fonction pour obtenir la longueur (nombre de pas) d'une barre spécifique
size_t AdikPattern::getBarLength(size_t barIndex) const {
    if (numBarres_ == 0) return 0;
    if (barIndex >= numBarres_) {
        barIndex = numBarres_ - 1; // Ajuste l'index au maximum valide
    }
    if (patData_[barIndex].empty() || patData_[barIndex][0].empty()) {
        return 0; // Retourne 0 si la barre ou le premier son est vide
    }
    return patData_[barIndex][0].size(); // Suppose que tous les sons d'une barre ont la même longueur
}

// Fonction pour accéder au pattern 2D d'une barre spécifique
std::vector<std::vector<bool>>& AdikPattern::getPatternBar(size_t barIndex) {
    if (numBarres_ == 0) {
        setBar(1); // Crée au moins une barre si aucune n'existe
        barIndex = 0;
    } else if (barIndex >= numBarres_) {
        barIndex = numBarres_ - 1; // Ajuste l'index au maximum valide
    }
    return patData_[barIndex];
}

// Fonction pour accéder au pattern 2D d'une barre spécifique (version const)
const std::vector<std::vector<bool>>& AdikPattern::getPatternBar(size_t barIndex) const {
    if (numBarres_ == 0) {
        // Dans une fonction const, on ne peut pas modifier l'état (appeler setBar).
        // Il faudrait gérer ce cas en retournant une référence à un pattern vide statique
        // ou en lançant une exception si la conception le permet.
        // Pour l'instant, nous allons simplement ajuster l'index et supposer un accès valide.
        if (barIndex >= numBarres_) {
            barIndex = numBarres_ - 1;
        }
    } else if (barIndex >= numBarres_) {
        barIndex = numBarres_ - 1;
    }
    return patData_[barIndex];
}

// Fonction pour afficher le pattern de batterie
void AdikPattern::displayPattern() const {
    if (patData_.empty()) {
        std::cout << "Le pattern de batterie est vide." << std::endl;
        return;
    }

    for (size_t i = 0; i < numBarres_; ++i) { // Itère sur les barres
        std::cout << "Barre " << i + 1 << ":" << std::endl;
        if (patData_[i].empty()) {
            std::cout << "  [Barre Vide]" << std::endl;
            continue;
        }
        for (size_t j = 0; j < numSoundsPerBar_; ++j) { // Itère sur les sons
            std::cout << "  Son " << j + 1 << ": ";
            if (patData_[i][j].empty()) {
                std::cout << "[Son Vide]" << std::endl;
                continue;
            }
            for (bool note : patData_[i][j]) { // Itère sur les pas
                std::cout << (note ? "1 " : "0 ");
            }
            std::cout << std::endl;
        }
    }
}

void AdikPattern::setCurrentBar(size_t newBarIndex) {
    // Clamp la nouvelle valeur entre 0 et numBarres_ - 1
    currentBar_ = std::clamp(newBarIndex, static_cast<size_t>(0), numBarres_ > 0 ? numBarres_ - 1 : 0);
}

void AdikPattern::genData() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 1);

    // Parcourir les trois dimensions du pattern : [barre][son][pas]
    for (size_t barIdx = 0; barIdx < patData_.size(); ++barIdx) {
        // Assurez-vous que la dimension des sons est correctement redimensionnée
        if (patData_[barIdx].empty()) {
            patData_[barIdx].resize(numSoundsPerBar_);
        }

        for (size_t soundIdx = 0; soundIdx < patData_[barIdx].size(); ++soundIdx) {
            // Assurez-vous que la dimension des pas est correctement redimensionnée
            if (patData_[barIdx][soundIdx].empty()) {
                patData_[barIdx][soundIdx].resize(getBarLength(barIdx), false);
            }

            for (size_t stepIdx = 0; stepIdx < patData_[barIdx][soundIdx].size(); ++stepIdx) {
                patData_[barIdx][soundIdx][stepIdx] = (distrib(gen) == 1);
            }
        }
    }
}


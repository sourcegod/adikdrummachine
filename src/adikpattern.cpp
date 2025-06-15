#include "adikpattern.h"
#include <algorithm> // Pour std::clamp
#include <random>    // Pour la génération aléatoire
namespace adikdrum {

// Constructeur par défaut
AdikPattern::AdikPattern()
    : numBars_(1), numSoundsPerBar_(16) { // Initialisation par défaut : 1 barre, 16 sons
    currentBar_ =0;
    patternData_.resize(numBars_);
    for (size_t i = 0; i < numBars_; ++i) {
        patternData_[i].resize(numSoundsPerBar_);
        for (size_t j = 0; j < numSoundsPerBar_; ++j) {
            patternData_[i][j].assign(16, false); // Par défaut 16 pas par son
        }
    }
    numSteps_ =16;

}
//----------------------------------------

// Constructeur avec initialisation du nombre de barres
AdikPattern::AdikPattern(size_t numBars)
    : numBars_(numBars), numSoundsPerBar_(16) { // Initialisation par défaut : 16 sons
    if (numBars_ == 0) numBars_ = 1; // Assure au moins 1 barre
    currentBar_ =0;
    patternData_.resize(numBars_);
    for (size_t i = 0; i < numBars_; ++i) {
        patternData_[i].resize(numSoundsPerBar_);
        for (size_t j = 0; j < numSoundsPerBar_; ++j) {
            patternData_[i][j].assign(16, false); // Par défaut 16 pas par son
        }
    }
    numSteps_ =16;
}
//----------------------------------------

// Fonction pour définir le nombre de barres
void AdikPattern::setNumBars(size_t numBars) {
    if (numBars == 0) {
        numBars = 1; // Correction : nombre minimal de barres est 1
    }
    size_t oldnumBars = numBars_;
    numBars_ = numBars;
    patternData_.resize(numBars_); // Redimensionne le vecteur patternData_
    // Initialise les barres nouvellement ajoutées
    for (size_t i = oldnumBars; i < numBars_; ++i) {
        patternData_[i].resize(numSoundsPerBar_);
        for (size_t j = 0; j < numSoundsPerBar_; ++j) {
            patternData_[i][j].assign(16, false); // Par défaut 16 pas pour les nouveaux sons dans les nouvelles barres
        }
    }
}
//----------------------------------------

// Fonction pour définir la longueur (nombre de pas) d'une barre spécifique
void AdikPattern::setBarLength(size_t barIndex, size_t length) {
    if (numBars_ == 0) { // Gère le cas où aucune barre n'existe encore
        setNumBars(1); // Crée au moins une barre
        barIndex = 0;
    } else if (barIndex >= numBars_) {
        barIndex = numBars_ - 1; // Ajuste l'index au maximum valide
    }

    if (patternData_[barIndex].empty()) { // S'assure que numSoundsPerBar_ est configuré pour cette barre
        patternData_[barIndex].resize(numSoundsPerBar_);
    }

    for (size_t j = 0; j < numSoundsPerBar_; ++j) {
        patternData_[barIndex][j].resize(length, false); // Redimensionne chaque son de la barre à la longueur spécifiée
    }
}
//----------------------------------------

// Fonction pour obtenir la longueur (nombre de pas) d'une barre spécifique
size_t AdikPattern::getBarLength(size_t barIndex) const {
    if (numBars_ == 0) return 0;
    if (barIndex >= numBars_) {
        barIndex = numBars_ - 1; // Ajuste l'index au maximum valide
    }
    if (patternData_[barIndex].empty() || patternData_[barIndex][0].empty()) {
        return 0; // Retourne 0 si la barre ou le premier son est vide
    }
    return patternData_[barIndex][0].size(); // Suppose que tous les sons d'une barre ont la même longueur
}
//----------------------------------------

// Fonction pour accéder au pattern 2D d'une barre spécifique
std::vector<std::vector<bool>>& AdikPattern::getPatternBar(size_t barIndex) {
    if (numBars_ == 0) {
        setNumBars(1); // Crée au moins une barre si aucune n'existe
        barIndex = 0;
    } else if (barIndex >= numBars_) {
        barIndex = numBars_ - 1; // Ajuste l'index au maximum valide
    }
    return patternData_[barIndex];
}
//----------------------------------------

// Fonction pour accéder au pattern 2D d'une barre spécifique (version const)
const std::vector<std::vector<bool>>& AdikPattern::getPatternBar(size_t barIndex) const {
    if (numBars_ == 0) {
        // Dans une fonction const, on ne peut pas modifier l'état (appeler setBar).
        // Il faudrait gérer ce cas en retournant une référence à un pattern vide statique
        // ou en lançant une exception si la conception le permet.
        // Pour l'instant, nous allons simplement ajuster l'index et supposer un accès valide.
        if (barIndex >= numBars_) {
            barIndex = numBars_ - 1;
        }
    } else if (barIndex >= numBars_) {
        barIndex = numBars_ - 1;
    }
    return patternData_[barIndex];
}
//----------------------------------------

// Fonction pour afficher le pattern de batterie
void AdikPattern::displayPattern() const {
    if (patternData_.empty()) {
        std::cout << "Le pattern de batterie est vide." << std::endl;
        return;
    }

    for (size_t i = 0; i < numBars_; ++i) { // Itère sur les barres
        std::cout << "Barre " << i + 1 << ":" << std::endl;
        if (patternData_[i].empty()) {
            std::cout << "  [Barre Vide]" << std::endl;
            continue;
        }
        for (size_t j = 0; j < numSoundsPerBar_; ++j) { // Itère sur les sons
            std::cout << "  Son " << j + 1 << ": ";
            if (patternData_[i][j].empty()) {
                std::cout << "[Son Vide]" << std::endl;
                continue;
            }
            for (bool note : patternData_[i][j]) { // Itère sur les pas
                std::cout << (note ? "1 " : "0 ");
            }
            std::cout << std::endl;
        }
    }
}
//----------------------------------------

bool AdikPattern::getNote(size_t barIdx, size_t soundIdx, size_t stepIdx) const {
    if (barIdx < patternData_.size() && soundIdx < numSoundsPerBar_ && stepIdx < getBarLength(barIdx)) {
        return patternData_[barIdx][soundIdx][stepIdx];
    }
    // std::cerr << "AVERTISSEMENT: getNote hors limites. Bar: " << barIdx << ", Sound: " << soundIdx << ", Step: " << stepIdx << std::endl;
    return false;
}
//----------------------------------------

// --- NOUVEAU : setNote avec l'ordre bar, sound, step ---
void AdikPattern::setNote(size_t barIdx, size_t soundIdx, size_t stepIdx, bool value) {
    // Ensure the pattern is large enough for the requested bar
    if (barIdx >= patternData_.size()) {
        // Option 1: Expand the pattern (if this behavior is desired)
        // resizeBars(barIdx + 1); // You might need a resizeBars method in AdikPattern
        // For now, we'll just print a warning and return if out of bounds
        std::cerr << "ERREUR: setNote hors limites de barres. Bar: " << barIdx << " (max: " << patternData_.size() << ")" << std::endl;
        return;
    }
    // Ensure the sound vector is large enough for the requested bar
    if (soundIdx >= numSoundsPerBar_) { // numSoundsPerBar_ is fixed to 16 in your AdikPattern
        std::cerr << "ERREUR: setNote hors limites de sons. Sound: " << soundIdx << " (max: " << numSoundsPerBar_ << ")" << std::endl;
        return;
    }
    // Ensure the step vector is large enough for the requested bar and sound
    if (stepIdx >= getBarLength(barIdx)) { // Use getBarLength for the current bar's specific length
        std::cerr << "ERREUR: setNote hors limites de pas. Step: " << stepIdx << " (max: " << getBarLength(barIdx) << ")" << std::endl;
        return;
    }

    patternData_[barIdx][soundIdx][stepIdx] = value;
}
//----------------------------------------

// --- NOUVEAU : clearNote avec l'ordre bar, sound, step ---
void AdikPattern::clearNote(size_t barIdx, size_t soundIdx, size_t stepIdx) {
    setNote(barIdx, soundIdx, stepIdx, false);
}
//----------------------------------------

void AdikPattern::setCurrentBar(size_t newBarIndex) {
    // Clamp la nouvelle valeur entre 0 et numBars_ - 1
    currentBar_ = std::clamp(newBarIndex, static_cast<size_t>(0), numBars_ -1);
}
//----------------------------------------

void AdikPattern::setCurrentStep(size_t newStepIndex) {
    // Clamp la nouvelle valeur entre 0 et la longueur de la barre courante - 1
    currentStep_ = std::clamp(newStepIndex, static_cast<size_t>(0), getBarLength(currentBar_) > 0 ? getBarLength(currentBar_) - 1 : 0);
}
//----------------------------------------

void AdikPattern::setPosition(size_t bar, size_t step) {
    setCurrentBar(bar); // Cela va aussi ajuster currentStep_ si nécessaire
    setCurrentStep(step); // Puis ajuster le pas spécifiquement
}
//----------------------------------------

void AdikPattern::genData() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 1);

    // Parcourir les trois dimensions du pattern : [barre][son][pas]
    for (size_t barIdx = 0; barIdx < patternData_.size(); ++barIdx) {
        // Assurez-vous que la dimension des sons est correctement redimensionnée
        if (patternData_[barIdx].empty()) {
            patternData_[barIdx].resize(numSoundsPerBar_);
        }

        for (size_t soundIdx = 0; soundIdx < patternData_[barIdx].size(); ++soundIdx) {
            // Assurez-vous que la dimension des pas est correctement redimensionnée
            if (patternData_[barIdx][soundIdx].empty()) {
                patternData_[barIdx][soundIdx].resize(getBarLength(barIdx), false);
            }

            for (size_t stepIdx = 0; stepIdx < patternData_[barIdx][soundIdx].size(); ++stepIdx) {
                patternData_[barIdx][soundIdx][stepIdx] = (distrib(gen) == 1);
            }
        }
    }
}
//----------------------------------------

void AdikPattern::saveData() {
    savedData_ = patternData_; // Copie profonde du patternData_ dans savedData_
    std::cout << "Pattern actuel sauvegardé dans savedData_." << std::endl;
}
//----------------------------------------

// Fonction utilitaire pour valider les indices
bool AdikPattern::isValidIndex(size_t barIndex, int soundIndex, size_t stepIndex) const {
    if (barIndex >= numBars_) {
        std::cerr << "Erreur d'index: barIndex (" << barIndex << ") est hors limites (max " << numBars_ - 1 << ")." << std::endl;
        return false;
    }
    if (soundIndex < 0 || static_cast<size_t>(soundIndex) >= numSoundsPerBar_) { // Conversion pour comparaison de taille_t
        std::cerr << "Erreur d'index: soundIndex (" << soundIndex << ") est hors limites (max " << numSoundsPerBar_ - 1 << ")." << std::endl;
        return false;
    }
    if (stepIndex >= numSteps_) {
        std::cerr << "Erreur d'index: stepIndex (" << stepIndex << ") est hors limites (max " << numSteps_ - 1 << ")." << std::endl;
        return false;
    }
    return true;
}
//----------------------------------------

// Implémentation de getSoundStep
bool AdikPattern::getSoundStep(size_t barIndex, int soundIndex, size_t stepIndex) const {
    if (!isValidIndex(barIndex, soundIndex, stepIndex)) {
        return false; // Retourne false si les indices sont invalides
    }
    // Accède directement à la valeur booléenne et la retourne
    return patternData_[barIndex][soundIndex][stepIndex];
}
//----------------------------------------

// Implémentation de toggleSoundStep
bool AdikPattern::toggleSoundStep(size_t barIndex, int soundIndex, size_t stepIndex) {
    if (!isValidIndex(barIndex, soundIndex, stepIndex)) {
        return false; // L'opération a échoué si les indices sont invalides
    }
    // Inverse la valeur booléenne du pas
    patternData_[barIndex][soundIndex][stepIndex] = !patternData_[barIndex][soundIndex][stepIndex];
    return true; // L'opération a réussi
}
//----------------------------------------

void AdikPattern::setSoundSteps(int soundIndex, const std::vector<bool>& newQuantizedSteps) {
    // 1. Validation de l'index du son.
    if (soundIndex < 0 || static_cast<size_t>(soundIndex) >= numSoundsPerBar_) {
        std::cerr << "Erreur dans setSoundSteps: Index de son invalide (" << soundIndex << ").\n";
        return;
    }

    // 2. Validation de la taille du vecteur de pas fourni.
    // Le vecteur 'newQuantizedSteps' doit correspondre à la taille totale du pattern
    // pour ce son (numBars_ * numSteps_ si numSteps_ est le nombre de pas par barre,
    // ou numSteps_ si numSteps_ est la taille totale du pattern comme votre déclaration le suggère).
    // Basé sur votre AdikPattern.h, numSteps_ est la taille totale du pattern.
    if (newQuantizedSteps.size() != numSteps_) {
        std::cerr << "Erreur dans setSoundSteps: Taille de 'newQuantizedSteps' ("
                  << newQuantizedSteps.size() << ") ne correspond pas à la taille totale du pattern ("
                  << numSteps_ << ").\n";
        return;
    }

    // 3. Effacer d'abord tous les pas existants pour ce son dans toutes les barres.
    // C'est important pour éviter les restes de pas non quantifiés qui ne seraient
    // pas réactivés par 'newQuantizedSteps' s'il est plus clairsemé.
    for (size_t barIdx = 0; barIdx < numBars_; ++barIdx) {
        // Redimensionner si la barre n'a pas été dimensionnée correctement
        if (patternData_[barIdx].size() <= static_cast<size_t>(soundIndex)) {
            patternData_[barIdx].resize(soundIndex + 1);
        }
        // Redimensionner le vecteur de pas si nécessaire et l'initialiser à false
        if (patternData_[barIdx][soundIndex].size() != getBarLength(barIdx)) {
             patternData_[barIdx][soundIndex].resize(getBarLength(barIdx), false);
        } else {
             // Si déjà de bonne taille, il suffit de mettre tout à false
             std::fill(patternData_[barIdx][soundIndex].begin(), patternData_[barIdx][soundIndex].end(), false);
        }
    }


    // 4. Remplir le pattern avec les nouveaux pas quantifiés.
    // L'index global du vecteur plat doit être converti en (barIndex, stepInBar).
    size_t globalStepCounter = 0;
    for (size_t barIdx = 0; barIdx < numBars_; ++barIdx) {
        size_t stepsInCurrentBar = getBarLength(barIdx); // Utilise la longueur réelle de la barre

        for (size_t stepInBar = 0; stepInBar < stepsInCurrentBar; ++stepInBar) {
            // S'assure que nous ne dépassons pas la taille du vecteur newQuantizedSteps
            if (globalStepCounter < newQuantizedSteps.size()) {
                // Utilise setNote ou accède directement si c'est plus simple
                // setNote est préférable pour la validation intégrée
                setNote(barIdx, soundIndex, stepInBar, newQuantizedSteps[globalStepCounter]);
            }
            globalStepCounter++;
        }
    }
    // std::cout << "DEBUG: setSoundSteps pour le son " << soundIndex << " terminée.\n";
}
//----------------------------------------

//==== End of class AdikPattern ====

} // namespace adikdrum

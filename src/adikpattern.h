#ifndef ADIKPATTERN_H
#define ADIKPATTERN_H

#include <iostream>
#include <vector>
#include <string>

namespace adikdrum {

class AdikPattern {
public:
    // Constructeur par défaut
    AdikPattern();

    // Constructeur avec initialisation du nombre de barres
    AdikPattern(size_t numBars);

    // Fonction pour obtenir le nombre de barres
    size_t getNumBars() const { return numBars_; }
    // Fonction pour définir le nombre de barres
    void setNumBars(size_t numBars);


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
    std::vector<std::vector<std::vector<bool>>>& getPatternData() { return patternData_; }
    const std::vector<std::vector<std::vector<bool>>>& getPatternData() const { return patternData_; }
    // --- NOUVEAU: getNote avec l'ordre bar, sound, step ---
    bool getNote(size_t barIdx, size_t soundIdx, size_t stepIdx) const;

    // --- NOUVEAU: setNote avec l'ordre bar, sound, step ---
    void setNote(size_t barIdx, size_t soundIdx, size_t stepIdx, bool value);

    // --- NOUVEAU: clearNote avec l'ordre bar, sound, step ---
    void clearNote(size_t barIdx, size_t soundIdx, size_t stepIdx);


    size_t getCurrentBar() const { return currentBar_; }
    void setCurrentBar(size_t newBarIndex);
    size_t getCurrentStep() const { return currentStep_; }
    size_t getNumSteps() const { return numSteps_; }
    void setCurrentStep(size_t newStepIndex);

    void setPosition(size_t bar=0, size_t step=0);
    void genData();

private:
    size_t numBars_; // Nombre de barres dans le pattern
    size_t currentBar_;
    size_t currentStep_ =0;
    size_t numSteps_; 
    size_t numSoundsPerBar_; // Nombre de sons par barre (fixe, par exemple 16)
    std::vector<std::vector<std::vector<bool>>> patternData_; // Structure pour stocker le pattern de batterie [barre][son][pas]
};
//==== End of class AdikPattern ====

} // namespace adikdrum
#endif // ADIKPATTERN_H

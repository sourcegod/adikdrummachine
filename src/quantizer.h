// quantizer.h
#ifndef QUANTIZER_H
#define QUANTIZER_H
#include "adikpattern.h"
#include <vector>
#include <map>
#include <memory> // Pour std::shared_ptr
#include <string>   // Pour std::string
#include <cmath>    // Pour std::round
#include <algorithm> // Pour std::sort, std::unique
#include <chrono> // Pour std::chrono::high_resolution_clock::time_point


// Déclaration forward pour éviter les dépendances circulaires

namespace adikdrum {

// Définition de la map de résolution de quantification
// Sera initialisée dans le constructeur de Quantizer
// Clé: dénomination de la note (1=ronde, 4=noire, 6=triolet noire, etc.)
// Valeur: La proportion de la mesure que représente cette unité.
// Par exemple: 1.0 pour ronde, 0.25 pour noire, 1.0/6.0 pour triolet de noire
using QuantResolutionMap = std::map<int, double>;

class Quantizer {
public:
    // Constructeur : aura besoin d'accéder au pattern et au BPM
    Quantizer(std::shared_ptr<AdikPattern> pattern, double& bpmRef, int numSounds);
    
    // Définir la résolution de quantification pour l'enregistrement
    void setRecQuantizeResolution(size_t resolution);
    // Définir la résolution de quantification pour la lecture/édition
    void setPlayQuantizeResolution(size_t resolution);

    // Retourne la résolution d'enregistrement actuelle
    size_t getRecQuantizeResolution() const { return quantRecReso_; }
    // Retourne la résolution de lecture/édition actuelle
    size_t getPlayQuantizeResolution() const { return quantPlayReso_; }

    // Fonctions de quantification des pas (transférées de DrumPlayer)
    // Retourne le pas quantifié
    size_t quantizeRecordedSteps(size_t currentStep, std::chrono::high_resolution_clock::time_point keyPressTime, std::chrono::high_resolution_clock::time_point lastUpdateTime);
    
    // Génère des pas pour un son donné selon la résolution de lecture/édition
    bool genStepsFromSound(size_t barIndex, size_t soundIndex);
    // Quantifie les pas existants pour un son donné selon la résolution de lecture/édition
    bool quantizeStepsFromSound(size_t soundIndex);
    bool genStepsFromSound(int soundIndex, size_t barIndex);
    // Quantifie l'ensemble du pattern
    void quantizePlayedSteps();

private:
    // Pointeur intelligent vers le pattern (ne gère pas sa durée de vie)
    std::shared_ptr<AdikPattern> curPattern_; 
    double& bpm_; // Référence au BPM actuel de DrumPlayer
    size_t numSounds_; // Nombre de sons (pour validation des index)

    size_t quantRecReso_; // Résolution de quantification pour l'enregistrement (e.g., 16, 4, 6)
    size_t quantPlayReso_; // Résolution de quantification pour la lecture/édition

    QuantResolutionMap quantResolutionMap; // Map des résolutions

    // Fonctions utilitaires (transférées de DrumPlayer, ou nouvelles)
    // Retourne la durée en ms d'une mesure entière (dépend du BPM)
    double getMeasureDurationMs() const;
    // Retourne l'intervalle temporel en ms pour la résolution de quantification donnée
    double getQuantizationIntervalMs(size_t resolution) const;
    // Vérifie si une résolution est de type triolet
    bool isTripletResolution(size_t resolution) const;
    // Trouve le pas de grille entier le plus proche pour une position temporelle donnée
    size_t getClosestGridStep(double targetTimeMs, size_t numStepsInMeasure) const;
    // Trouve le pas de triolet entier le plus proche pour une position temporelle donnée
    size_t getClosestTripletStep(double initialSourceTimeMs, double tripletBaseDurationMs, size_t numStepsInMeasure) const;
    
    // Les membres pour la compensation de latence (devaient déjà être dans DrumPlayer, ou déplacés ici si la compensation est exclusivement pour quantizeRecordedSteps)
    // Pour cet exemple, je les laisse dans DrumPlayer pour la compensation, mais les fonctions qui utilisent sont ici.
    // std::vector<double> recentLatencies_;
    // const size_t maxRecentLatencies_;
    // double calculateAverageLatency() const; // Si déplacé ici

}; 
//==== End of class Quantizer ====

} // namespace adikdrum

#endif // QUANTIZER_H

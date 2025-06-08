/*
 * quantizer.cpp
 *
 * Implémentation de la classe Quantizer pour la quantification musicale.
 * Ce fichier reprend la logique exacte des fonctions de quantification
 * de la classe DrumPlayer fournies par l'utilisateur, en l'adaptant
 * aux signatures et membres de la classe Quantizer déclarés dans quantizer.h.
 */

#include "quantizer.h"
#include "adikpattern.h"
#include <iostream> // Pour les messages DEBUG

namespace adikdrum {

// Constructeur de Quantizer
Quantizer::Quantizer(std::shared_ptr<AdikPattern> pattern, double& bpmRef, int numSounds)
    : curPattern_(pattern), bpm_(bpmRef), numSounds_(numSounds),
      quantRecReso_(16), quantPlayReso_(16) // Initialisation par défaut à 16ème note
{
    // Initialisation de la map des résolutions de quantification.
    // Selon le quantizer.h, la valeur est une proportion de la mesure (double).
    // Ces proportions seront multipliées par la longueur du pattern (ex: 16 pas)
    // pour obtenir le nombre de "pas unitaires" pour la quantification.
    quantResolutionMap[1] = 1.0;        // Ronde (mesure entière)
    quantResolutionMap[2] = 0.5;        // Blanche (1/2 mesure)
    quantResolutionMap[3] = 1.0 / 3.0;  // Triplet division (ex: 3ème de mesure)
    quantResolutionMap[4] = 0.25;       // Noire (1/4 mesure)
    quantResolutionMap[6] = 1.0 / 6.0;  // Triolet de noire (si 6 subdivisions par mesure)
    quantResolutionMap[8] = 0.125;      // Croche (1/8 mesure)
    quantResolutionMap[12] = 1.0 / 12.0; // Triolet de croche (1/12 de mesure)
    quantResolutionMap[16] = 0.0625;    // Double croche (1/16 mesure)
    quantResolutionMap[24] = 1.0 / 24.0; // Triolet de double croche
    quantResolutionMap[32] = 1.0 / 32.0; // Triple croche (32ème)
    quantResolutionMap[48] = 1.0 / 48.0; // Triolet de triple croche
    quantResolutionMap[64] = 1.0 / 64.0; // Quadruple croche (64ème)

    // NOTE IMPORTANTE: La logique de quantification fournie par l'utilisateur (issue de DrumPlayer)
    // utilise une approche de grille simple basée sur des `size_t quantUnitSteps` entiers.
    // Pour les résolutions comme 3, 6, 12, 24, 48, 64 qui ne divisent pas la longueur du pattern
    // (ex: 16 pas) en un nombre entier de "pas unitaires", le comportement sera une approximation
    // due à l'arrondi et à la division entière. Le code DrumPlayer fourni n'implémente pas
    // un traitement spécifique pour les triolets au-delà de cette approche simple.
}
//----------------------------------------


// Définit la résolution de quantification pour l'enregistrement.
// Valide la résolution en vérifiant sa présence dans la map.
void Quantizer::setRecQuantizeResolution(size_t resolution) {
    if (quantResolutionMap.count(resolution)) {
        quantRecReso_ = resolution;
    } else {
        std::cerr << "Erreur: Résolution d'enregistrement " << resolution << " non reconnue." << std::endl;
    }
}
//----------------------------------------

// Définit la résolution de quantification pour la lecture/édition.
// Valide la résolution en vérifiant sa présence dans la map.
void Quantizer::setPlayQuantizeResolution(size_t resolution) {
    if (quantResolutionMap.count(resolution)) {
        quantPlayReso_ = resolution;
    } else {
        std::cerr << "Erreur: Résolution de lecture/édition " << resolution << " non reconnue." << std::endl;
    }
}
//----------------------------------------

// Retourne la durée en millisecondes d'une mesure entière.
// Dépend du BPM (Battements Par Minute). Une mesure est considérée comme 4 noires.
double Quantizer::getMeasureDurationMs() const {
    // Calcul: (60 secondes / BPM) * 1000 ms/seconde * 4 temps/mesure (pour une mesure 4/4 typique)
    // Le BPM est la vitesse en noires par minute.
    if (bpm_ <= 0) {
        // Gérer le cas de BPM invalide pour éviter une division par zéro ou des calculs erronés.
        // Un BPM très faible ou nul peut indiquer une pause ou un problème.
        std::cerr << "Avertissement: BPM est nul ou négatif. Retourne une durée de mesure de 0ms." << std::endl;
        return 0.0;
    }
    return (60.0 / bpm_) * 1000.0 * 4.0;
}
//----------------------------------------

// Retourne l'intervalle temporel en millisecondes pour une résolution de quantification donnée.
// Utilise la map des résolutions pour trouver la proportion de mesure et la convertir en ms.
double Quantizer::getQuantizationIntervalMs(size_t resolution) const {
    auto it = quantResolutionMap.find(resolution);
    if (it != quantResolutionMap.end()) {
        return getMeasureDurationMs() * it->second;
    }
    // Si la résolution n'est pas trouvée, un message d'erreur est affiché.
    std::cerr << "Erreur: Résolution de quantification " << resolution << " non trouvée dans la map. Retourne 0." << std::endl;
    return 0.0;
}
//----------------------------------------

// Vérifie si une résolution donnée est de type triolet.
// Les résolutions comme 3 (tierce de mesure), 6 (triolet de noire), 12 (triolet de croche), etc.,
// sont considérées comme des triolets.
bool Quantizer::isTripletResolution(size_t resolution) const {
    return resolution == 3 || resolution == 6 || resolution == 12 ||
           resolution == 24 || resolution == 48; // Liste des résolutions de triolets communes
}
//----------------------------------------

// Trouve le pas de grille entier le plus proche pour une position temporelle donnée.
// `numStepsInMeasure` représente le nombre total de pas dans une mesure (e.g., 16 pour 16èmes de note).
size_t Quantizer::getClosestGridStep(double targetTimeMs, size_t numStepsInMeasure) const {
    if (numStepsInMeasure == 0) {
        std::cerr << "Avertissement: Le nombre de pas dans la mesure est zéro pour getClosestGridStep. Retourne 0." << std::endl;
        return 0;
    }

    double measureDurationMs = getMeasureDurationMs();
    if (measureDurationMs <= 0) {
        std::cerr << "Avertissement: Durée de la mesure est nulle ou négative dans getClosestGridStep. Retourne 0." << std::endl;
        return 0;
    }

    // Calcul de la durée d'un seul pas de grille.
    double stepDurationMs = measureDurationMs / numStepsInMeasure;
    if (stepDurationMs == 0) {
         std::cerr << "Avertissement: Durée du pas de grille est nulle dans getClosestGridStep. Retourne 0." << std::endl;
         return 0;
    }

    // Calcul de la position fractionnaire sur la grille de pas.
    double targetStepFractional = targetTimeMs / stepDurationMs;

    // Arrondir au pas entier le plus proche.
    size_t closestStep = static_cast<size_t>(std::round(targetStepFractional));

    // S'assurer que le pas quantifié reste dans les limites du pattern (0 à numStepsInMeasure-1).
    // Si le temps est exactement à la fin de la mesure (ex: 16.0 pour un pattern de 16 pas),
    // il devrait revenir au pas 0 si on est cyclique. Ici, on le garde au dernier pas.
    if (closestStep >= numStepsInMeasure) {
        closestStep = numStepsInMeasure - 1;
    }

    return closestStep;
}
//----------------------------------------

// Trouve le pas de triolet entier le plus proche pour une position temporelle donnée.
// Cette fonction est plus spécifique et aligne le temps sur une subdivision de triolet,
// en considérant un 'tripletBaseDurationMs' comme la durée de la note de base du triolet.
size_t Quantizer::getClosestTripletStep(double initialSourceTimeMs, double tripletBaseDurationMs, size_t numStepsInMeasure) const {
    if (tripletBaseDurationMs <= 0 || numStepsInMeasure == 0) {
        std::cerr << "Avertissement: Paramètres invalides pour getClosestTripletStep. Retourne 0." << std::endl;
        return 0;
    }

    // Durée d'une seule note au sein du triolet (ex: pour un triolet de croches, c'est 1/3 d'une noire)
    double singleTripletNoteDurationMs = tripletBaseDurationMs / 3.0;
    if (singleTripletNoteDurationMs == 0) {
        std::cerr << "Avertissement: Durée d'une note de triolet est nulle. Retourne 0." << std::endl;
        return 0;
    }

    // Trouver le point de départ du groupe de triolets auquel initialSourceTimeMs appartient.
    // Cela se fait en trouvant le multiple le plus proche de tripletBaseDurationMs.
    double tripletGroupStartMs = std::round(initialSourceTimeMs / tripletBaseDurationMs) * tripletBaseDurationMs;

    // Calculer la position relative de initialSourceTimeMs par rapport au début de ce groupe de triolets.
    double relativeTimeInTripletGroupMs = initialSourceTimeMs - tripletGroupStartMs;

    // Quantifier cette position relative à la note de triolet la plus proche.
    double closestRelativeTripletNoteTimeMs = std::round(relativeTimeInTripletGroupMs / singleTripletNoteDurationMs) * singleTripletNoteDurationMs;

    // Reconstruire le temps absolu quantifié en ajoutant le temps relatif quantifié au début du groupe.
    double quantizedTimeMs = tripletGroupStartMs + closestRelativeTripletNoteTimeMs;

    // Utiliser getClosestGridStep pour convertir ce temps quantifié en un index de pas.
    // Assurez-vous que le temps quantifié est positif et dans la mesure.
    double measureDurationMs = getMeasureDurationMs();
    quantizedTimeMs = fmod(quantizedTimeMs + measureDurationMs, measureDurationMs);
    if (quantizedTimeMs < 0) {
        quantizedTimeMs += measureDurationMs;
    }

    return getClosestGridStep(quantizedTimeMs, numStepsInMeasure);
}
//----------------------------------------


// Quantifie les pas enregistrés par l'utilisateur (événements en temps réel).
// Retourne le pas (index) quantifié le plus proche.
size_t Quantizer::quantizeRecordedSteps(size_t currentStep, std::chrono::high_resolution_clock::time_point keyPressTime, std::chrono::high_resolution_clock::time_point lastUpdateTime) {
    if (!curPattern_) {
        std::cerr << "Erreur: Pattern non valide dans quantizeRecordedSteps. Retourne le pas actuel." << std::endl;
        return currentStep;
    }

    size_t numStepsInPattern = curPattern_->getNumSteps();
    if (numStepsInPattern == 0) {
        std::cerr << "Erreur: Le nombre de pas du pattern est zéro. Retourne le pas actuel." << std::endl;
        return currentStep;
    }

    double totalPatternDurationMs = getMeasureDurationMs();
    if (totalPatternDurationMs <= 0) {
        std::cerr << "Erreur: Durée totale du pattern est nulle ou négative. Retourne le pas actuel." << std::endl;
        return currentStep;
    }

    // Durée d'un seul pas logique du pattern (ex: un 16ème de note si 16 pas/mesure).
    double stepDurationMs = totalPatternDurationMs / numStepsInPattern;

    // Calculer le temps écoulé entre la dernière mise à jour du sequencer et la frappe de la touche.
    double timeElapsedSinceLastUpdateMs = std::chrono::duration_cast<std::chrono::milliseconds>(keyPressTime - lastUpdateTime).count();

    // Calculer le temps "exact" de la frappe par rapport au début de la mesure/pattern.
    // On suppose que lastUpdateTime correspond au début du 'currentStep'.
    double exactTimeInPatternMs = (currentStep * stepDurationMs) + timeElapsedSinceLastUpdateMs;

    // S'assurer que le temps exact est cyclique dans la mesure (0 à totalPatternDurationMs - epsilon).
    exactTimeInPatternMs = std::fmod(exactTimeInPatternMs, totalPatternDurationMs);
    if (exactTimeInPatternMs < 0) { // fmod peut retourner un résultat négatif si le numérateur est négatif
        exactTimeInPatternMs += totalPatternDurationMs;
    }

    // Obtenir l'intervalle de quantification basé sur la résolution d'enregistrement (`quantRecReso_`).
    double quantIntervalMs = getQuantizationIntervalMs(quantRecReso_);
    if (quantIntervalMs == 0) {
        std::cerr << "Erreur: Intervalle de quantification est nul pour l'enregistrement. Retourne le pas actuel." << std::endl;
        return currentStep;
    }

    // Quantifier le temps de la frappe à l'intervalle temporel le plus proche.
    double quantizedTimeInPatternMs = std::round(exactTimeInPatternMs / quantIntervalMs) * quantIntervalMs;

    // S'assurer que le temps quantifié est cyclique et dans les limites de la mesure.
    quantizedTimeInPatternMs = std::fmod(quantizedTimeInPatternMs, totalPatternDurationMs);
    if (quantizedTimeInPatternMs < 0) {
        quantizedTimeInPatternMs += totalPatternDurationMs;
    }

    // Convertir le temps quantifié en un index de pas sur la grille du pattern.
    size_t quantizedStep = getClosestGridStep(quantizedTimeInPatternMs, numStepsInPattern);

    return quantizedStep;
}
//----------------------------------------

bool Quantizer::genStepsFromSound(size_t barIndex, size_t soundIndex) {
    // 1. Validation des paramètres et du pattern
    if (!curPattern_) {
        std::cerr << "Erreur: Le pattern n'est pas valide dans Quantizer::genStepsFromSound. Aucune modification." << std::endl;
        return false;
    }
    if (static_cast<size_t>(soundIndex) >= numSounds_) {
        std::cerr << "Erreur: Index de son invalide (" << soundIndex << ") dans Quantizer::genStepsFromSound. Aucune modification." << std::endl;
        return false;
    }
    if (barIndex >= curPattern_->getNumBars()) {
        std::cerr << "Erreur: Indice de mesure invalide (" << barIndex << ") dans Quantizer::genStepsFromSound. Aucune modification." << std::endl;
        return false;
    }

    size_t totalStepsInBar = curPattern_->getNumSteps(); // Nombre total de pas par mesure
    if (totalStepsInBar == 0) {
        std::cerr << "Avertissement: Le nombre de pas par mesure est zéro. Rien à générer pour le son " << soundIndex << " dans la mesure " << barIndex << "." << std::endl;
        return false;
    }

    // 2. Détermination de l'intervalle de pas de quantification (équivalent à `quantUnitSteps`)
    double resolutionProportion = 0.0;
    auto it = quantResolutionMap.find(quantPlayReso_);
    if (it != quantResolutionMap.end()) {
        resolutionProportion = it->second;
    } else {
        // Fallback si la résolution n'est pas reconnue (devrait être géré par setPlayQuantizeResolution)
        std::cerr << "Avertissement: Résolution de lecture/édition " << quantPlayReso_ << " non reconnue pour la génération. Utilisation de la résolution par défaut (1/16)." << std::endl;
        resolutionProportion = quantResolutionMap[16]; // 16ème de note par défaut
    }

    // Calcul de l'intervalle en nombre de pas sur la grille du pattern.
    // Par exemple, si 16 pas par mesure et résolution 1/4 (noire), l'intervalle est 16 * 0.25 = 4 pas.
    size_t quantUnitSteps = static_cast<size_t>(std::round(totalStepsInBar * resolutionProportion));

    // Assurer que `quantUnitSteps` est au moins 1 pour éviter une boucle infinie ou des problèmes
    if (quantUnitSteps == 0) {
         std::cerr << "Avertissement: L'unité de quantification calculée est zéro. Utilisation d'une unité de 1 pas pour la génération afin d'éviter une boucle infinie." << std::endl;
         quantUnitSteps = 1; // Un minimum d'un pas pour la génération
    }

    bool changed = false;

    // 3. Effacer tous les pas existants pour ce son dans cette mesure
    // On itère sur tous les pas et on les désactive s'ils sont actifs.
    // AdikPattern::toggleSoundStep est utilisé car il gère l'état courant et modifie directement le pattern.
    for (size_t stepIdx = 0; stepIdx < totalStepsInBar; ++stepIdx) {
        if (curPattern_->getSoundStep(barIndex, soundIndex, stepIdx)) {
            curPattern_->toggleSoundStep(barIndex, soundIndex, stepIdx); // Désactive le pas
            changed = true;
        }
    }

    // 4. Générer (activer) les nouveaux pas à l'intervalle de quantification
    // On itère en incrémentant par `quantUnitSteps` pour placer les notes.
    for (size_t stepIdx = 0; stepIdx < totalStepsInBar; stepIdx += quantUnitSteps) {
        // Puisqu'on vient de tout effacer, on sait que le pas sera inactif.
        // Un simple appel à toggleSoundStep l'activera.
        // On vérifie quand même pour la robustesse, mais ce if pourrait être omis après un effacement total.
        if (!curPattern_->getSoundStep(barIndex, soundIndex, stepIdx)) { // Si le pas est inactif, l'activer
            curPattern_->toggleSoundStep(barIndex, soundIndex, stepIdx);
            changed = true;
        }
    }

    std::cout << "Génération de pas pour le son " << soundIndex << " dans la mesure " << barIndex
              << " terminée avec résolution " << quantPlayReso_ << "." << std::endl;
    return changed;
}
//----------------------------------------

// Quantifie les pas existants pour un son donné selon la résolution de lecture/édition.
// C'est utile pour "nettoyer" ou modifier un pattern déjà créé.
bool Quantizer::quantizeStepsFromSound(size_t soundIndex) {
    // Vérifications initiales des paramètres
    if (!curPattern_ || static_cast<size_t>(soundIndex) >= numSounds_) {
        std::cerr << "Erreur: Index de son ou pattern non valide dans quantizeStepsFromSound." << std::endl;
        return false;
    }

    size_t numStepsInPattern = curPattern_->getNumSteps();
    if (numStepsInPattern == 0) {
        std::cerr << "Erreur: Nombre de pas du pattern est zéro pour le son " << soundIndex << ". Rien à quantifier." << std::endl;
        return false;
    }

    double totalPatternDurationMs = getMeasureDurationMs();
    if (totalPatternDurationMs <= 0) {
        std::cerr << "Erreur: Durée totale du pattern est nulle ou négative. Rien à quantifier." << std::endl;
        return false;
    }

    // Durée d'un pas logique dans le pattern (global).
    double stepDurationMs = totalPatternDurationMs / numStepsInPattern;

    // Récupérer le nombre de mesures et de pas par mesure depuis le pattern.
    // C'est essentiel pour pouvoir parcourir la structure en 3D.
    int numBars = curPattern_->getNumBars();
    int stepsPerBar = curPattern_->getNumSteps();

    // Vérification de cohérence (recommandé)
    if (numStepsInPattern != static_cast<size_t>(numBars * stepsPerBar)) {
        std::cerr << "Avertissement: Le nombre total de pas rapporté par getNumSteps() ne correspond pas au produit de getNumBars() * getStepsPerBar(). Veuillez vérifier la logique de AdikPattern." << std::endl;
        // Pour cet exemple, nous continuons en nous basant sur numStepsInPattern
        // pour la taille des vecteurs originalSteps et newQuantizedSteps.
    }

    // Récupérer tous les pas originaux du son en itérant sur chaque mesure et chaque pas dans la mesure.
    // C'est ici que le 'barIndex' est introduit et utilisé.
    std::vector<bool> originalSteps(numStepsInPattern);
    for (int barIndex = 0; barIndex < numBars; ++barIndex) { // Boucle sur chaque mesure
        for (int stepInBar = 0; stepInBar < stepsPerBar; ++stepInBar) { // Boucle sur chaque pas dans la mesure
            // Calcul de l'index global du pas dans le vecteur plat
            size_t globalStepIndex = static_cast<size_t>(barIndex * stepsPerBar + stepInBar);
            if (globalStepIndex < numStepsInPattern) {
                // Utilisation de AdikPattern::getSoundStep avec le barIndex généré
                originalSteps[globalStepIndex] = curPattern_->getSoundStep(barIndex, soundIndex, stepInBar);
            } else {
                // Cette erreur ne devrait pas se produire si les calculs sont corrects
                std::cerr << "Erreur interne: globalStepIndex dépasse numStepsInPattern lors de la lecture." << std::endl;
            }
        }
    }

    // Créer un nouveau vecteur pour les pas quantifiés, tous à 'false' initialement.
    std::vector<bool> newQuantizedSteps(numStepsInPattern, false);

    // Obtenir l'intervalle de quantification pour la lecture/édition.
    double quantIntervalMs = getQuantizationIntervalMs(quantPlayReso_);
    if (quantIntervalMs == 0) {
        std::cerr << "Erreur: Intervalle de quantification est nul pour la lecture/édition. Son " << soundIndex << " non quantifié." << std::endl;
        return false;
    }

    // Traiter chaque pas actif du pattern original.
    for (size_t step = 0; step < numStepsInPattern; ++step) {
        if (originalSteps[step]) { // Si le pas global est actif
            // Calculer son temps exact dans le pattern.
            double originalTimeMs = step * stepDurationMs;

            // Quantifier ce temps à l'intervalle le plus proche.
            double quantizedTimeMs = std::round(originalTimeMs / quantIntervalMs) * quantIntervalMs;

            // S'assurer que le temps quantifié est cyclique et dans les limites du pattern.
            quantizedTimeMs = std::fmod(quantizedTimeMs, totalPatternDurationMs);
            if (quantizedTimeMs < 0) {
                quantizedTimeMs += totalPatternDurationMs;
            }

            // Convertir le temps quantifié en un index de pas global et le marquer comme actif.
            size_t quantizedStepIndex = getClosestGridStep(quantizedTimeMs, numStepsInPattern);
            newQuantizedSteps[quantizedStepIndex] = true;
        }
    }

    // Mettre à jour le pattern avec les nouveaux pas quantifiés.
    // C'est AdikPattern::setSoundSteps qui gérera la redistribution dans la structure interne.
    curPattern_->setSoundSteps(soundIndex, newQuantizedSteps);
    return true;
}
//----------------------------------------

// Quantifie l'ensemble du pattern en itérant sur tous les sons.
void Quantizer::quantizePlayedSteps() {
    std::cout << "Début de la quantification de tous les pas joués..." << std::endl;
    auto numSounds = curPattern_->getNumSoundsPerBar();
    for (size_t i = 0; i < numSounds; ++i) {
        // Appelle quantizeStepsFromSound pour chaque son.
        if (!quantizeStepsFromSound(i)) {
            std::cerr << "Avertissement: La quantification pour le son " << i << " a échoué." << std::endl;
        }
    }
    std::cout << "Quantification de tous les pas joués terminée." << std::endl;
}
//----------------------------------------


//==== End of class Quantizer ====

} // namespace adikdrum

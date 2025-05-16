#ifndef SIMPLE_DELAY_H
#define SIMPLE_DELAY_H

#include <vector>
#include <iostream>
#include <algorithm>

namespace adikdrum {

class SimpleDelay {
public:
    /**
     * @brief Constructeur de la classe SimpleDelay.
     * @param maxDelaySamples La taille maximale du buffer de délai en échantillons.
     * Une valeur plus grande permet des délais plus longs.
     * @param sampleRate La fréquence d'échantillonnage du signal audio.
     */
    SimpleDelay(size_t maxDelaySamples, int sampleRate)
        : maxDelaySamples_(maxDelaySamples),
          sampleRate_(sampleRate),
          delayBuffer_(maxDelaySamples, 0.0f), // Initialise le buffer avec des zéros
          delayIndex_(0),
          feedback_(0.0f),
          delayTime_(0.0f),
          isEnabled_(false) {
        // Validation de la taille du buffer
        if (maxDelaySamples_ == 0) {
            std::cerr << "SimpleDelay : maxDelaySamples doit être supérieur à zéro. Utilisation de la valeur par défaut : 1000." << std::endl;
            maxDelaySamples_ = 1000;
        }
        if (sampleRate_ <= 0)
        {
             std::cerr << "SimpleDelay : sampleRate doit être supérieur à zéro. Utilisation de la valeur par défaut : 44100." << std::endl;
             sampleRate_ = 44100;
        }

        delayBuffer_.resize(maxDelaySamples_);
        std::fill(delayBuffer_.begin(), delayBuffer_.end(), 0.0f); // Assure que le buffer est initialisé à zéro.
    }

    /**
     * @brief Destructeur de la classe SimpleDelay.
     * Bien que le destructeur par défaut soit suffisant ici, il est inclus pour la clarté.
     */
    virtual ~SimpleDelay() {}

    /**
     * @brief Applique l'effet de délai à un échantillon audio.
     * @param input L'échantillon audio d'entrée.
     * @return L'échantillon audio modifié avec l'effet de délai.
     */
    float process(float input) {
        if (!isEnabled_) {
            return input; // Si le délai est désactivé, retourne l'entrée sans modification
        }

        // Calcule l'indice de lecture dans le buffer de délai
        size_t readIndex = (delayIndex_ + maxDelaySamples_ - static_cast<size_t>(delaySamples_)) % maxDelaySamples_;
        float delayedSample = delayBuffer_[readIndex];

        // Écrit l'échantillon d'entrée actuel, plus le feedback du signal retardé, dans le buffer
        delayBuffer_[delayIndex_] = input + delayedSample * feedback_;

        // Incrémente l'index d'écriture, en bouclant si nécessaire
        delayIndex_ = (delayIndex_ + 1) % maxDelaySamples_;

        return input + delayedSample * gain_; // Retourne la somme de l'entrée et du signal retardé
    }

    /**
     * @brief Définit le temps de délai en millisecondes.
     * @param delayTimeMs Le temps de délai en millisecondes.
     */
    void setDelayTime(float delayTimeMs) {
        if (delayTimeMs < 0) {
            delayTimeMs = 0;
            std::cerr << "SimpleDelay : delayTimeMs doit être positif. Utilisation de la valeur minimale : 0." << std::endl;
        }
        delayTime_ = delayTimeMs;
        // Conversion du temps de délai en millisecondes en nombre d'échantillons
        delaySamples_ = static_cast<size_t>(delayTime_ * sampleRate_ / 1000.0f);
        // Assure que le temps de délai ne dépasse pas la taille du buffer
        if (delaySamples_ >= maxDelaySamples_) {
            delaySamples_ = maxDelaySamples_ - 1;
            std::cerr << "SimpleDelay : delayTimeMs est trop long. Utilisation de la valeur maximale possible." << std::endl;
        }
    }

    /**
     * @brief Définit le facteur de feedback.
     * @param feedback Le facteur de feedback (0.0 à 1.0).
     */
    void setFeedback(float feedback) {
        if (feedback < 0.0f) {
            feedback = 0.0f;
            std::cerr << "SimpleDelay : feedback doit être positif. Utilisation de la valeur minimale : 0." << std::endl;
        }
        if (feedback > 1.0f) {
            feedback = 1.0f;
             std::cerr << "SimpleDelay : feedback doit être inférieur à 1. Utilisation de la valeur maximale : 1." << std::endl;
        }
        feedback_ = feedback;
    }

    /**
     * @brief Définit le gain du signal retardé.
     * @param gain Le gain appliqué au signal retardé.
     */
    void setGain(float gain) {
        if (gain < 0.0f) {
            gain = 0.0f;
            std::cerr << "SimpleDelay : gain doit être positif. Utilisation de la valeur minimale : 0." << std::endl;
        }
        if (gain > 1.0f) {
            gain = 1.0f;
            std::cerr << "SimpleDelay : gain doit être inférieur à 1. Utilisation de la valeur maximale : 1." << std::endl;
        }
        gain_ = gain;
    }

    /**
     * @brief Active ou désactive l'effet de délai.
     * @param isEnabled Un booléen indiquant si le délai doit être activé (true) ou désactivé (false).
     */
    void setIsEnabled(bool isEnabled) {
        isEnabled_ = isEnabled;
    }

    /**
     * @brief Réinitialise l'état du délai (buffer et index).
     * Utile pour éviter d'anciens artefacts sonores lors de la réactivation du délai.
     */
    void reset() {
        std::fill(delayBuffer_.begin(), delayBuffer_.end(), 0.0f);
        delayIndex_ = 0;
    }

    /**
     * @brief Obtient le temps de délai actuel.
     * @return Le temps de délai en millisecondes.
     */
    float getDelayTime() const { return delayTime_; }

    /**
      * @brief Obtient le facteur de feedback actuel.
      * @return Le facteur de feedback.
      */
    float getFeedback() const { return feedback_; }

     /**
      * @brief Obtient le gain actuel.
      * @return Le gain.
      */
    float getGain() const { return gain_; }

    /**
     * @brief Obtient l'état d'activation du délai.
     * @return true si le délai est activé, false sinon.
     */
    bool getIsEnabled() const { return isEnabled_; }

private:
    size_t maxDelaySamples_;  // Taille maximale du buffer de délai
    size_t delaySamples_;    // Délai en nombre d'échantillons
    int sampleRate_;
    std::vector<float> delayBuffer_; // Buffer pour stocker les échantillons retardés
    size_t delayIndex_;       // Index pour écrire dans le buffer de délai
    float feedback_;         // Facteur de feedback
    float delayTime_;          // Temps de délai en millisecondes
    float gain_;             // Gain du signal retardé
    bool isEnabled_;         // Indique si le délai est activé
};

} // namespace adikdrum

#endif // SIMPLE_DELAY_H


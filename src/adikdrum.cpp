/*
 *  File: adikdrum.cpp
 *  Drum Machine Engine in C++
 *  Compile: g++ adikdrum.cpp -o adikdrum -lportaudio -lpthread
 *  Date: Sat, 12/04/2025
 *  Author: Coolbrother
 *  */
//----------------------------------------

#include "adikdrum.h"

#include "adikcuiapp.h" // Inclure l'en-tête de ConsoleUIApp
#include "audiodriver.h" // Inclure le header de AudioDriver
#include "drumplayer.h"
#include "audiomixer.h"
#include "constants.h"

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <map>
#include <utility> // Pour utiliser std::pair
#include <random>

// for performance checking
#include <thread>
#include <chrono>

//----------------------------------------


volatile int callbackCounter =0;

void beep() {
    std::cout << '\a' << std::flush;
}
//----------------------------------------

namespace adikdrum {

AdikDrum::AdikDrum(UIApp* uiApp)
    : uiApp_(uiApp),
      cursorPos({0, 0}),
      sampleRate_(44100),
      mixer_(32),
      numSounds_(16),
      numSteps_(16),
      drumPlayer_(numSounds_, numSteps_),
      msgText_(""), // Initialisation optionnelle
      previousMsgText_("")

{
      std::cout << "AdikDrum::Constructor - numSounds_: " << numSounds_ << ", numSteps_: " << numSteps_ << std::endl;
    helpText_ = "Aide pour Adik Drum Machine\n"
                "Appuyez sur les touches suivantes pour contrôler l'application:\n"
                "  Q: Quitter l'application\n"
                "  F1: Afficher/masquer cette aide\n" // Nouveau raccourci
                "  Entrée: Activer le pas à la position du curseur\n"
                "  Backspace: Désactiver le pas à la position du curseur\n"
                "  Espace / 0: Activer/désactiver la lecture\n"
                "  c: Démarrer/arrêter le métronome\n"
                "  p: Lancer la démo des sons\n"
                "  Ctrl+P: Charger un nouveau pattern aléatoire\n" // Correction: Ctrl+P pour charger
                "  Ctrl+R: Activer/désactiver l'enregistrement\n" // Nouveau raccourci
                "  v / .: Arrêter tous les sons en cours\n"
                "  x: Muter/démuter le son courant (ligne du curseur)\n"
                "  X: Réinitialiser le mute de tous les canaux\n"
                "  +: Augmenter le volume global\n"
                "  -: Diminuer le volume global\n"
                "  (: Diminuer le BPM\n"
                "  ): Augmenter le BPM\n"
                "  [: Déplacer le panoramique du son courant vers la gauche\n"
                "  ]: Déplacer le panoramique du son courant vers la droite\n"
                "  {: Diminuer la vitesse de lecture du son courant\n"
                "  }: Augmenter la vitesse de lecture du son courant\n"
                "  D: Activer/désactiver le délai pour le son courant\n"
                "  l / 9: Rejouer le dernier son joué\n"
                "  m: Jouer le son à la position actuelle du curseur\n"
                "  /: Diminuer le décalage (shift) du pad\n"
                "  *: Augmenter le décalage (shift) du pad\n"
                "  <: Aller au début du pattern\n"
                "  >: Aller à la fin du pattern\n"
                "  Flèche Haut: Déplacer le curseur vers le haut et jouer le son\n"
                "  Flèche Bas: Déplacer le curseur vers le bas et jouer le son\n"
                "  Flèche Droite: Déplacer le curseur vers la droite\n"
                "  Flèche Gauche: Déplacer le curseur vers la gauche\n"
                "  PageHaut: Mesure précédente\n"
                "  PageBas: Mesure suivante\n"
                "  Delete: Effacer le dernier son joué au pas de lecture actuel\n"
                "  Ctrl+D: Effacer toutes les occurrences du son au curseur (dans tout le pattern)\n" // Nouveau raccourci
                "  Ctrl+K: Effacer toutes les occurrences du dernier son joué (dans tout le pattern)\n" // Nouveau raccourci
                "  Touches [q-k, a-i]: Jouer le son correspondant\n"
                "  Touches [1-8]: Jouer le son correspondant\n"; // Ajout de cette ligne

}
//----------------------------------------


AdikDrum::~AdikDrum() {
    // closeApp();
}
//----------------------------------------

bool AdikDrum::initApp() {
    const int numChannelsMixer = 32; // Clarifier le nom pour le mixer
    const int sampleRate = 44100;
    const int framesPerBuffer = 256; // Nouvelle variable pour la taille du buffer
    mixer_ = AudioMixer(numChannelsMixer);

    // Générer les sons du métronome
    SoundPtr soundClick1 = mixer_.genTone("buzzer", 880.0, 50); // Son aigu
    SoundPtr soundClick2 = mixer_.genTone("buzzer", 440.0, 50); // Son aigu

    float fadeOutStartPercentage = 0.1f; // Appliquer le fondu à partir d'un pourcentage de la longueur
    soundClick1->applyStaticFadeOutLinear(fadeOutStartPercentage);
    soundClick2->applyStaticFadeOutLinear(fadeOutStartPercentage);
 
    // global structure for now
    drumData_.player = &drumPlayer_;
    drumData_.mixer = &mixer_;
    drumData_.sampleRate = sampleRate;
    drumPlayer_.setMixer(mixer_); // Assigner le mixer à player
    loadSounds(); // charger les sons
    // genTones();
    drumPlayer_.drumSounds_ = this->getDrumSounds();

    // Assigner les sons du métronome à DrumPlayer
    drumPlayer_.soundClick1_ = soundClick1;
    drumPlayer_.soundClick2_ = soundClick2;

    const int numOutputChannels = 2; // Définir explicitement le nombre de canaux de sortie
    if (!audioDriver_.init(numOutputChannels, sampleRate, framesPerBuffer, &drumData_)) {
    // if (!audioDriver_.init(numOutputChannels, sampleRate, framesPerBuffer, drumMachineCallback, &drumData_)) {
        std::cerr << "Erreur lors de l'initialisation de l'AudioDriver." << std::endl;
        return 1;
    }

    if (!audioDriver_.start()) {
        std::cerr << "Erreur lors du démarrage de l'AudioDriver." << std::endl;
        audioDriver_.stop();
        return 1;
    }

    // Tester les sons
    demo();

    std::cout << "AdikDrum initialisé et démarré." << std::endl;
    // std::cin.get();

    return true;
}
//----------------------------------------

void AdikDrum::closeApp() {
    audioDriver_.stop();
    // audioDriver_.close(); // not nessary cause it managing by the AudioDriver's destructor
    std::cout << "AdikDrum fermé." << std::endl;

}
//----------------------------------------

void AdikDrum::loadSounds() {
    auto soundCount = SOUND_LIST.size();
    drumSounds_.clear();
    drumSounds_.resize(soundCount); // Redimensionner drumSounds_ en fonction du nombre de fichiers à charger

    for (size_t i = 0; i < soundCount; ++i) {
        std::string filePath = MEDIA_DIR + "/" + SOUND_LIST[i]; // Construire le chemin complet du fichier
        SoundPtr sound = mixer_.loadSound(filePath); // Charger le fichier

        if (sound->getLength() > 0) {
            drumSounds_[i] = sound;
            std::cout << "Loaded " << SOUND_LIST[i] << " at index " << i << std::endl;
        } else {
            std::cerr << "Error loading " << filePath << ". Loading default sound instead." << std::endl;
        }
    }

    /*
    float fadeOutStartPercentage = 0.3f;
    for (auto& sound : drumSounds_) {
        if (sound) {
            sound->applyStaticFadeOutLinear(fadeOutStartPercentage);
        }
    }
    */


}
//----------------------------------------


void AdikDrum::genTones() {
    const float defaultFrequency = 440.0;
    const float defaultDuration = 0.1;
    drumSounds_.clear(); // S'assurer que le vecteur est vide avant de charger

    drumSounds_.push_back(mixer_.genTone("kick"));
    drumSounds_.push_back(mixer_.genTone("snare"));
    drumSounds_.push_back(mixer_.genTone("hihat", defaultFrequency, 0.25));
    drumSounds_.push_back(mixer_.genTone("kick2"));
    drumSounds_.push_back(mixer_.genTone("snare2"));
    drumSounds_.push_back(mixer_.genTone("cymbal", defaultFrequency, 3.0));
    drumSounds_.push_back(mixer_.genTone("sine", 440.0));
    drumSounds_.push_back(mixer_.genTone("sine", 550.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 220.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 330.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("hihat", defaultFrequency, 0.5));
    drumSounds_.push_back(mixer_.genTone("sine", 110.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 165.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 165.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 175.0, defaultDuration));
    drumSounds_.push_back(mixer_.genTone("sine", 440.0, 0.3));
    
    float fadeOutStartPercentage = 0.3f; // Appliquer le fondu à partir d'un pourcentage de la longueur
    // float expFadeOutStartPercentage = 0.8f; // Commencer le fondu à 60% pour les clics
    // float exponentialPower = 3.0f; // Facteur de puissance pour le fondu exponentiel

    for (auto&  sound : drumSounds_) {
      // std::cout << "voici len: " << sound->getLength() << " et pourcentage de début de fadeout: " << fadeOutStartPercentage << std::endl;
      // Appliquer un fondu linéaire par défaut
      sound->applyStaticFadeOutLinear(fadeOutStartPercentage);
      
      // std::cout << "Appliquer un fondu exponentiel au clic." << std::endl;
      // sound->applyStaticFadeOutExp(expFadeOutStartPercentage, exponentialPower);

    }
      

}
//----------------------------------------


const std::vector<SoundPtr>& AdikDrum::getDrumSounds() const {
    return drumSounds_;
}
//----------------------------------------

void AdikDrum::demo() {
    // Tester les sons
    msgText_ = "Demo en train de jouer";
    displayMessage(msgText_);
    for (size_t i = 0; i < NUM_SOUNDS; ++i) {
        drumPlayer_.playSound(i);
        long long sleepDurationMs = static_cast<long long>(drumPlayer_.drumSounds_[i]->getSize() * 1000.0 / sampleRate_ * 1.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDurationMs));
    }
    msgText_ = "Demo terminée.";
    displayMessage(msgText_);

}
//----------------------------------------

void AdikDrum::loadPattern() {
    msgText_ = "Chargement d'un pattern de démonstration...";
    displayMessage(msgText_);
    
    // Récupère le pattern courant
    auto curPattern = drumPlayer_.curPattern_;
    // Vérifie si le pattern est valide avant de générer des données
    if (curPattern) {
        // Génère les données du pattern directement dans l'objet AdikPattern
        curPattern->genData();
    } else {
        std::cerr << "Erreur: curPattern_ n'est pas initialisé dans DrumPlayer." << std::endl;
        msgText_ = "Erreur: Impossible de charger le pattern de démonstration.";
        displayMessage(msgText_);
        return;
    }

    msgText_ = "Pattern de démonstration chargé.";
    displayMessage(msgText_);
    drumPlayer_.resetMute();

    // Affiche la première barre du pattern (index 0) par défaut.
    // Vous pourriez vouloir afficher la barre courante si vous en avez une.
    displayGrid(curPattern->getPatternBar(0), cursorPos);
}
//----------------------------------------

void AdikDrum::displayMessage(const std::string& message) {
    if (uiApp_) {
        uiApp_->displayMessage(message);
    } else {
        std::cerr << "Erreur: UIApp n'est pas initialisé." << std::endl;
    }
}
//----------------------------------------

void AdikDrum::displayGrid(const std::vector<std::vector<bool>>& grid, std::pair<int, int> cursor) {
    if (uiApp_) {
        auto numSounds = getNumSounds();
        auto numSteps = getNumSteps();
        uiApp_->displayGrid(grid, cursor, numSounds, numSteps);
    } else {
        std::cerr << "Erreur: UIApp n'est pas initialisé." << std::endl;
    }
}
//----------------------------------------


void AdikDrum::selectStep() {
    // Utiliser la barre courante du pattern
    size_t currentBar = drumPlayer_.curPattern_->getCurrentBar();
    // Accéder et modifier directement le pas dans la barre courante du pattern
    drumPlayer_.curPattern_->getPatternBar(currentBar)[cursorPos.second][cursorPos.first] = true;
    msgText_ = "Step " + std::to_string(cursorPos.first + 1) + " on sound " + std::to_string(cursorPos.second + 1) + " activated and playing.";
    displayMessage(msgText_);
    drumPlayer_.playSound(cursorPos.second);
    // Afficher la grille mise à jour pour la barre courante
    displayGrid(drumPlayer_.curPattern_->getPatternBar(currentBar), cursorPos);
}
//----------------------------------------

void AdikDrum::unselectStep() {
    // Utiliser la barre courante du pattern
    size_t currentBar = drumPlayer_.curPattern_->getCurrentBar();
    // Accéder et modifier directement le pas dans la barre courante du pattern
    drumPlayer_.curPattern_->getPatternBar(currentBar)[cursorPos.second][cursorPos.first] = false;
    msgText_ = "Step " + std::to_string(cursorPos.first + 1) + " on sound " + std::to_string(cursorPos.second + 1) + " deactivated.";
    displayMessage(msgText_);
    // Afficher la grille mise à jour pour la barre courante
    displayGrid(drumPlayer_.curPattern_->getPatternBar(currentBar), cursorPos);
}
//----------------------------------------

void AdikDrum::moveCursorUp() {
    if (cursorPos.second > 0) {
        cursorPos.second--;
        size_t currentBar = drumPlayer_.curPattern_->getCurrentBar();
        displayGrid(drumPlayer_.curPattern_->getPatternBar(currentBar), cursorPos);
        msgText_ = "Cursor up, playing sound " + std::to_string(cursorPos.second + 1);
        displayMessage(msgText_);
        drumPlayer_.playSound(cursorPos.second);
    } else {
        beep();
    }
}
//----------------------------------------

void AdikDrum::moveCursorDown() {
    if (cursorPos.second < numSounds_ - 1) {
        cursorPos.second++;
        size_t currentBar = drumPlayer_.curPattern_->getCurrentBar();
        displayGrid(drumPlayer_.curPattern_->getPatternBar(currentBar), cursorPos);
        msgText_ = "Cursor down, playing sound " + std::to_string(cursorPos.second + 1);
        displayMessage(msgText_);
        drumPlayer_.playSound(cursorPos.second);
    } else {
        beep();
    }
}
//----------------------------------------

void AdikDrum::moveCursorRight() {
    if (cursorPos.first < numSteps_ - 1) {
        cursorPos.first++;
        size_t currentBar = drumPlayer_.curPattern_->getCurrentBar();
        displayGrid(drumPlayer_.curPattern_->getPatternBar(currentBar), cursorPos);
        msgText_ = "Cursor right, step " + std::to_string(cursorPos.first + 1);
        displayMessage(msgText_);
    } else {
        beep();
        displayMessage("Reached the end (right).");
    }
}
//----------------------------------------

void AdikDrum::moveCursorLeft() {
    if (cursorPos.first > 0) {
        cursorPos.first--;
        size_t currentBar = drumPlayer_.curPattern_->getCurrentBar();
        displayGrid(drumPlayer_.curPattern_->getPatternBar(currentBar), cursorPos);
        msgText_ = "Cursor left, step " + std::to_string(cursorPos.first + 1);
        displayMessage(msgText_);
    } else {
        beep();
        displayMessage("Reached the beginning (left).");
    }
}
//----------------------------------------

void AdikDrum::playPause() {
    if (drumPlayer_.isPlaying()) { // Si la lecture est en cours
        drumPlayer_.stopPlay(); // Arrête la lecture (ce qui arrêtera aussi l'enregistrement)
        msgText_ = "Lecture: PAUSE. Enregistrement INACTIF.";
    } else { // Si la lecture est en pause
        drumPlayer_.startPlay(); // Démarre la lecture
        msgText_ = "Lecture: EN COURS.";
    }
    displayMessage(msgText_);
    // Optionnel : Vous pourriez vouloir rafraîchir la grille ici si l'état de lecture
    // affecte son affichage (par exemple, si le curseur de lecture s'arrête).
    // displayGrid(drumPlayer_.curPattern_->getPatternBar(drumPlayer_.curPattern_->getCurrentBar()), cursorPos);
}
//----------------------------------------

void AdikDrum::toggleClick() {
    drumPlayer_.toggleClick();
    msgText_ = std::string("Metronome: ") + (drumPlayer_.isClicking() ? "ON" : "OFF");
    displayMessage(msgText_);
    if (drumPlayer_.isClicking()) {
        drumPlayer_.startClick();
    } else {
        drumPlayer_.stopClick();
    }
}
//----------------------------------------

void AdikDrum::stopAllSounds() {
    drumPlayer_.stopAllSounds();
    msgText_ = "All sounds stopped.";
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::toggleMute() {
    int currentSoundIndex = cursorPos.second;
    bool currentMuted = drumPlayer_.isSoundMuted(currentSoundIndex);
    drumPlayer_.setSoundMuted(currentSoundIndex, !currentMuted);
    msgText_ = "Son " + std::to_string(currentSoundIndex) +
               " (canal " + std::to_string(currentSoundIndex + 1) +
               ") est maintenant " + (currentMuted ? "démuté" : "muté") + ".";
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::resetMute() {
    drumPlayer_.resetMute();
    msgText_ = "Tous les sons ont été démutés.";
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::changeVolume(float deltaVolume) {
    float currentVolume = mixer_.getGlobalVolume();
    mixer_.setGlobalVolume(std::clamp(currentVolume + deltaVolume, 0.0f, 1.0f));
    msgText_ = "Volume global: " + std::to_string(static_cast<int>(mixer_.getGlobalVolume() * 10)) + "/10";
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::changeBpm(float deltaBpm) {
    auto bpm = drumPlayer_.getBpm();
    drumPlayer_.setBpm(bpm + deltaBpm);
    msgText_ = "BPM réglé à " + std::to_string(drumPlayer_.getBpm());
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::changePan(float deltaPan) {
    int currentChannelIndex = cursorPos.second + 1;
    float currentPan = mixer_.getChannelPan(currentChannelIndex);
    mixer_.setChannelPan(currentChannelIndex, std::clamp(currentPan + deltaPan, -1.0f, 1.0f));
    msgText_ = "Pan du canal " + std::to_string(currentChannelIndex) +
               " réglé à " + std::to_string(mixer_.getChannelPan(currentChannelIndex));
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::playKey(int soundIndex) {
    soundIndex += shiftPadIndex_;
    drumPlayer_.playSound(soundIndex);
}
//----------------------------------------

void AdikDrum::playKeyPad(int soundIndex) {
    soundIndex += shiftPadIndex_;
    drumPlayer_.playSound(soundIndex);
}
//----------------------------------------


void AdikDrum::triggerLastSound() {
    drumPlayer_.playLastSound();
    // msgText_ = "Rejouer le dernier son sur le canal 17.";
    // displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::playCurrentSound() {
    size_t currentSoundIndex = static_cast<size_t>(cursorPos.second);
    drumPlayer_.playSound(currentSoundIndex);
    // msgText_ = "Jouer le son " + std::to_string(currentSoundIndex + 1) + " (canal " + std::to_string(currentSoundIndex + 1) + ").";
    // displayMessage(msgText_);
}
//----------------------------------------


void AdikDrum::changeSpeed(float speed) {
    int currentChannelIndex =  drumPlayer_.getLastSoundIndex() + 1;
    mixer_.setSpeed(currentChannelIndex, std::clamp(mixer_.getChannelList()[currentChannelIndex].speed + speed, 0.25f, 4.0f));
    std::string msgText = "Vitesse du canal " + std::to_string(currentChannelIndex) + " réglée à " + std::to_string(mixer_.getChannelList()[currentChannelIndex].speed);
    displayMessage(msgText);
}
//----------------------------------------

void AdikDrum::toggleDelay() {
    int currentChannelIndex =  drumPlayer_.getLastSoundIndex() + 1;
    bool active = mixer_.isDelayActive(currentChannelIndex);
    mixer_.setDelayActive(currentChannelIndex, !active);
    msgText_ = "Délai du Canal (" + std::to_string(currentChannelIndex + 1) +
               ") est maintenant " + (active ? "désactivé" : "activé") + ".";

    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::changeShiftPad(size_t deltaShiftPad) {
    // Note: converti le résultat en int pour que le compilateur ne converti pas un nombre négatif en un grand nombre unsigned, du fait que le type est size_t
    int tempVal = shiftPadIndex_ + deltaShiftPad;
    shiftPadIndex_ = std::clamp(tempVal, 0, 32);
    msgText_ = "Shift Pad: " + std::to_string(shiftPadIndex_) + "/32";
    displayMessage(msgText_);
}
//----------------------------------------

void AdikDrum::changeBar(int delta) {
    if (!drumPlayer_.curPattern_) {
        msgText_ = "Erreur: Aucun pattern chargé pour changer de mesure.";
        displayMessage(msgText_);
        return;
    }

    size_t curBar = drumPlayer_.curPattern_->getCurrentBar();
    size_t numBars = drumPlayer_.curPattern_->getBar(); // Get total number of bars

    // Calculate new bar index, ensuring it stays within bounds
    // Note: Need to cast currentBar to int for arithmetic with delta, then back to size_t for clamp
    int barInt = static_cast<int>(curBar) + delta;
    // Note: cannot convert int to size_t,  cause the int will be maximum int value.
    size_t barIndex = std::clamp(barInt, 0, static_cast<int>(numBars -1));

    // Set the new position, keeping the current step
    drumPlayer_.curPattern_->setPosition(barIndex, 0);

    msgText_ = "Mesure changée à : " + std::to_string(barIndex + 1) + "/" + std::to_string(numBars);
    displayMessage(msgText_);

    // Update the displayed grid to the new bar
    displayGrid(drumPlayer_.curPattern_->getPatternBar(barIndex), cursorPos);
}
//----------------------------------------

void AdikDrum::gotoStart() {
    const auto& curPattern = drumPlayer_.curPattern_;
    if (!curPattern) {
        msgText_ = "Erreur: Aucun pattern chargé pour aller au début.";
        displayMessage(msgText_);
        return;
    }
    curPattern->setPosition(0, 0);
    auto curBar = curPattern->getCurrentBar();
    auto numBars = curPattern->getBar();

    msgText_ = "Première mesure: " + std::to_string(curBar +1) + "/" + std::to_string(numBars);
    displayMessage(msgText_);
    displayGrid(drumPlayer_.curPattern_->getPatternBar(0), cursorPos);
}
//----------------------------------------

void AdikDrum::gotoEnd() {
    if (!drumPlayer_.curPattern_) {
        msgText_ = "Erreur: Aucun pattern chargé pour aller à la fin.";
        displayMessage(msgText_);
        return;
    }
    size_t numBars = drumPlayer_.curPattern_->getBar();
    if (numBars > 0) {
        drumPlayer_.curPattern_->setPosition(numBars - 1, drumPlayer_.curPattern_->getCurrentStep());
        msgText_ = "Dernière mesure: " + std::to_string(numBars) + "/" + std::to_string(numBars);
        displayMessage(msgText_);
        displayGrid(drumPlayer_.curPattern_->getPatternBar(numBars - 1), cursorPos);
    } else {
        msgText_ = "Aucune mesure à laquelle aller.";
        displayMessage(msgText_);
    }
}
//----------------------------------------

void AdikDrum::setUIApp(UIApp* uiApp) {
    uiApp_ = uiApp;
}
//----------------------------------------

void AdikDrum::toggleRecord() {
    if (drumPlayer_.isRecording()) { // Si c'était en enregistrement, on l'arrête
        drumPlayer_.stopRecord();
        msgText_ = "Enregistrement: INACTIF. Lecture continue.";
    } else { // Si ce n'était pas en enregistrement, on le démarre
        drumPlayer_.startRecord();
        msgText_ = "Enregistrement: ACTIF. Métronome et lecture démarrés.";
    }
    displayMessage(msgText_);
    // Optionnel: Mettre à jour la grille au cas où le pattern ait été effacé au démarrage de l'enregistrement
    // displayGrid(drumPlayer_.curPattern_->getPatternBar(drumPlayer_.curPattern_->getCurrentBar()), cursorPos);
}
//----------------------------------------

void AdikDrum::recordSound(size_t soundIndex) {
    if (!drumPlayer_.curPattern_ || !drumPlayer_.isPlaying()) {
        // Pas de displayMessage ici. Le message sera mis à jour par le mécanisme normal de l'UI.
        return;
    }

    // Jouer le son IMMÉDIATEMENT pour le retour sonore sans latence
    // La fonction playSound de DrumPlayer utilise le mixer_
    drumPlayer_.playSound(soundIndex);

    // Capture le temps exact de la frappe juste après playSound
    // car playSound peut introduire un petit délai.
    // L'important est que ce timestamp soit le plus proche possible du moment où le son est audible.
    std::chrono::high_resolution_clock::time_point keyPressTime = std::chrono::high_resolution_clock::now(); // <-- Capture ici


    // Enregistrer le pas dans la liste des enregistrements en attente
    size_t currentBar = drumPlayer_.getCurrentBar();
    size_t currentStep = drumPlayer_.getCurrentStep();

    // ICI : Appel à la fonction de quantification
    size_t quantizedStep = drumPlayer_.quantizeRecord(currentStep, keyPressTime);

    // Appel à la fonction de quantification simplifiée
    // size_t quantizedStep = drumPlayer_.quantizeStep(currentStep); // Ne passe que currentStep
    drumPlayer_.addPendingRecording(static_cast<int>(soundIndex), currentBar, currentStep);

}


//----------------------------------------


/*
void AdikDrum::recordSound(size_t soundIndex) {
    if (!drumPlayer_.isRecording()) {
        return; // Ne fait rien si l'enregistrement n'est pas actif
    }

    if (drumPlayer_.curPattern_) {
        // La logique d'enregistrement du pas est maintenant dans DrumPlayer
        drumPlayer_.recordStep(soundIndex);

        // size_t currentBar = drumPlayer_.curPattern_->getCurrentBar();
        size_t currentStep = drumPlayer_.getCurrentStep(); // Accède à la variable membre currentStep de drumPlayer

        msgText_ = "Enregistré: Son " + std::to_string(soundIndex + 1) + " au pas " + std::to_string(currentStep + 1);
        // displayMessage(msgText_);

        // Mettre à jour l'affichage de la grille pour montrer le pas enregistré
        // displayGrid(drumPlayer_.curPattern_->getPatternBar(currentBar), cursorPos);
    } else {
        msgText_ = "Erreur: Aucun pattern chargé pour enregistrer.";
        displayMessage(msgText_);
    }
}
//----------------------------------------
*/

void AdikDrum::deleteLastPlayedStep() {
    int lastSoundIndex = drumPlayer_.getLastSoundIndex();
    if (lastSoundIndex == -1) {
        msgText_ = "Aucun son joué récemment pour supprimer.";
        displayMessage(msgText_);
        return;
    }

    if (!drumPlayer_.curPattern_) {
        msgText_ = "Erreur: Aucun pattern chargé pour supprimer des pas.";
        displayMessage(msgText_);
        return;
    }

    size_t currentBar = drumPlayer_.curPattern_->getCurrentBar();
    size_t currentStep = drumPlayer_.currentStep_; // Le pas de lecture actuel

    // Appel de la fonction DrumPlayer. La fonction de DrumPlayer retourne un booléen
    // pour indiquer si la suppression a réussi.
    bool success = drumPlayer_.deleteStepAtPos(lastSoundIndex, currentStep, currentBar);

    if (success) {
        msgText_ = "Effacé: Son " + std::to_string(lastSoundIndex + 1) + " au pas " + std::to_string(currentStep + 1);
        displayMessage(msgText_);

        // Mettre à jour l'affichage de la grille
        displayGrid(drumPlayer_.curPattern_->getPatternBar(currentBar), cursorPos);
    } else {
        // Ce cas devrait être rare si les checks sont faits en amont ou si drumPlayer::deleteStepAtPos est robuste
        msgText_ = "Erreur lors de la suppression du pas. Indices invalides.";
        displayMessage(msgText_);
    }
}
//----------------------------------------

void AdikDrum::clearCurrentSound() {
    int currentSoundIndex = cursorPos.second; // Le son à la position Y du curseur

    if (!drumPlayer_.curPattern_) {
        msgText_ = "Erreur: Aucun pattern chargé pour effacer des sons.";
        displayMessage(msgText_);
        return;
    }

    // Appeler la logique de DrumPlayer
    bool changed = drumPlayer_.clearSoundFromPattern(currentSoundIndex);

    if (changed) {
        msgText_ = "Effacé: Toutes les occurrences du son " + std::to_string(currentSoundIndex + 1) + ".";
        displayMessage(msgText_);
        // Mettre à jour l'affichage de la grille
        displayGrid(drumPlayer_.curPattern_->getPatternBar(drumPlayer_.curPattern_->getCurrentBar()), cursorPos);
    } else {
        msgText_ = "Le son " + std::to_string(currentSoundIndex + 1) + " n'avait aucune occurrence.";
        displayMessage(msgText_);
    }
}
//----------------------------------------

void AdikDrum::clearLastPlayedSound() {
    int lastSoundIndex = drumPlayer_.getLastSoundIndex(); // Récupérer le dernier son joué
    if (lastSoundIndex == -1) {
        msgText_ = "Aucun son joué récemment pour effacer.";
        displayMessage(msgText_);
        return;
    }

    if (!drumPlayer_.curPattern_) {
        msgText_ = "Erreur: Aucun pattern chargé pour effacer des sons.";
        displayMessage(msgText_);
        return;
    }

    bool changed = drumPlayer_.clearSoundFromPattern(lastSoundIndex);
    if (changed) {
        msgText_ = "Effacé: Toutes les occurrences du dernier son joué (" + std::to_string(lastSoundIndex + 1) + ").";
        displayMessage(msgText_);
        // Mettre à jour l'affichage de la grille
        displayGrid(drumPlayer_.curPattern_->getPatternBar(drumPlayer_.curPattern_->getCurrentBar()), cursorPos);
    } else {
        msgText_ = "Le dernier son joué (" + std::to_string(lastSoundIndex + 1) + ") n'avait aucune occurrence.";
        displayMessage(msgText_);
    }
}
//----------------------------------------

void AdikDrum::toggleHelp() {
    helpDisplayed_ = !helpDisplayed_; // Inverse l'état d'affichage de l'aide

    if (helpDisplayed_) {
        // Sauvegarder le message actuel avant d'afficher l'aide
        previousMsgText_ = msgText_; // Assurez-vous d'avoir un membre previousMsgText_
        msgText_ = helpText_;
        displayMessage(msgText_); 
    } else {
        // Restaurer le message précédent ou effacer l'aide
        msgText_ = previousMsgText_; // Restaurer le message d'origine
        displayMessage(msgText_); // Rafraîchir l'affichage
    }
    // L'affichage de la grille n'est pas affecté directement par l'aide
}
//----------------------------------------


//==== End of class AdikDrum ====

} // namespace adikdrum

/*
int main() {
    AdikDrum adikDrumApp(nullptr); // Créer AdikDrum sans UIApp pour l'instant
    ConsoleUIApp consoleUI(adikDrumApp); // Créer ConsoleUIApp en passant une référence à AdikDrum
    adikDrumApp.uiApp_ = &consoleUI; // Assigner l'UIApp à AdikDrum
    if (!adikDrumApp.initApp()) {
        return false;
    }

    if (consoleUI.init()) {
        consoleUI.run();
        consoleUI.close();
    }
    adikDrumApp.closeApp();

    return 0;
}
*/
//----------------------------------------


/*
 * File: commands.cpp
 * Test for parsing string command on C++
 * Compile:
 * g++ commands.cpp -o commands
 * Date: Sun, 01/06/2025
 * Author: Google Gemini
 * */

#include "adikcommands.h"
#include "adikdrum.h" // Inclut le header complet de AdikDrum pour accéder à ses méthodes
#include <iostream>
#include <string>
#include <vector>
#include <sstream> // Pour std::istringstream
#include <algorithm> // Pour std::remove_if, std::isspace

namespace adikdrum {

// Définition de la COMMAND_MAP
const std::map<std::string, CommandAction> COMMAND_MAP = {
    {"playpause", {
        [](AdikDrum* drum, [[maybe_unused]] const std::vector<std::string>& args) {
            if (drum) {
                drum->playPause();
            }
        },
        "playpause / pp: Démarre ou met en pause la lecture du séquenceur."
    }},
    {"pp", { // Alias pour playpause
        [](AdikDrum* drum, [[maybe_unused]] const std::vector<std::string>& args) {
            if (drum) {
                drum->playPause();
            }
        },
        "playpause / pp: Démarre ou met en pause la lecture du séquenceur."
    }},
    {"bpm", {
        [](AdikDrum* drum, const std::vector<std::string>& args) {
            if (drum && args.size() == 1) {
                try {
                    float bpm = std::stof(args[0]);
                    drum->changeBpm(bpm);
                } catch (const std::exception& e) {
                    // Pour l'instant, on ne gère pas directement les messages d'erreur via displayMessage ici.
                    // Ils seront gérés dans executeCommand.
                    std::cerr << "Erreur BPM: " << e.what() << std::endl;
                }
            }
        },
        "bpm <valeur>: Règle le tempo en BPM."
    }},
    {"quantplayreso", {
        [](AdikDrum* drum, const std::vector<std::string>& args) {
            if (drum && args.size() == 1) {
                try {
                    // int reso = std::stoi(args[0]);
                    // drum->setPlayQuantizeResolution(reso);
                } catch (const std::exception& e) {
                    std::cerr << "Erreur Quantize: " << e.what() << std::endl;
                }
            }
        },
        "quantplayreso <résolution>: Règle la résolution de quantification de lecture."
    }},
    {"mute", {
        [](AdikDrum* drum, const std::vector<std::string>& args) {
            if (drum) {
                if (args.empty()) {
                    drum->toggleMute(); // Mute le son courant si pas d'argument
                } else if (args.size() == 1) {
                    try {
                        // int soundIndex = std::stoi(args[0]);
                        // drum->muteSound(soundIndex); // Mute un son spécifique par index
                    } catch (const std::exception& e) {
                         std::cerr << "Erreur Mute: " << e.what() << std::endl;
                    }
                }
            }
        },
        "mute [index]: Coupe le son courant ou le son à l'index spécifié."
    }},
    {"unmute", {
        [](AdikDrum* drum, const std::vector<std::string>& args) {
            if (drum && args.size() == 1) {
                try {
                    // int soundIndex = std::stoi(args[0]);
                    // drum->unmuteSound(soundIndex); // Unmute un son spécifique par index
                } catch (const std::exception& e) {
                     std::cerr << "Erreur Unmute: " << e.what() << std::endl;
                }
            }
        },
        "unmute <index>: Réactive le son à l'index spécifié."
    }},
    {"resetmute", {
        [](AdikDrum* drum, [[maybe_unused]] const std::vector<std::string>& args) {
            if (drum) {
                // drum->resetMute();
            }
        },
        "resetmute: Réactive tous les sons mutés."
    }},
    {"volume", {
        [](AdikDrum* drum, const std::vector<std::string>& args) {
            if (drum && args.size() == 1) {
                try {
                    float change = std::stof(args[0]);
                    drum->changeVolume(change);
                } catch (const std::exception& e) {
                    std::cerr << "Erreur Volume: " << e.what() << std::endl;
                }
            }
        },
        "volume <changement>: Change le volume global (+/- 0.1 par exemple)."
    }},
    {"pan", {
        [](AdikDrum* drum, const std::vector<std::string>& args) {
            if (drum && args.size() == 1) {
                try {
                    float change = std::stof(args[0]);
                    drum->changePan(change);
                } catch (const std::exception& e) {
                    std::cerr << "Erreur Pan: " << e.what() << std::endl;
                }
            }
        },
        "pan <changement>: Change le panoramique (+/- 0.1 par exemple)."
    }},
    {"speed", {
        [](AdikDrum* drum, const std::vector<std::string>& args) {
            if (drum && args.size() == 1) {
                try {
                    float change = std::stof(args[0]);
                    drum->changeSpeed(change);
                } catch (const std::exception& e) {
                    std::cerr << "Erreur Speed: " << e.what() << std::endl;
                }
            }
        },
        "speed <changement>: Change la vitesse de lecture (+/- 0.25 par exemple)."
    }},
    {"delay", {
        [](AdikDrum* drum, [[maybe_unused]] const std::vector<std::string>& args) {
            if (drum) {
                drum->toggleDelay();
            }
        },
        "delay: Active/désactive l'effet de délai."
    }},
    {"clear", {
        [](AdikDrum* drum, [[maybe_unused]] const std::vector<std::string>& args) {
            if (drum) {
                drum->clearCurrentSound();
            }
        },
        "clear: Efface tous les pas du son courant."
    }},
    {"record", {
        [](AdikDrum* drum, [[maybe_unused]] const std::vector<std::string>& args) {
            if (drum) {
                drum->toggleRecord();
            }
        },
        "record: Active/désactive le mode d'enregistrement."
    }},
    {"load", {
        [](AdikDrum* drum, const std::vector<std::string>& args) {
            if (drum && args.size() == 1) {
                // drum->loadPattern(args[0]); // Supposons que loadPattern prend un nom de fichier
            } else if (drum && args.empty()) {
                drum->loadPattern(); // S'il y a une version sans argument (par défaut/dernier)
            }
        },
        "load [fichier]: Charge un pattern depuis un fichier ou le dernier chargé."
    }},
    {"save", {
        [](AdikDrum* drum, const std::vector<std::string>& args) {
            if (drum && args.size() == 1) {
                // drum->savePattern(args[0]); // Supposons que savePattern prend un nom de fichier
            } else if (drum && args.empty()) {
                // drum->savePattern(); // S'il y a une version sans argument (sauvegarde par défaut)
            }
        },
        "save [fichier]: Sauvegarde le pattern dans un fichier ou le dernier utilisé."
    }},
    {"help", {
        [](AdikDrum* drum, [[maybe_unused]] const std::vector<std::string>& args) {
            if (drum) {
                drum->toggleHelp(); // Ou une fonction showHelp(const std::vector<std::string>& args) pour filtrer
            }
        },
        "help: Affiche/masque l'aide."
    }}
    // Ajoute d'autres commandes ici...
};

// Fonction utilitaire pour supprimer les espaces blancs de début/fin
// (trim leading/trailing whitespace)
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (std::string::npos == first) {
        return str; // La chaîne est vide ou ne contient que des espaces blancs
    }
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, (last - first + 1));
}

// Fonction principale pour parser la chaîne de commande
CommandInput parseCommandString(const std::string& inputString) {
    CommandInput result;
    std::string trimmedInput = trim(inputString);

    if (trimmedInput.empty()) {
        return result; // Retourne une commande vide si l'entrée est vide
    }

    std::istringstream iss(trimmedInput);
    std::string segment;

    // Lire le premier mot comme nom de commande
    if (iss >> result.commandName) {
        // Lire le reste des mots comme arguments
        while (iss >> segment) {
            result.args.push_back(segment);
        }
    }

    return result;
}


} // namespace adikdrum

/*
int main() {
    // Exemples de test
    CommandInput cmd1 = parseCommandString("bpm 120");
    cmd1.print(); // Attend: Command: 'bpm', Args: '120'

    CommandInput cmd2 = parseCommandString("  quantplayreso   8   ");
    cmd2.print(); // Attend: Command: 'quantplayreso', Args: '8'

    CommandInput cmd3 = parseCommandString(" play ");
    cmd3.print(); // Attend: Command: 'play', Args: (vide)

    CommandInput cmd4 = parseCommandString(" ");
    cmd4.print(); // Attend: Command: '', Args: (vide)

    CommandInput cmd5 = parseCommandString("  load_pattern my_great_song.adk  ");
    cmd5.print(); // Attend: Command: 'load_pattern', Args: 'my_great_song.adk'

    CommandInput cmd6 = parseCommandString("save_pattern \"my song with spaces.adk\"");
    cmd6.print(); // Attend: Command: 'save_pattern', Args: '"my song with spaces.adk"'
                  // NOTE: La gestion des guillemets autour des arguments est plus complexe et n'est pas incluse ici.

    return 0;
}
*/


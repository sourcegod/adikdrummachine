/*
 * File: commands.cpp
 * Test for parsing string command on C++
 * Compile:
 * g++ commands.cpp -o commands
 * Date: Sun, 01/06/2025
 * Author: Google Gemini
 * */

#include "adikcommands.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream> // Pour std::istringstream
#include <algorithm> // Pour std::remove_if, std::isspace

namespace adikdrum {

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


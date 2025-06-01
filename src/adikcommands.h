#ifndef ADIKCOMMANDS_H
#define ADIKCOMMANDS_H

#include <string>
#include <vector>
#include <iostream> // Pour la méthode print() de CommandInput

namespace adikdrum {

// Structure pour stocker le nom de la commande et ses arguments
struct CommandInput {
    std::string commandName;
    std::vector<std::string> args;

    // Pour l'affichage de debug
    void print() const; // Déclaration de la fonction membre
};

// Déclaration de la fonction utilitaire pour supprimer les espaces blancs
std::string trim(const std::string& str);

// Déclaration de la fonction principale pour parser la chaîne de commande
CommandInput parseCommandString(const std::string& inputString);

} // namespace adikdrum

#endif // ADIKCOMMANDS_H

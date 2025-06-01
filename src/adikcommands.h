#ifndef ADIKCOMMANDS_H
#define ADIKCOMMANDS_H

#include <string>
#include <vector>
#include <iostream> // Pour la méthode print() de CommandInput
#include <map>
#include <functional> // Pour std::function
#include <memory>     // Pour std::shared_ptr ou std::weak_ptr si tu le gères ainsi

// Forward declaration pour éviter les dépendances circulaires
// AdikDrum n'a pas besoin d'être inclus ici car la fonction prendra un AdikDrum* ou une référence
namespace adikdrum {
    class AdikDrum; // Déclare AdikDrum sans inclure son .h complet
}

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

// Structure pour définir une action de commande
// Elle inclura la fonction à appeler et potentiellement des infos sur les arguments attendus
struct CommandAction {
    // La fonction à exécuter. Prend un pointeur vers AdikDrum et une liste d'arguments.
    // Utiliser std::function permet de stocker des lambdas, des pointeurs sur fonctions membres, etc.
    std::function<void(AdikDrum*, const std::vector<std::string>&)> action;
    std::string helpText; // Description de la commande pour l'aide
    // Tu pourrais ajouter d'autres champs ici, comme:
    // size_t minArgs;
    // size_t maxArgs;
    // std::vector<std::string> argTypes; // Pour la validation des types d'arguments
};

// La map qui stockera les noms/alias de commande et leurs actions associées.
// Déclarée comme 'extern' car elle sera définie dans un fichier .cpp (par exemple, constants.cpp)
// pour éviter les problèmes de "multiple definition" si constants.h est inclus dans plusieurs .cpp.
extern const std::map<std::string, CommandAction> COMMAND_MAP;

} // namespace adikdrum

#endif // ADIKCOMMANDS_H

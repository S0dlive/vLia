#include <iostream>
#include <spdlog/spdlog.h>
#include "config/configmanager.h"
#include "logging/logging.h"
#include "runtime/agentruntime.h"

int main() {
    logging::initLogging();
    logging::activeDebugLogging();

    spdlog::info("Démarrage du test vLia Runtime - Official MCP Servers...");

    try {
        configManager cfgMgr("qwen-abliterated:latest");
        cfgMgr.load();

        agentRuntime runtime(cfgMgr, "agent-test-01");

        spdlog::info("Initialisation de l'environnement Sandbox & MCP...");
        runtime.setup();

        std::string prompt = "Utilise l'outil disponible du serveur MCP pour me donner l'heure actuelle dans le fuseau horaire 'Europe/Paris'.";

        spdlog::info("Envoi du prompt : '{}'" + prompt);
        std::string result = runtime.runUserQuery(prompt);

        std::cout << "\n================ RÉPONSE AGENT ================\n";
        std::cout << result << std::endl;
        std::cout << "===============================================\n";

    } catch (const std::exception& e) {
        spdlog::critical("Erreur pendant le test MCP Officiel : {}" + std::string(e.what()));
        return 1;
    }

    return 0;
}
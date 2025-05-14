#include "stronghold.h"

Conflict::Conflict() : warCount(0) {
    for (int i = 0; i < 50; i++) {
        wars[i] = War();
    }
}

void Conflict::declareWar(Kingdom& attacker, Kingdom& defender) {
    try {
        if (attacker.getName() == defender.getName()) {
            throw GameException("A kingdom cannot declare war on itself.");
        }
        if (warCount >= 50) {
            throw GameException("War storage full.");
        }
        wars[warCount].attackerName = attacker.getName();
        wars[warCount].defenderName = defender.getName();
        warCount++;
        std::cout << attacker.getName() << " declares war on " << defender.getName() << "!\n";
    }
    catch (const GameException& e) {
        std::cout << "Error declaring war: " << e.getMessage() << std::endl;
    }
}

void Conflict::resolveBattle(Kingdom& attacker, Kingdom& defender) {
    try {
        bool warExists = false;
        for (int i = 0; i < warCount; i++) {
            if (wars[i].attackerName == attacker.getName() && wars[i].defenderName == defender.getName()) {
                warExists = true;
                break;
            }
        }
        if (!warExists) {
            throw GameException("No active war between these kingdoms.");
        }
        int attackerStrength = attacker.getMilitary().calculateStrength();
        int defenderStrength = defender.getMilitary().calculateStrength();
        if (attackerStrength > defenderStrength) {
            std::cout << attacker.getName() << " wins the battle against " << defender.getName() << "!\n";
            try {
                Resource& defenderFood = defender.getResource("Food");
                defenderFood.setAmount(defenderFood.getAmount() - 100);
                if (defenderFood.getAmount() < 0) defenderFood.setAmount(0);
            }
            catch (const std::runtime_error&) {
                // Resource not found; skip
            }
        }
        else {
            std::cout << defender.getName() << " repels " << attacker.getName() << "!\n";
            try {
                Resource& attackerFood = attacker.getResource("Food");
                attackerFood.setAmount(attackerFood.getAmount() - 100);
                if (attackerFood.getAmount() < 0) attackerFood.setAmount(0);
            }
            catch (const std::runtime_error&) {
                // Resource not found; skip
            }
        }
    }
    catch (const GameException& e) {
        std::cout << "Error resolving battle: " << e.getMessage() << std::endl;
    }
}

void Conflict::betrayAlly(Kingdom& betrayer, Kingdom& betrayed) {
    try {
        Alliance alliance;
        if (!alliance.areAllied(betrayer, betrayed)) {
            throw GameException("Kingdoms are not allied.");
        }
        alliance.breakAlliance(betrayer, betrayed);
        std::cout << betrayer.getName() << " betrays " << betrayed.getName() << "!\n";
        declareWar(betrayer, betrayed);
    }
    catch (const GameException& e) {
        std::cout << "Error during betrayal: " << e.getMessage() << std::endl;
    }
}

bool Conflict::saveConflictLog(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw GameException("Cannot open conflict log file.");
        }
        for (int i = 0; i < warCount; i++) {
            file << wars[i].attackerName << " vs " << wars[i].defenderName << "\n";
        }
        file.close();
        std::cout << "Conflict log saved to " << filename << std::endl;
        return true;
    }
    catch (const GameException& e) {
        std::cout << "Error saving conflict log: " << e.getMessage() << std::endl;
        return false;
    }
}
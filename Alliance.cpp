#include "stronghold.h"

Alliance::Alliance() : treatyCount(0) {
    for (int i = 0; i < 50; i++) {
        treaties[i] = Treaty();
    }
}

void Alliance::formAlliance(Kingdom& kingdom1, Kingdom& kingdom2, const std::string& treatyName) {
    try {
        if (treatyCount >= 50) {
            throw GameException("Treaty storage full.");
        }
        treaties[treatyCount].kingdom1Name = kingdom1.getName();
        treaties[treatyCount].kingdom2Name = kingdom2.getName();
        treaties[treatyCount].treatyName = treatyName;
        treatyCount++;
        std::cout << kingdom1.getName() << " and " << kingdom2.getName() << " formed alliance: " << treatyName << "\n";
    }
    catch (const GameException& e) {
        std::cout << "Error forming alliance: " << e.getMessage() << std::endl;
    }
}

void Alliance::breakAlliance(Kingdom& kingdom1, Kingdom& kingdom2) {
    try {
        for (int i = 0; i < treatyCount; i++) {
            if ((treaties[i].kingdom1Name == kingdom1.getName() && treaties[i].kingdom2Name == kingdom2.getName()) ||
                (treaties[i].kingdom1Name == kingdom2.getName() && treaties[i].kingdom2Name == kingdom1.getName())) {
                std::cout << "Alliance " << treaties[i].treatyName << " between " << kingdom1.getName() << " and " << kingdom2.getName() << " has been broken.\n";
                treaties[i] = treaties[treatyCount - 1];
                treatyCount--;
                return;
            }
        }
        throw GameException("No alliance found between these kingdoms.");
    }
    catch (const GameException& e) {
        std::cout << "Error breaking alliance: " << e.getMessage() << std::endl;
    }
}

bool Alliance::areAllied(const Kingdom& kingdom1, const Kingdom& kingdom2) const {
    for (int i = 0; i < treatyCount; i++) {
        if ((treaties[i].kingdom1Name == kingdom1.getName() && treaties[i].kingdom2Name == kingdom2.getName()) ||
            (treaties[i].kingdom1Name == kingdom2.getName() && treaties[i].kingdom2Name == kingdom1.getName())) {
            return true;
        }
    }
    return false;
}

void Alliance::displayAlliances() const {
    std::cout << "\nCurrent Alliances:\n";
    if (treatyCount == 0) {
        std::cout << "No alliances formed.\n";
    }
    else {
        for (int i = 0; i < treatyCount; i++) {
            std::cout << treaties[i].treatyName << ": " << treaties[i].kingdom1Name << " and " << treaties[i].kingdom2Name << "\n";
        }
    }
}

bool Alliance::saveTreatyLog(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw GameException("Cannot open treaty log file.");
        }
        for (int i = 0; i < treatyCount; i++) {
            file << treaties[i].treatyName << " " << treaties[i].kingdom1Name << " " << treaties[i].kingdom2Name << "\n";
        }
        file.close();
        std::cout << "Treaty log saved to " << filename << std::endl;
        return true;
    }
    catch (const GameException& e) {
        std::cout << "Error saving treaty log: " << e.getMessage() << std::endl;
        return false;
    }
}
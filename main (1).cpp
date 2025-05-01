#include "Stronghold.h"
#include <iostream>
#include <string>
#include <stdexcept>

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "       STRONGHOLD SIMULATOR       " << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "A medieval kingdom management game" << std::endl;
    std::cout << "\n";
    
    
    int difficulty = 2; 
    std::string difficultyInput;
    
    std::cout << "Select difficulty:" << std::endl;
    std::cout << "1. Easy" << std::endl;
    std::cout << "2. Medium" << std::endl;
    std::cout << "3. Hard" << std::endl;
    std::cout << "Enter your choice (1-3): ";
    
    std::getline(std::cin, difficultyInput);
    try {
        if (!difficultyInput.empty()) {
            difficulty = std::stoi(difficultyInput);
            if (difficulty < 1 || difficulty > 3) {
                std::cout << "Invalid choice. Setting difficulty to Medium (2)." << std::endl;
                difficulty = 2;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Invalid input. Setting difficulty to Medium (2)." << std::endl;
        difficulty = 2;
    }
    
    
    int turns = 100; 
    std::string turnsInput;
    
    std::cout << "\nHow many turns to play? (10-200, default 100): ";
    std::getline(std::cin, turnsInput);
    try {
        if (!turnsInput.empty()) {
            turns = std::stoi(turnsInput);
            if (turns < 10 || turns > 200) {
                std::cout << "Invalid turns range. Setting to default (100)." << std::endl;
                turns = 100;
            }
        }
    } catch (const std::exception& e) {
        std::cout << "Invalid input. Setting turns to default (100)." << std::endl;
        turns = 100;
    }
    
    std::cout << "\nStarting new game with difficulty level " << difficulty;
    std::cout << " for " << turns << " turns." << std::endl;
    std::cout << "Press Enter to continue...";
    std::cin.get();
    
    try {
        
        GameEngine game(difficulty, turns);
        game.mainLoop();
    } catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        std::cerr << "The game has crashed. Please report this issue." << std::endl;
        return 1;
    }
    
    return 0;
}
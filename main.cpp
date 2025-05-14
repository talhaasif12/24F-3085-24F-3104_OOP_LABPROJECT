#include "stronghold.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    std::cout << "==================================" << std::endl;
    std::cout << "       STRONGHOLD SIMULATOR       " << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "A medieval kingdom management game" << std::endl;
    std::cout << "\n";

    // Get difficulty
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
    }
    catch (const std::exception& e) {
        std::cout << "Invalid input. Setting difficulty to Medium (2)." << std::endl;
        difficulty = 2;
    }

    // Get number of turns
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
    }
    catch (const std::exception& e) {
        std::cout << "Invalid input. Setting turns to default (100)." << std::endl;
        turns = 100;
    }

    // Select player kingdom
    int playerKingdomIdx = 0;
    std::string kingdomInput;
    std::cout << "\nSelect your kingdom:" << std::endl;
    std::cout << "1. Aragon" << std::endl;
    std::cout << "2. Bavaria" << std::endl;
    std::cout << "3. Caledonia" << std::endl;
    std::cout << "Enter your choice (1-3): ";
    std::getline(std::cin, kingdomInput);
    try {
        if (!kingdomInput.empty()) {
            playerKingdomIdx = std::stoi(kingdomInput) - 1;
            if (playerKingdomIdx < 0 || playerKingdomIdx > 2) {
                std::cout << "Invalid choice. Selecting Aragon (1)." << std::endl;
                playerKingdomIdx = 0;
            }
        }
    }
    catch (const std::exception& e) {
        std::cout << "Invalid input. Selecting Aragon (1)." << std::endl;
        playerKingdomIdx = 0;
    }

    std::cout << "\nStarting new game with difficulty level " << difficulty;
    std::cout << " for " << turns << " turns." << std::endl;
    std::cout << "Press Enter to continue...";
    std::cin.get();

    try {
        // Initialize kingdoms
        Kingdom kingdoms[3] = {
            Kingdom("Aragon"),
            Kingdom("Bavaria"),
            Kingdom("Caledonia")
        };
        for (int i = 0; i < 3; i++) {
            kingdoms[i].initialize();
            // Set leader for each kingdom
            kingdoms[i].setLeader(new Leader(kingdoms[i].getName() + " King", "Monarch"));
        }

        // Initialize Module 2 components
        Communication comm;
        Alliance alliance; // Note: May need to be persistent; check your Alliance.cpp
        Market market;
        Conflict conflict;
        Map gameMap(10, 10);

        // Place kingdoms on map
        gameMap.placeKingdom(kingdoms[0], 2, 2);
        gameMap.placeKingdom(kingdoms[1], 5, 5);
        gameMap.placeKingdom(kingdoms[2], 8, 3);

        // Initialize GameEngine
        GameEngine game(difficulty, turns);
        // Note: GameEngine likely needs to manage multiple kingdoms; adjust as needed

        // Custom game loop to integrate Module 2
        bool running = true;
        int currentTurn = 1;
        while (running && currentTurn <= turns) {
            std::cout << "\n=== Turn " << currentTurn << " ===" << std::endl;
            std::cout << "Player Kingdom: " << kingdoms[playerKingdomIdx].getName() << "\n";
            std::cout << "1. Display Kingdom Status\n";
            std::cout << "2. Send Message\n";
            std::cout << "3. Display Messages\n";
            std::cout << "4. Form Alliance\n";
            std::cout << "5. Break Alliance\n";
            std::cout << "6. Display Alliances\n";
            std::cout << "7. Offer Trade\n";
            std::cout << "8. Accept Trade\n";
            std::cout << "9. Smuggle Resources\n";
            std::cout << "10. Declare War\n";
            std::cout << "11. Resolve Battle\n";
            std::cout << "12. Betray Ally\n";
            std::cout << "13. Move Kingdom on Map\n";
            std::cout << "14. Display Map\n";
            std::cout << "15. Simulate Turn\n";
            std::cout << "16. Save Logs\n";
            std::cout << "17. Exit\n";
            std::cout << "Enter choice: ";

            int choice;
            std::cin >> choice;
            clearInputBuffer();

            if (choice == 1) {
                kingdoms[playerKingdomIdx].displayStatus();
            }
            else if (choice == 2) {
                int receiverIdx;
                std::string message;
                std::cout << "Receiver (0=Aragon, 1=Bavaria, 2=Caledonia): ";
                std::cin >> receiverIdx;
                clearInputBuffer();
                std::cout << "Message: ";
                std::getline(std::cin, message);
                if (receiverIdx >= 0 && receiverIdx < 3 && receiverIdx != playerKingdomIdx) {
                    comm.sendMessage(kingdoms[playerKingdomIdx], kingdoms[receiverIdx], message);
                }
                else {
                    std::cout << "Invalid receiver.\n";
                }
            }
            else if (choice == 3) {
                comm.displayMessages(kingdoms[playerKingdomIdx]);
            }
            else if (choice == 4) {
                int allyIdx;
                std::string treatyName;
                std::cout << "Ally (0=Aragon, 1=Bavaria, 2=Caledonia): ";
                std::cin >> allyIdx;
                clearInputBuffer();
                std::cout << "Treaty name: ";
                std::getline(std::cin, treatyName);
                if (allyIdx >= 0 && allyIdx < 3 && allyIdx != playerKingdomIdx) {
                    alliance.formAlliance(kingdoms[playerKingdomIdx], kingdoms[allyIdx], treatyName);
                }
                else {
                    std::cout << "Invalid ally.\n";
                }
            }
            else if (choice == 5) {
                int allyIdx;
                std::cout << "Ally to break (0=Aragon, 1=Bavaria, 2=Caledonia): ";
                std::cin >> allyIdx;
                if (allyIdx >= 0 && allyIdx < 3 && allyIdx != playerKingdomIdx) {
                    alliance.breakAlliance(kingdoms[playerKingdomIdx], kingdoms[allyIdx]);
                }
                else {
                    std::cout << "Invalid ally.\n";
                }
            }
            else if (choice == 6) {
                alliance.displayAlliances();
            }
            else if (choice == 7) {
                std::string itemType;
                int quantity, goldCost;
                std::cout << "Item type (Food, Wood, Stone, Iron): ";
                std::getline(std::cin, itemType);
                std::cout << "Quantity: ";
                std::cin >> quantity;
                std::cout << "Gold cost: ";
                std::cin >> goldCost;
                TradeItem<int> item(itemType, quantity, goldCost);
                market.offerTrade(kingdoms[playerKingdomIdx], item);
            }
            else if (choice == 8) {
                int sellerIdx, offerIdx;
                std::cout << "Seller (0=Aragon, 1=Bavaria, 2=Caledonia): ";
                std::cin >> sellerIdx;
                std::cout << "Offer index: ";
                std::cin >> offerIdx;
                if (sellerIdx >= 0 && sellerIdx < 3 && sellerIdx != playerKingdomIdx) {
                    market.acceptTrade(kingdoms[playerKingdomIdx], kingdoms[sellerIdx], offerIdx - 1);
                }
                else {
                    std::cout << "Invalid seller.\n";
                }
            }
            else if (choice == 9) {
                int receiverIdx;
                std::string itemType;
                int quantity;
                std::cout << "Receiver (0=Aragon, 1=Bavaria, 2=Caledonia): ";
                std::cin >> receiverIdx;
                clearInputBuffer();
                std::cout << "Item type (Food, Wood, Stone, Iron): ";
                std::getline(std::cin, itemType);
                std::cout << "Quantity: ";
                std::cin >> quantity;
                if (receiverIdx >= 0 && receiverIdx < 3 && receiverIdx != playerKingdomIdx) {
                    TradeItem<int> item(itemType, quantity, 0);
                    market.smuggle(kingdoms[playerKingdomIdx], kingdoms[receiverIdx], item);
                }
                else {
                    std::cout << "Invalid receiver.\n";
                }
            }
            else if (choice == 10) {
                int defenderIdx;
                std::cout << "Defender (0=Aragon, 1=Bavaria, 2=Caledonia): ";
                std::cin >> defenderIdx;
                if (defenderIdx >= 0 && defenderIdx < 3 && defenderIdx != playerKingdomIdx) {
                    conflict.declareWar(kingdoms[playerKingdomIdx], kingdoms[defenderIdx]);
                }
                else {
                    std::cout << "Invalid defender.\n";
                }
            }
            else if (choice == 11) {
                int defenderIdx;
                std::cout << "Defender (0=Aragon, 1=Bavaria, 2=Caledonia): ";
                std::cin >> defenderIdx;
                if (defenderIdx >= 0 && defenderIdx < 3 && defenderIdx != playerKingdomIdx) {
                    conflict.resolveBattle(kingdoms[playerKingdomIdx], kingdoms[defenderIdx]);
                }
                else {
                    std::cout << "Invalid defender.\n";
                }
            }
            else if (choice == 12) {
                int betrayedIdx;
                std::cout << "Ally to betray (0=Aragon, 1=Bavaria, 2=Caledonia): ";
                std::cin >> betrayedIdx;
                if (betrayedIdx >= 0 && betrayedIdx < 3 && betrayedIdx != playerKingdomIdx) {
                    conflict.betrayAlly(kingdoms[playerKingdomIdx], kingdoms[betrayedIdx]);
                }
                else {
                    std::cout << "Invalid ally.\n";
                }
            }
            else if (choice == 13) {
                int newX, newY;
                std::cout << "New X coordinate (0-9): ";
                std::cin >> newX;
                std::cout << "New Y coordinate (0-9): ";
                std::cin >> newY;
                gameMap.moveKingdom(kingdoms[playerKingdomIdx], newX, newY);
            }
            else if (choice == 14) {
                gameMap.displayMap();
            }
            else if (choice == 15) {
                for (int i = 0; i < 3; i++) {
                    kingdoms[i].simulateTurn();
                }
                currentTurn++;
            }
            else if (choice == 16) {
                comm.saveChatLog("chat_log.txt");
                alliance.saveTreatyLog("treaty_log.txt");
                conflict.saveConflictLog("conflict_log.txt");
                kingdoms[playerKingdomIdx].logScore();
            }
            else if (choice == 17) {
                running = false;
            }
            else {
                std::cout << "Invalid choice.\n";
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << "FATAL ERROR: " << e.what() << std::endl;
        std::cerr << "The game has crashed. Please report this issue." << std::endl;
        return 1;
    }

    return 0;
}
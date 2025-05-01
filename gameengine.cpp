#define _CRT_SECURE_NO_WARNINGS
#include "Stronghold.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>

GameEngine::GameEngine(int difficulty, int turns)
    : gameDifficulty(difficulty), maxTurns(turns), isRunning(false), logSize(0), maxLogSize(100) {

    playerKingdom = nullptr;

    gameLog = new char* [maxLogSize];
    for (int i = 0; i < maxLogSize; i++) {
        gameLog[i] = nullptr;
    }
}

GameEngine::~GameEngine() {
    if (playerKingdom) {
        delete playerKingdom;
    }

    for (int i = 0; i < maxLogSize; i++) {
        if (gameLog[i]) {
            delete[] gameLog[i];
        }
    }
    delete[] gameLog;
}

void GameEngine::initialize() {
    srand(static_cast<unsigned int>(time(nullptr)));

    std::string kingdomName = "Stronghold";
    playerKingdom = new Kingdom(kingdomName);

    playerKingdom->initialize();

    std::string leaderName = "Player";
    std::string leaderTitle = "Ruler";

    Leader* leader = new Leader(leaderName, leaderTitle);

    int baseAttribute = 70 - (gameDifficulty * 10);
    leader->setCharisma(baseAttribute + (rand() % 20));
    leader->setIntelligence(baseAttribute + (rand() % 20));
    leader->setLeadership(baseAttribute + (rand() % 20));
    leader->setCorruption(10 + (rand() % 20) + (gameDifficulty * 5));

    playerKingdom->setLeader(leader);

    if (gameDifficulty == 1) {
        Event* startEvent = new Event(
            "Bountiful Harvest",
            "The kingdom enjoys a bountiful harvest, filling the granaries.",
            "economic",
            3
        );
        startEvent->addEffect("Food", 1.5);
        playerKingdom->addEvent(startEvent);
    }
    else if (gameDifficulty == 3) {
        Event* startEvent = new Event(
            "Border Skirmish",
            "Enemies at the border are causing trouble for the kingdom.",
            "military",
            3
        );
        startEvent->addEffect("military", 0.8);
        playerKingdom->addEvent(startEvent);
    }

    isRunning = true;

    std::stringstream ss;
    ss << "Welcome to the kingdom of " << kingdomName << "! You are now the "
        << leaderTitle << ". Difficulty level: " << gameDifficulty;
    addToLog(ss.str());
}

void GameEngine::mainLoop() {
    initialize();
    render();

    while (isRunning && playerKingdom->getTurn() < maxTurns) {
        std::string input;

        std::cout << "\nEnter command (type 'help' for list of commands): ";
        std::getline(std::cin, input);

        processInput(input);

        if (!isRunning) {
            break;
        }
    }

    endGame();
}

void GameEngine::processInput(const std::string& input)
{
    std::string lowerInput = input;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(),
        [](unsigned char c) { return std::tolower(c); });

    if (lowerInput == "help") {
        displayHelp();
    }
    else if (lowerInput == "next" || lowerInput == "n") {
        update();
        render();
    }
    else if (lowerInput == "status" || lowerInput == "s") {
        playerKingdom->displayStatus();
    }
    else if (lowerInput.substr(0, 4) == "save") {
        std::string filename = "stronghold_save.txt";
        if (input.length() > 5) {
            filename = input.substr(5);
        }
        saveGame(filename);
    }
    else if (lowerInput.substr(0, 4) == "load") {
        std::string filename = "stronghold_save.txt";
        if (input.length() > 5) {
            filename = input.substr(5);
        }
        loadGame(filename);
        render();
    }
    else if (lowerInput == "quit" || lowerInput == "exit" || lowerInput == "q") {
        std::cout << "Are you sure you want to quit? (y/n): ";
        std::string confirm;
        std::getline(std::cin, confirm);
        if (confirm == "y" || confirm == "Y") {
            quit();
        }
    }
    else if (lowerInput.substr(0, 3) == "tax") {
        try {
            if (input.length() > 4) {
                double taxRate = std::stod(input.substr(4)) / 100.0;
                if (taxRate >= 0.0 && taxRate <= 1.0) {
                    playerKingdom->getEconomy().setTaxRate(taxRate);
                    std::stringstream ss;
                    ss << "Tax rate set to " << (taxRate * 100.0) << "%";
                    addToLog(ss.str());
                }
                else {
                    addToLog("Tax rate must be between 0 and 100 percent.");
                }
            }
            else {
                addToLog("Current tax rate: " +
                    std::to_string(playerKingdom->getEconomy().getTaxRate() * 100.0) + "%");
            }
        }
        catch (const std::exception& e) {
            addToLog("Invalid tax rate format. Use 'tax [rate as number]'");
        }
    }
    else if (lowerInput.substr(0, 7) == "recruit") {
        try {
            if (input.length() > 8) {
                int recruitCount = std::stoi(input.substr(8));
                int available = playerKingdom->getPopulation().getAvailableRecruits();

                if (recruitCount <= 0) {
                    addToLog("Must recruit a positive number of soldiers.");
                }
                else if (recruitCount > available) {
                    std::stringstream ss;
                    ss << "Cannot recruit " << recruitCount << " soldiers. Only "
                        << available << " citizens are available for recruitment.";
                    addToLog(ss.str());
                }
                else {
                    double costPerSoldier = 10.0;
                    double totalCost = recruitCount * costPerSoldier;

                    if (playerKingdom->getEconomy().getTreasury() < totalCost) {
                        addToLog("Not enough gold in treasury to recruit soldiers.");
                    }
                    else {
                        playerKingdom->getEconomy().payExpense(totalCost, "Military recruitment");
                        playerKingdom->getMilitary().recruit(recruitCount);

                        std::stringstream ss;
                        ss << "Recruited " << recruitCount << " soldiers for " << totalCost << " gold.";
                        addToLog(ss.str());
                    }
                }
            }
            else {
                addToLog("Usage: recruit [number]");
            }
        }
        catch (const std::exception& e) {
            addToLog("Invalid recruitment format. Use 'recruit [number]'");
        }
    }
    else if (lowerInput.substr(0, 5) == "train") {
        try {
            if (input.length() > 6) {
                int trainingLevel = std::stoi(input.substr(6));
                if (trainingLevel < 1 || trainingLevel > 10) {
                    addToLog("Training level must be between 1 and 10.");
                }
                else {
                    double costPerLevel = 50.0 * trainingLevel;

                    if (playerKingdom->getEconomy().getTreasury() < costPerLevel) {
                        addToLog("Not enough gold in treasury for military training.");
                    }
                    else {
                        playerKingdom->getEconomy().payExpense(costPerLevel, "Military training");
                        playerKingdom->getMilitary().train(trainingLevel * 10);

                        std::stringstream ss;
                        ss << "Military trained at level " << trainingLevel << " for " << costPerLevel << " gold.";
                        addToLog(ss.str());
                    }
                }
            }
            else {
                addToLog("Usage: train [level 1-10]");
            }
        }
        catch (const std::exception& e) {
            addToLog("Invalid training format. Use 'train [level]'");
        }
    }
    else if (lowerInput.substr(0, 6) == "budget") {
        if (input.length() > 7) {
            std::istringstream iss(input.substr(7));
            std::string category;
            double percentage;

            iss >> category >> percentage;

            if (!category.empty() && percentage >= 0 && percentage <= 100) {
                bool success = playerKingdom->getEconomy().allocateBudget(category, percentage / 100.0);
                if (success) {
                    std::stringstream ss;
                    ss << "Budget for " << category << " set to " << percentage << "%";
                    addToLog(ss.str());
                }
                else {
                    addToLog("Failed to allocate budget. Make sure allocations sum to 100%.");
                }
            }
            else {
                addToLog("Usage: budget [category] [percentage]");
            }
        }
        else {
            std::cout << "\n--- Budget Allocations ---" << std::endl;
            std::cout << "Military: " << (playerKingdom->getEconomy().getBudgetFor("Military") * 100) << "%" << std::endl;
            std::cout << "Infrastructure: " << (playerKingdom->getEconomy().getBudgetFor("Infrastructure") * 100) << "%" << std::endl;
            std::cout << "Healthcare: " << (playerKingdom->getEconomy().getBudgetFor("Healthcare") * 100) << "%" << std::endl;
            std::cout << "Education: " << (playerKingdom->getEconomy().getBudgetFor("Education") * 100) << "%" << std::endl;
            std::cout << "Reserve: " << (playerKingdom->getEconomy().getBudgetFor("Reserve") * 100) << "%" << std::endl;
        }
    }
    else if (lowerInput.substr(0, 4) == "loan") {
        if (input.length() > 5) {
            try {
                std::istringstream iss(input.substr(5));
                double amount;
                int term;

                iss >> amount >> term;

                if (amount > 0 && term > 0) {
                    bool success = playerKingdom->getBank().issueLoan("Treasury", amount, term);
                    if (success) {
                        playerKingdom->getEconomy().addIncome(amount, "Bank Loan");

                        std::stringstream ss;
                        ss << "Loan of " << amount << " gold approved for " << term << " months.";
                        addToLog(ss.str());
                    }
                    else {
                        addToLog("Loan request denied. Bank may have insufficient reserves or too many outstanding loans.");
                    }
                }
                else {
                    addToLog("Usage: loan [amount] [term in months]");
                }
            }
            catch (const std::exception& e) {
                addToLog("Invalid loan format. Use 'loan [amount] [term]'");
            }
        }
        else {
            addToLog("Usage: loan [amount] [term in months]");
        }
    }
    else if (lowerInput == "log") {
        std::cout << "\n=== GAME LOG ===" << std::endl;
        for (int i = 0; i < logSize; i++) {
            if (gameLog[i]) {
                std::cout << gameLog[i] << std::endl;
            }
        }
    }
    else {
        addToLog("Unknown command. Type 'help' for list of commands.");
    }
}

void GameEngine::update() {
    playerKingdom->simulateTurn();
    addToLog("Turn " + std::to_string(playerKingdom->getTurn()) + " completed.");

    if (playerKingdom->getTurn() >= maxTurns) {
        addToLog("Maximum number of turns reached.");
        isRunning = false;
    }

    if (!playerKingdom->isStable()) {
        addToLog("Your kingdom has fallen into chaos. Game over.");
        isRunning = false;
    }
}

void GameEngine::render() {
    std::cout << "\n===================================" << std::endl;
    std::cout << "KINGDOM OF " << playerKingdom->getName() << " - TURN " << playerKingdom->getTurn() << std::endl;
    std::cout << "===================================" << std::endl;

    playerKingdom->displayStatus();

    std::cout << "\nRECENT EVENTS:" << std::endl;
    int start = std::max(0, logSize - 5);
    for (int i = start; i < logSize; i++) {
        if (gameLog[i]) {
            std::cout << "- " << gameLog[i] << std::endl;
        }
    }
}

void GameEngine::endGame() {
    std::cout << "\n===================================" << std::endl;
    std::cout << "GAME OVER - FINAL RESULTS" << std::endl;
    std::cout << "===================================" << std::endl;

    playerKingdom->displayStatus();

    double stabilityScore = playerKingdom->calculateStabilityIndex() * 100;
    int populationScore = playerKingdom->getPopulation().getTotalPopulation() / 100;
    double treasuryScore = playerKingdom->getEconomy().getTreasury() / 1000;
    int militaryScore = playerKingdom->getMilitary().getSoldiers() *
        playerKingdom->getMilitary().getTraining() / 100;

    int finalScore = static_cast<int>(stabilityScore + populationScore + treasuryScore + militaryScore);

    std::cout << "\nFINAL SCORE: " << finalScore << std::endl;
    std::cout << "Difficulty Level: " << gameDifficulty << std::endl;
    std::cout << "Turns Survived: " << playerKingdom->getTurn() << " of " << maxTurns << std::endl;

    playerKingdom->logScore();

    std::cout << "\nThank you for playing Stronghold!" << std::endl;
}

void GameEngine::saveGame(const std::string& filename) const {
    bool success = playerKingdom->saveGame(filename);
    if (success) {
        std::cout << "Game saved successfully to " << filename << std::endl;
    }
    else {
        std::cout << "Failed to save game to " << filename << std::endl;
    }
}

bool GameEngine::loadGame(const std::string& filename) {
    bool success = playerKingdom->loadGame(filename);
    if (success) {
        std::cout << "Game loaded successfully from " << filename << std::endl;
        return true;
    }
    else {
        std::cout << "Failed to load game from " << filename << std::endl;
        return false;
    }
}

void GameEngine::displayHelp() const {
    std::cout << "\n===================================" << std::endl;
    std::cout << "STRONGHOLD - COMMAND HELP" << std::endl;
    std::cout << "===================================" << std::endl;
    std::cout << "next, n       - Advance to the next turn" << std::endl;
    std::cout << "status, s     - Display current kingdom status" << std::endl;
    std::cout << "tax [rate]    - Set tax rate (0-100 percent)" << std::endl;
    std::cout << "recruit [num] - Recruit soldiers to your army" << std::endl;
    std::cout << "train [1-10]  - Train your military (levels 1-10)" << std::endl;
    std::cout << "budget        - View current budget allocations" << std::endl;
    std::cout << "budget [cat] [pct] - Set budget allocation for category" << std::endl;
    std::cout << "loan [amt] [term]  - Request a loan from the bank" << std::endl;
    std::cout << "log           - Display the game log" << std::endl;
    std::cout << "save [file]   - Save game (default: stronghold_save.txt)" << std::endl;
    std::cout << "load [file]   - Load game (default: stronghold_save.txt)" << std::endl;
    std::cout << "quit, exit, q - Exit the game" << std::endl;
    std::cout << "help          - Display this help menu" << std::endl;
}

void GameEngine::addToLog(const std::string& message) {
    if (logSize >= maxLogSize) {
        delete[] gameLog[0];

        for (int i = 1; i < maxLogSize; i++) {
            gameLog[i - 1] = gameLog[i];
        }

        logSize = maxLogSize - 1;
    }

    size_t msgLength = message.length();
    gameLog[logSize] = new char[msgLength + 1];
    strncpy(gameLog[logSize], message.c_str(), msgLength);
    gameLog[logSize][msgLength] = '\0';

    logSize++;

    std::cout << message << std::endl;
}

bool GameEngine::running() const {
    return isRunning;
}

void GameEngine::quit() {
    isRunning = false;
    std::cout << "Exiting game..." << std::endl;
}
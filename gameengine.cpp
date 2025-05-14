#define _CRT_SECURE_NO_WARNINGS
#include "stronghold.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <ctime>

GameEngine::GameEngine(int difficulty, int maxTurns)
    : difficulty(difficulty), maxTurns(maxTurns), isRunning(false), logSize(0), maxLogSize(100), gameMap(10, 10) {
    for (int i = 0; i < 3; i++) {
        kingdoms[i] = nullptr;
    }
    gameLog = new char* [maxLogSize];
    for (int i = 0; i < maxLogSize; i++) {
        gameLog[i] = nullptr;
    }
}

GameEngine::~GameEngine() {
    for (int i = 0; i < 3; i++) {
        delete kingdoms[i];
        kingdoms[i] = nullptr;
    }
    for (int i = 0; i < maxLogSize; i++) {
        delete[] gameLog[i];
        gameLog[i] = nullptr;
    }
    delete[] gameLog;
}

void GameEngine::initialize(int playerKingdomIdx) {
    if (playerKingdomIdx < 0 || playerKingdomIdx >= 3) {
        throw GameException("Invalid player kingdom index.");
    }
    srand(static_cast<unsigned int>(time(nullptr)));
    this->playerKingdomIdx = playerKingdomIdx;

    kingdoms[0] = new Kingdom("Aragon");
    kingdoms[1] = new Kingdom("Bavaria");
    kingdoms[2] = new Kingdom("Caledonia");

    for (int i = 0; i < 3; i++) {
        if (!kingdoms[i]) {
            throw GameException("Failed to allocate kingdom.");
        }
        kingdoms[i]->initialize();
        std::string leaderName = kingdoms[i]->getName() + " King";
        Leader* leader = new Leader(leaderName, "Monarch");
        int baseAttribute = 70 - (difficulty * 10);
        leader->setCharisma(baseAttribute + (rand() % 20));
        leader->setIntelligence(baseAttribute + (rand() % 20));
        leader->setLeadership(baseAttribute + (rand() % 20));
        leader->setCorruption(10 + (rand() % 20) + (difficulty * 5));
        kingdoms[i]->setLeader(leader);
    }

    gameMap.placeKingdom(*kingdoms[0], 2, 2);
    gameMap.placeKingdom(*kingdoms[1], 5, 5);
    gameMap.placeKingdom(*kingdoms[2], 8, 3);

    if (difficulty == 1) {
        Event* startEvent = new Event(
            "Bountiful Harvest",
            "The kingdom enjoys a bountiful harvest, filling the granaries.",
            "economic",
            3
        );
        startEvent->addEffect("Food", 1.5);
        kingdoms[playerKingdomIdx]->addEvent(startEvent);
    }
    else if (difficulty == 3) {
        Event* startEvent = new Event(
            "Border Skirmish",
            "Enemies at the border are causing trouble for the kingdom.",
            "military",
            3
        );
        startEvent->addEffect("military", 0.8);
        kingdoms[playerKingdomIdx]->addEvent(startEvent);
    }

    isRunning = true;

    std::stringstream ss;
    ss << "Welcome to the kingdom of " << kingdoms[playerKingdomIdx]->getName()
        << "! You are now the Monarch. Difficulty level: " << difficulty;
    addToLog(ss.str());
}

void GameEngine::mainLoop() {
    render();
    while (isRunning && kingdoms[playerKingdomIdx]->getTurn() < maxTurns) {
        std::string input;
        std::cout << "\nEnter command (type 'help' for list of commands): ";
        std::getline(std::cin, input);
        try {
            processInput(input);
        }
        catch (const GameException& e) {
            addToLog("Error: " + e.getMessage());
        }
        catch (...) {
            addToLog("Unknown error processing command.");
        }
        if (!isRunning) {
            break;
        }
    }
    endGame();
}

void GameEngine::processInput(const std::string& input) {
    std::string lowerInput = input;
    std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(),
        [](unsigned char c) { return std::tolower(c); });
    lowerInput.erase(std::remove_if(lowerInput.begin(), lowerInput.end(), ::isspace), lowerInput.end());

    if (lowerInput == "help") {
        displayHelp();
    }
    else if (lowerInput == "next" || lowerInput == "n") {
        update();
        render();
    }
    else if (lowerInput == "status" || lowerInput == "s") {
        kingdoms[playerKingdomIdx]->displayStatus();
    }
    else if (lowerInput.substr(0, 4) == "save") {
        std::string filename = input.length() > 5 ? input.substr(5) : "stronghold_save.txt";
        filename.erase(0, filename.find_first_not_of(" \t"));
        if (filename.empty()) filename = "stronghold_save.txt";
        saveGame(filename);
    }
    else if (lowerInput.substr(0, 4) == "load") {
        std::string filename = input.length() > 5 ? input.substr(5) : "stronghold_save.txt";
        filename.erase(0, filename.find_first_not_of(" \t"));
        if (filename.empty()) filename = "stronghold_save.txt";
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
                    kingdoms[playerKingdomIdx]->getEconomy().setTaxRate(taxRate);
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
                    std::to_string(kingdoms[playerKingdomIdx]->getEconomy().getTaxRate() * 100.0) + "%");
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
                int available = kingdoms[playerKingdomIdx]->getPopulation().getAvailableRecruits();
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
                    if (kingdoms[playerKingdomIdx]->getEconomy().getTreasury() < totalCost) {
                        addToLog("Not enough gold in treasury to recruit soldiers.");
                    }
                    else {
                        kingdoms[playerKingdomIdx]->getEconomy().payExpense(totalCost, "Military recruitment");
                        kingdoms[playerKingdomIdx]->getMilitary().recruit(recruitCount);
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
                    if (kingdoms[playerKingdomIdx]->getEconomy().getTreasury() < costPerLevel) {
                        addToLog("Not enough gold in treasury for military training.");
                    }
                    else {
                        kingdoms[playerKingdomIdx]->getEconomy().payExpense(costPerLevel, "Military training");
                        kingdoms[playerKingdomIdx]->getMilitary().train(trainingLevel * 10);
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
                bool success = kingdoms[playerKingdomIdx]->getEconomy().allocateBudget(category, percentage / 100.0);
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
            std::cout << "Military: " << (kingdoms[playerKingdomIdx]->getEconomy().getBudgetFor("Military") * 100) << "%" << std::endl;
            std::cout << "Infrastructure: " << (kingdoms[playerKingdomIdx]->getEconomy().getBudgetFor("Infrastructure") * 100) << "%" << std::endl;
            std::cout << "Healthcare: " << (kingdoms[playerKingdomIdx]->getEconomy().getBudgetFor("Healthcare") * 100) << "%" << std::endl;
            std::cout << "Education: " << (kingdoms[playerKingdomIdx]->getEconomy().getBudgetFor("Education") * 100) << "%" << std::endl;
            std::cout << "Reserve: " << (kingdoms[playerKingdomIdx]->getEconomy().getBudgetFor("Reserve") * 100) << "%" << std::endl;
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
                    bool success = kingdoms[playerKingdomIdx]->getBank().issueLoan("Treasury", amount, term);
                    if (success) {
                        kingdoms[playerKingdomIdx]->getEconomy().addIncome(amount, "Bank Loan");
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
        std::cout << "\n=== GAME LOG ===\n";
        for (int i = 0; i < logSize; i++) {
            if (gameLog[i]) {
                std::cout << gameLog[i] << std::endl;
            }
        }
    }
    else if (lowerInput.substr(0, 7) == "message") {
        if (input.length() > 8) {
            std::istringstream iss(input.substr(8));
            std::string receiverName, message;
            iss >> receiverName;
            std::getline(iss, message);
            message.erase(0, message.find_first_not_of(" \t"));
            int receiverIdx = -1;
            for (int i = 0; i < 3; i++) {
                if (kingdoms[i] && kingdoms[i]->getName() == receiverName) {
                    receiverIdx = i;
                    break;
                }
            }
            if (receiverIdx >= 0 && receiverIdx != playerKingdomIdx) {
                comm.sendMessage(*kingdoms[playerKingdomIdx], *kingdoms[receiverIdx], message);
                std::stringstream ss;
                ss << "Sent message to " << receiverName << ": " << message;
                addToLog(ss.str());
            }
            else {
                addToLog("Invalid receiver kingdom.");
            }
        }
        else {
            addToLog("Usage: message [receiver] [message]");
        }
    }
    else if (lowerInput == "messages") {
        comm.displayMessages(*kingdoms[playerKingdomIdx]);
    }
    else if (lowerInput.substr(0, 8) == "alliance") {
        if (input.length() > 9) {
            std::istringstream iss(input.substr(9));
            std::string allyName, treatyName;
            iss >> allyName;
            std::getline(iss, treatyName);
            treatyName.erase(0, treatyName.find_first_not_of(" \t"));
            int allyIdx = -1;
            for (int i = 0; i < 3; i++) {
                if (kingdoms[i] && kingdoms[i]->getName() == allyName) {
                    allyIdx = i;
                    break;
                }
            }
            if (allyIdx >= 0 && allyIdx != playerKingdomIdx) {
                alliance.formAlliance(*kingdoms[playerKingdomIdx], *kingdoms[allyIdx], treatyName);
                std::stringstream ss;
                ss << "Formed alliance with " << allyName << ": " << treatyName;
                addToLog(ss.str());
            }
            else {
                addToLog("Invalid ally kingdom.");
            }
        }
        else {
            addToLog("Usage: alliance [kingdom] [treaty name]");
        }
    }
    else if (lowerInput.substr(0, 12) == "breakalliance") {
        if (input.length() > 13) {
            std::string allyName = input.substr(13);
            allyName.erase(0, allyName.find_first_not_of(" \t"));
            int allyIdx = -1;
            for (int i = 0; i < 3; i++) {
                if (kingdoms[i] && kingdoms[i]->getName() == allyName) {
                    allyIdx = i;
                    break;
                }
            }
            if (allyIdx >= 0 && allyIdx != playerKingdomIdx) {
                alliance.breakAlliance(*kingdoms[playerKingdomIdx], *kingdoms[allyIdx]);
                std::stringstream ss;
                ss << "Broke alliance with " << allyName;
                addToLog(ss.str());
            }
            else {
                addToLog("Invalid ally kingdom.");
            }
        }
        else {
            addToLog("Usage: breakalliance [kingdom]");
        }
    }
    else if (lowerInput == "alliances") {
        alliance.displayAlliances();
    }
    else if (lowerInput.substr(0, 5) == "trade") {
        if (input.length() > 6) {
            std::istringstream iss(input.substr(6));
            std::string itemType;
            int quantity, goldCost;
            iss >> itemType >> quantity >> goldCost;
            if (quantity > 0 && goldCost >= 0) {
                TradeItem<int> item(itemType, quantity, goldCost * (1.0 + difficulty * 0.1));
                market.offerTrade(*kingdoms[playerKingdomIdx], item);
                std::stringstream ss;
                ss << "Offered " << quantity << " " << itemType << " for " << item.getGoldCost() << " gold";
                addToLog(ss.str());
            }
            else {
                addToLog("Invalid trade parameters.");
            }
        }
        else {
            market.displayMarket(); // Corrected from displayOffers()
        }
    }
    else if (lowerInput.substr(0, 10) == "accepttrade") {
        if (input.length() > 11) {
            std::istringstream iss(input.substr(11));
            std::string sellerName;
            int offerIdx;
            iss >> sellerName >> offerIdx;
            int sellerIdx = -1;
            for (int i = 0; i < 3; i++) {
                if (kingdoms[i] && kingdoms[i]->getName() == sellerName) {
                    sellerIdx = i;
                    break;
                }
            }
            if (sellerIdx >= 0 && sellerIdx != playerKingdomIdx) {
                if (market.acceptTrade(*kingdoms[playerKingdomIdx], *kingdoms[sellerIdx], offerIdx - 1)) {
                    std::stringstream ss;
                    ss << "Accepted trade from " << sellerName << " (offer " << offerIdx << ")";
                    addToLog(ss.str());
                }
            }
            else {
                addToLog("Invalid seller kingdom.");
            }
        }
        else {
            addToLog("Usage: accepttrade [seller] [offer number]");
        }
    }
    else if (lowerInput.substr(0, 7) == "smuggle") {
        if (input.length() > 8) {
            std::istringstream iss(input.substr(8));
            std::string receiverName, itemType;
            int quantity;
            iss >> receiverName >> itemType >> quantity;
            int receiverIdx = -1;
            for (int i = 0; i < 3; i++) {
                if (kingdoms[i] && kingdoms[i]->getName() == receiverName) {
                    receiverIdx = i;
                    break;
                }
            }
            if (receiverIdx >= 0 && receiverIdx != playerKingdomIdx && quantity > 0) {
                TradeItem<int> item(itemType, quantity, 0);
                market.smuggle(*kingdoms[playerKingdomIdx], *kingdoms[receiverIdx], item);
                std::stringstream ss;
                ss << "Smuggled " << quantity << " " << itemType << " to " << receiverName;
                addToLog(ss.str());
            }
            else {
                addToLog("Invalid smuggling parameters.");
            }
        }
        else {
            addToLog("Usage: smuggle [receiver] [item] [quantity]");
        }
    }
    else if (lowerInput.substr(0, 3) == "war") {
        if (input.length() > 4) {
            std::string defenderName = input.substr(4);
            defenderName.erase(0, defenderName.find_first_not_of(" \t"));
            int defenderIdx = -1;
            for (int i = 0; i < 3; i++) {
                if (kingdoms[i] && kingdoms[i]->getName() == defenderName) {
                    defenderIdx = i;
                    break;
                }
            }
            if (defenderIdx >= 0 && defenderIdx != playerKingdomIdx) {
                conflict.declareWar(*kingdoms[playerKingdomIdx], *kingdoms[defenderIdx]);
                std::stringstream ss;
                ss << "Declared war on " << defenderName;
                addToLog(ss.str());
            }
            else {
                addToLog("Invalid defender kingdom.");
            }
        }
        else {
            addToLog("Usage: war [kingdom]");
        }
    }
    else if (lowerInput.substr(0, 6) == "battle") {
        if (input.length() > 7) {
            std::string defenderName = input.substr(7);
            defenderName.erase(0, defenderName.find_first_not_of(" \t"));
            int defenderIdx = -1;
            for (int i = 0; i < 3; i++) {
                if (kingdoms[i] && kingdoms[i]->getName() == defenderName) {
                    defenderIdx = i;
                    break;
                }
            }
            if (defenderIdx >= 0 && defenderIdx != playerKingdomIdx) {
                conflict.resolveBattle(*kingdoms[playerKingdomIdx], *kingdoms[defenderIdx]);
                std::stringstream ss;
                ss << "Resolved battle with " << defenderName;
                addToLog(ss.str());
            }
            else {
                addToLog("Invalid defender kingdom.");
            }
        }
        else {
            addToLog("Usage: battle [kingdom]");
        }
    }
    else if (lowerInput.substr(0, 6) == "betray") {
        if (input.length() > 7) {
            std::string betrayedName = input.substr(7);
            betrayedName.erase(0, betrayedName.find_first_not_of(" \t"));
            int betrayedIdx = -1;
            for (int i = 0; i < 3; i++) {
                if (kingdoms[i] && kingdoms[i]->getName() == betrayedName) {
                    betrayedIdx = i;
                    break;
                }
            }
            if (betrayedIdx >= 0 && betrayedIdx != playerKingdomIdx) {
                conflict.betrayAlly(*kingdoms[playerKingdomIdx], *kingdoms[betrayedIdx]);
                std::stringstream ss;
                ss << "Betrayed ally " << betrayedName;
                addToLog(ss.str());
            }
            else {
                addToLog("Invalid ally kingdom.");
            }
        }
        else {
            addToLog("Usage: betray [kingdom]");
        }
    }
    else if (lowerInput.substr(0, 4) == "move") {
        if (input.length() > 5) {
            std::istringstream iss(input.substr(5));
            int x, y;
            if (iss >> x >> y) {
                if (x >= 0 && x < 10 && y >= 0 && y < 10) {
                    gameMap.moveKingdom(*kingdoms[playerKingdomIdx], x, y);
                    std::stringstream ss;
                    ss << "Moved to (" << x << ", " << y << ")";
                    addToLog(ss.str());
                }
                else {
                    addToLog("Invalid coordinates. Use 0-9 for x and y.");
                }
            }
            else {
                addToLog("Invalid input format. Use 'move [x] [y]'");
            }
        }
        else {
            addToLog("Usage: move [x] [y]");
        }
    }
    else if (lowerInput == "map") {
        gameMap.displayMap();
    }
    else {
        addToLog("Unknown command. Type 'help' for list of commands.");
    }
}

void GameEngine::update() {
    for (int i = 0; i < 3; i++) {
        if (kingdoms[i]) {
            kingdoms[i]->simulateTurn();
        }
    }
    addToLog("Turn " + std::to_string(kingdoms[playerKingdomIdx]->getTurn()) + " completed.");

    if (kingdoms[playerKingdomIdx]->getTurn() >= maxTurns) {
        addToLog("Maximum number of turns reached.");
        isRunning = false;
    }

    if (!kingdoms[playerKingdomIdx]->isStable()) {
        addToLog("Your kingdom has fallen into chaos. Game over.");
        isRunning = false;
    }
}

void GameEngine::render() {
    std::cout << "\n===================================" << std::endl;
    std::cout << "KINGDOM OF " << kingdoms[playerKingdomIdx]->getName() << " - TURN " << kingdoms[playerKingdomIdx]->getTurn() << std::endl;
    std::cout << "===================================" << std::endl;

    kingdoms[playerKingdomIdx]->displayStatus();

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

    kingdoms[playerKingdomIdx]->displayStatus();

    double stabilityScore = kingdoms[playerKingdomIdx]->calculateStabilityIndex() * 100;
    int populationScore = kingdoms[playerKingdomIdx]->getPopulation().getTotalPopulation() / 100;
    double treasuryScore = kingdoms[playerKingdomIdx]->getEconomy().getTreasury() / 1000;
    int militaryScore = kingdoms[playerKingdomIdx]->getMilitary().getSoldiers() *
        kingdoms[playerKingdomIdx]->getMilitary().getTraining() / 100;

    int finalScore = static_cast<int>(stabilityScore + populationScore + treasuryScore + militaryScore);

    std::cout << "\nFINAL SCORE: " << finalScore << std::endl;
    std::cout << "Difficulty Level: " << difficulty << std::endl;
    std::cout << "Turns Survived: " << kingdoms[playerKingdomIdx]->getTurn() << " of " << maxTurns << std::endl;

    kingdoms[playerKingdomIdx]->logScore();

    std::cout << "\nThank you for playing Stronghold!" << std::endl;
}

void GameEngine::saveGame(const std::string& filename) const {
    bool success = kingdoms[playerKingdomIdx]->saveGame(filename);
    if (success) {
        std::cout << "Game saved successfully to " << filename << std::endl;
        comm.saveChatLog("chat_log.txt");
        alliance.saveTreatyLog("treaty_log.txt");
        conflict.saveConflictLog("conflict_log.txt");
    }
    else {
        std::cout << "Failed to save game to " << filename << std::endl;
    }
}

bool GameEngine::loadGame(const std::string& filename) {
    bool success = kingdoms[playerKingdomIdx]->loadGame(filename);
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
    std::cout << "next, n            - Advance to the next turn" << std::endl;
    std::cout << "status, s          - Display current kingdom status" << std::endl;
    std::cout << "tax [rate]         - Set tax rate (0-100 percent)" << std::endl;
    std::cout << "recruit [num]      - Recruit soldiers to your army" << std::endl;
    std::cout << "train [1-10]       - Train your military (levels 1-10)" << std::endl;
    std::cout << "budget             - View current budget allocations" << std::endl;
    std::cout << "budget [cat] [pct] - Set budget allocation for category" << std::endl;
    std::cout << "loan [amt] [term]  - Request a loan from the bank" << std::endl;
    std::cout << "log                - Display the game log" << std::endl;
    std::cout << "save [file]        - Save game (default: stronghold_save.txt)" << std::endl;
    std::cout << "load [file]        - Load game (default: stronghold_save.txt)" << std::endl;
    std::cout << "message [kingdom] [msg] - Send a message to another kingdom" << std::endl;
    std::cout << "messages           - Display all messages" << std::endl;
    std::cout << "alliance [kingdom] [treaty] - Form an alliance" << std::endl;
    std::cout << "breakalliance [kingdom] - Break an alliance" << std::endl;
    std::cout << "alliances          - Display all alliances" << std::endl;
    std::cout << "trade              - Display market offers" << std::endl;
    std::cout << "trade [item] [qty] [gold] - Offer a trade" << std::endl;
    std::cout << "accepttrade [seller] [num] - Accept a trade offer" << std::endl;
    std::cout << "smuggle [receiver] [item] [qty] - Smuggle resources" << std::endl;
    std::cout << "war [kingdom]      - Declare war on a kingdom" << std::endl;
    std::cout << "battle [kingdom]   - Fight a battle" << std::endl;
    std::cout << "betray [kingdom]   - Betray an ally" << std::endl;
    std::cout << "move [x] [y]       - Move kingdom on map" << std::endl;
    std::cout << "map                - Display the game map" << std::endl;
    std::cout << "quit, exit, q      - Exit the game" << std::endl;
    std::cout << "help               - Display this help menu" << std::endl;
}

void GameEngine::addToLog(const std::string& message) {
    if (logSize >= maxLogSize) {
        delete[] gameLog[0];
        for (int i = 1; i < maxLogSize; i++) {
            gameLog[i - 1] = gameLog[i];
        }
        gameLog[maxLogSize - 1] = nullptr;
        logSize = maxLogSize - 1;
    }

    size_t msgLength = message.length();
    gameLog[logSize] = new char[msgLength + 1];
    if (gameLog[logSize]) {
        strncpy(gameLog[logSize], message.c_str(), msgLength + 1);
    }
    else {
        std::cout << "Memory allocation failed for log entry." << std::endl;
        return;
    }

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
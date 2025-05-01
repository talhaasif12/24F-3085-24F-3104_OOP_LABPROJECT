#define _CRT_SECURE_NO_WARNINGS
#include "Stronghold.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>

Kingdom::Kingdom(const std::string& name)
    : name(name), population(nullptr), economy(nullptr), military(nullptr),
    bank(nullptr), currentLeader(nullptr), numEvents(0), maxEvents(10), turn(0) {

    activeEvents = new Event * [maxEvents];
    for (int i = 0; i < maxEvents; i++) {
        activeEvents[i] = nullptr;
    }
}

Kingdom::~Kingdom() {
    if (population) {
        delete population;
    }

    if (economy) {
        delete economy;
    }

    if (military) {
        delete military;
    }

    if (bank) {
        delete bank;
    }

    if (currentLeader) {
        delete currentLeader;
    }

    for (auto& pair : resources) {
        if (pair.second) {
            delete pair.second;
        }
    }

    for (int i = 0; i < maxEvents; i++) {
        if (activeEvents[i]) {
            delete activeEvents[i];
        }
    }
    delete[] activeEvents;
}

std::string Kingdom::getName() const {
    return name;
}

Population& Kingdom::getPopulation() {
    return *population;
}

Economy& Kingdom::getEconomy() {
    return *economy;
}

Military& Kingdom::getMilitary() {
    return *military;
}

Bank& Kingdom::getBank() {
    return *bank;
}

Leader* Kingdom::getLeader() const {
    return currentLeader;
}

Resource& Kingdom::getResource(const std::string& name) {
    if (resources.find(name) == resources.end()) {
        throw std::runtime_error("Resource not found: " + name);
    }
    return *resources[name];
}

int Kingdom::getTurn() const {
    return turn;
}

void Kingdom::initialize() {
    population = new Population(1000);

    population->addSocialClass(SocialClass("Nobles", 50, 0.05, 0.8, 0.5));
    population->addSocialClass(SocialClass("Merchants", 150, 0.1, 0.7, 1.2));
    population->addSocialClass(SocialClass("Craftsmen", 300, 0.15, 0.6, 1.0));
    population->addSocialClass(SocialClass("Farmers", 500, 0.2, 0.5, 0.8));

    economy = new Economy(1000.0, 0.1);

    economy->allocateBudget("Military", 0.4);
    economy->allocateBudget("Infrastructure", 0.2);
    economy->allocateBudget("Healthcare", 0.15);
    economy->allocateBudget("Education", 0.15);
    economy->allocateBudget("Reserve", 0.1);

    military = new Military(100, 50, 70);

    bank = new Bank(5000.0, 0.05);

    Resource* food = new Resource("Food", 1000, 200, 150, 1.0);
    Resource* wood = new Resource("Wood", 500, 100, 80, 0.8);
    Resource* stone = new Resource("Stone", 300, 40, 30, 2.0);
    Resource* iron = new Resource("Iron", 200, 20, 15, 4.0);
    Resource* gold = new Resource("Gold", 100, 5, 2, 20.0);

    addResource(food);
    addResource(wood);
    addResource(stone);
    addResource(iron);
    addResource(gold);

    population->getSocialClass("Nobles").addResourceConsumption("Food", 3.0);
    population->getSocialClass("Merchants").addResourceConsumption("Food", 2.0);
    population->getSocialClass("Craftsmen").addResourceConsumption("Food", 1.5);
    population->getSocialClass("Farmers").addResourceConsumption("Food", 1.0);

    turn = 1;
}

void Kingdom::simulateTurn() {
    processEvents();

    produceResources();

    economy->collectTaxes(*population);

    bank->processLoanPayments(*economy);

    military->updateMorale(economy->getBudgetFor("Military") * economy->getTreasury(),
        resources["Food"]->getAmount());

    int foodForArmy = static_cast<int>(military->getSoldiers() * 0.5);
    resources["Food"]->consume(foodForArmy);

    consumeResources();

    updateResourceValues();

    if (currentLeader) {
        currentLeader->updatePopularity(*population);
        currentLeader->incrementMonthsInPower();
    }

    bool isFamine = (resources["Food"]->getAmount() < (population->getTotalPopulation() * 0.3));
    bool isDisease = (population->getHealthLevel() < 0.3);
    bool isWar = (military->getMorale() < 30);

    population->updatePopulation(isFamine, isDisease, isWar);

    checkRebellion();
    checkLeadershipChange();

    if ((rand() % 100) < 10) {
        Event* newEvent = new Event(Event::generateRandomEvent());
        addEvent(newEvent);
    }

    turn++;
}

void Kingdom::addResource(Resource* resource) {
    if (resource) {
        resources[resource->getName()] = resource;
    }
}

void Kingdom::setLeader(Leader* leader) {
    if (currentLeader) {
        delete currentLeader;
    }
    currentLeader = leader;
}

void Kingdom::addEvent(Event* event) {
    if (!event) {
        return;
    }

    for (int i = 0; i < maxEvents; i++) {
        if (activeEvents[i] == nullptr) {
            activeEvents[i] = event;
            numEvents++;

            std::cout << "Event occurred: " << event->getName() << " - "
                << event->getDescription() << std::endl;
            return;
        }
    }

    if (numEvents >= maxEvents) {
        delete activeEvents[0];

        for (int i = 1; i < maxEvents; i++) {
            activeEvents[i - 1] = activeEvents[i];
        }

        activeEvents[maxEvents - 1] = event;

        std::cout << "Event occurred: " << event->getName() << " - "
            << event->getDescription() << std::endl;
    }
}

void Kingdom::processEvents() {
    for (int i = 0; i < maxEvents; i++) {
        if (activeEvents[i] && activeEvents[i]->isActive()) {
            activeEvents[i]->applyEffects(*this);

            activeEvents[i]->incrementDuration();

            if (!activeEvents[i]->isActive()) {
                std::cout << "Event ended: " << activeEvents[i]->getName() << std::endl;
                delete activeEvents[i];
                activeEvents[i] = nullptr;
                numEvents--;
            }
        }
    }
}

void Kingdom::produceResources() {
    for (auto& pair : resources) {
        if (pair.second) {
            pair.second->produce();
        }
    }
}

void Kingdom::consumeResources() {
    for (auto& pair : resources) {
        if (pair.second) {
            int baseConsumption = pair.second->getConsumptionRate();

            double populationFactor = static_cast<double>(population->getTotalPopulation()) / 1000.0;
            int totalConsumption = static_cast<int>(baseConsumption * populationFactor);

            pair.second->consume(totalConsumption);
        }
    }
}

void Kingdom::updateResourceValues() {
    double inflationFactor = 1.0 + economy->getInflationRate();

    for (auto& pair : resources) {
        if (pair.second) {
            double demand = pair.second->getConsumptionRate() *
                (static_cast<double>(population->getTotalPopulation()) / 1000.0);
            double supply = pair.second->getProductionRate();

            double scarcityFactor = demand / (supply + 0.001);

            double valueChange = (inflationFactor * scarcityFactor) - 1.0;
            valueChange = std::max(-0.1, std::min(valueChange, 0.1));

            double newValue = pair.second->getMarketValue() * (1.0 + valueChange);
            pair.second->setMarketValue(newValue);
        }
    }
}

void Kingdom::checkRebellion() {
    if (population->checkRebellion()) {
        std::cout << "REBELLION! The people are revolting against your rule!" << std::endl;

        if (currentLeader) {
            bool success = currentLeader->handleRebellion(*population, *military);

            if (!success) {
                std::cout << "The rebellion has overthrown " << currentLeader->getName() << "!" << std::endl;

                delete currentLeader;
                currentLeader = new Leader("Rebel Leader", "Usurper");

                currentLeader->setCharisma(40 + (rand() % 40));
                currentLeader->setIntelligence(40 + (rand() % 40));
                currentLeader->setLeadership(40 + (rand() % 40));
                currentLeader->setCorruption(10 + (rand() % 50));

                economy->setTreasury(economy->getTreasury() * 0.7);

                military->setSoldiers(military->getSoldiers() / 2);
                military->setMorale(30);
            }
        }
    }
}

void Kingdom::checkLeadershipChange() {
    if (currentLeader && currentLeader->isCoupLikely(*military, *population)) {
        std::cout << "MILITARY COUP! The army has overthrown " << currentLeader->getName() << "!" << std::endl;

        delete currentLeader;
        currentLeader = new Leader("General", "Military Dictator");

        currentLeader->setCharisma(30 + (rand() % 30));
        currentLeader->setIntelligence(40 + (rand() % 40));
        currentLeader->setLeadership(60 + (rand() % 40));
        currentLeader->setCorruption(20 + (rand() % 40));

        military->setMorale(90);

        for (int i = 0; i < population->getSocialClassCount(); i++) {
            SocialClass& socialClass = population->getSocialClassByIndex(i);
            socialClass.setHappiness(socialClass.getHappiness() * 0.8);
        }
    }
}

bool Kingdom::isStable() const {
    double stabilityIndex = calculateStabilityIndex();
    return stabilityIndex > 0.3;
}

double Kingdom::calculateStabilityIndex() const {
    double populationHappiness = population->getOverallHappiness();
    double militaryMorale = static_cast<double>(military->getMorale()) / 100.0;
    double economicHealth = std::min(1.0, economy->getTreasury() / 5000.0);

    return (populationHappiness * 0.5) + (militaryMorale * 0.3) + (economicHealth * 0.2);
}

void Kingdom::displayStatus() const {
    std::cout << "\n--- KINGDOM STATUS ---" << std::endl;

    std::cout << "Turn: " << turn << std::endl;

    if (currentLeader) {
        std::cout << "\nLeader: " << currentLeader->getName() << ", " << currentLeader->getTitle() << std::endl;
        std::cout << "  Popularity: " << static_cast<int>(currentLeader->getPopularity() * 100) << "%" << std::endl;
        std::cout << "  Months in Power: " << currentLeader->getMonthsInPower() << std::endl;
    }

    std::cout << "\nPopulation: " << population->getTotalPopulation() << std::endl;
    std::cout << "  Happiness: " << static_cast<int>(population->getOverallHappiness() * 100) << "%" << std::endl;

    std::cout << "\nResources:" << std::endl;
    for (const auto& pair : resources) {
        std::cout << "  " << std::left << std::setw(10) << pair.first << ": "
            << std::setw(6) << pair.second->getAmount()
            << " (+" << pair.second->getProductionRate() << "/-" << pair.second->getConsumptionRate() << ")"
            << " Value: " << std::fixed << std::setprecision(2) << pair.second->getMarketValue() << std::endl;
    }

    std::cout << "\nEconomy:" << std::endl;
    std::cout << "  Treasury: " << std::fixed << std::setprecision(2) << economy->getTreasury() << " gold" << std::endl;
    std::cout << "  Tax Rate: " << (economy->getTaxRate() * 100) << "%" << std::endl;
    std::cout << "  Inflation: " << (economy->getInflationRate() * 100) << "%" << std::endl;

    std::cout << "\nMilitary:" << std::endl;
    std::cout << "  Soldiers: " << military->getSoldiers() << std::endl;
    std::cout << "  Training: " << military->getTraining() << "/100" << std::endl;
    std::cout << "  Morale: " << military->getMorale() << "/100" << std::endl;

    bool hasEvents = false;
    std::cout << "\nActive Events:" << std::endl;
    for (int i = 0; i < maxEvents; i++) {
        if (activeEvents[i] && activeEvents[i]->isActive()) {
            std::cout << "  " << activeEvents[i]->getName() << " - "
                << activeEvents[i]->getDescription() << " ("
                << activeEvents[i]->getDuration() - activeEvents[i]->getCurrentDuration()
                << " turns remaining)" << std::endl;
            hasEvents = true;
        }
    }
    if (!hasEvents) {
        std::cout << "  None" << std::endl;
    }

    double stability = calculateStabilityIndex() * 100;
    std::cout << "\nKingdom Stability: " << static_cast<int>(stability) << "%" << std::endl;

    if (stability < 40) {
        std::cout << "  WARNING: Kingdom stability is dangerously low!" << std::endl;
    }
}

bool Kingdom::saveGame(const std::string& filename) const {
    try {
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile.is_open()) {
            return false;
        }

        size_t nameLength = name.length();
        outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
        outFile.write(name.c_str(), nameLength);
        outFile.write(reinterpret_cast<const char*>(&turn), sizeof(turn));

        size_t resourceCount = resources.size();
        outFile.write(reinterpret_cast<const char*>(&resourceCount), sizeof(resourceCount));
        for (const auto& pair : resources) {
        }

        outFile.write(reinterpret_cast<const char*>(&numEvents), sizeof(numEvents));
        for (int i = 0; i < maxEvents; i++) {
            if (activeEvents[i] && activeEvents[i]->isActive()) {
            }
        }

        outFile.close();
        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error saving game: " << ex.what() << std::endl;
        return false;
    }
}

bool Kingdom::loadGame(const std::string& filename) {
    try {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile.is_open()) {
            return false;
        }

        size_t nameLength;
        inFile.read(reinterpret_cast<char*>(&nameLength), sizeof(nameLength));
        char* nameBuffer = new char[nameLength + 1];
        inFile.read(nameBuffer, nameLength);
        nameBuffer[nameLength] = '\0';
        name = nameBuffer;
        delete[] nameBuffer;

        inFile.read(reinterpret_cast<char*>(&turn), sizeof(turn));

        size_t resourceCount;
        inFile.read(reinterpret_cast<char*>(&resourceCount), sizeof(resourceCount));
        for (size_t i = 0; i < resourceCount; i++) {
        }

        inFile.read(reinterpret_cast<char*>(&numEvents), sizeof(numEvents));
        for (int i = 0; i < numEvents; i++) {
        }

        inFile.close();
        return true;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error loading game: " << ex.what() << std::endl;
        return false;
    }
}

void Kingdom::logScore() const {
    try {
        std::ofstream scoreFile("stronghold_scores.txt", std::ios::app);
        if (scoreFile.is_open()) {
            time_t now = time(nullptr);
            tm* timeInfo = localtime(&now);
            char timeBuffer[80];
            strftime(timeBuffer, 80, "%Y-%m-%d %H:%M:%S", timeInfo);

            double stabilityScore = calculateStabilityIndex() * 100;
            int populationScore = population->getTotalPopulation() / 100;
            double treasuryScore = economy->getTreasury() / 1000;
            int militaryScore = military->getSoldiers() * military->getTraining() / 100;

            int finalScore = static_cast<int>(stabilityScore + populationScore + treasuryScore + militaryScore);

            scoreFile << timeBuffer << " | "
                << name << " | "
                << "Score: " << finalScore << " | "
                << "Turns: " << turn << " | "
                << "Population: " << population->getTotalPopulation() << " | "
                << "Treasury: " << economy->getTreasury() << std::endl;

            scoreFile.close();
        }
    }
    catch (const std::exception& ex) {
        std::cerr << "Error logging score: " << ex.what() << std::endl;
    }
}
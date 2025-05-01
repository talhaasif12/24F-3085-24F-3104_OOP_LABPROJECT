#ifndef STRONGHOLD_H
#define STRONGHOLD_H

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <stdexcept>
#include <sstream>

class Resource;
class Population;
class SocialClass;
class Economy;
class Bank;
class Military;
class Leader;
class Event;
class Kingdom;

class Resource {
private:
    std::string name;
    int amount;
    int productionRate;
    int consumptionRate;
    double marketValue;

public:
    Resource();
    Resource(const std::string& name, int initialAmount = 0, int prodRate = 0, int consRate = 0, double value = 1.0);
    ~Resource();

    std::string getName() const;
    int getAmount() const;
    void setAmount(int newAmount);
    int getProductionRate() const;
    void setProductionRate(int rate);
    int getConsumptionRate() const;
    void setConsumptionRate(int rate);
    double getMarketValue() const;
    void setMarketValue(double value);

    void produce();
    bool consume(int amount);
    void trade(Resource& other, int amount, double exchangeRate);
    void updateValue(double inflationRate);

    void saveToFile(std::ofstream& outFile) const;
    void loadFromFile(std::ifstream& inFile);
};

class SocialClass {
private:
    std::string name;
    int population;
    double taxRate;
    double happiness;
    double productivity;
    double recruitmentRate;
    std::map<std::string, double> resourceConsumption;

public:
    SocialClass(const std::string& name, int pop = 0, double tax = 0.1, double happy = 0.5, double prod = 1.0);
    ~SocialClass();

    std::string getName() const;
    int getPopulation() const;
    void setPopulation(int pop);
    double getTaxRate() const;
    void setTaxRate(double rate);
    double getHappiness() const;
    void setHappiness(double happy);
    double getProductivity() const;
    void setProductivity(double prod);
    double getRecruitmentRate() const;
    void setRecruitmentRate(double rate);

    void addResourceConsumption(const std::string& resource, double amount);
    double getResourceConsumption(const std::string& resource) const;
    int calculateTaxRevenue(double baseTaxValue) const;
    void updateHappiness(double foodSatisfaction, double safetySatisfaction, double wealthSatisfaction);
    bool checkRebellion() const;
    int getAvailableRecruits() const;

    void saveToFile(std::ofstream& outFile) const;
    void loadFromFile(std::ifstream& inFile);
};

class Population {
private:
    int totalPopulation;
    double growthRate;
    double deathRate;
    double migrationRate;
    double healthLevel;
    SocialClass** socialClasses;
    int numSocialClasses;
    int maxSocialClasses;

public:
    Population(int initialPop = 1000);
    ~Population();

    int getTotalPopulation() const;
    double getGrowthRate() const;
    void setGrowthRate(double rate);
    double getDeathRate() const;
    void setDeathRate(double rate);
    double getHealthLevel() const;
    void setHealthLevel(double level);

    void addSocialClass(const SocialClass& socialClass);
    SocialClass& getSocialClass(const std::string& name);
    SocialClass& getSocialClassByIndex(int index) const;
    int getSocialClassCount() const;
    void updatePopulation(bool isFamine, bool isDisease, bool isWar);
    double getOverallHappiness() const;
    void distributeResources(std::map<std::string, Resource>& resources);
    bool checkRebellion() const;
    int getAvailableRecruits() const;
    void updateSocialClassHappiness(double foodSatisfaction, double safetySatisfaction, double wealthSatisfaction);

    void saveToFile(std::ofstream& outFile) const;
    void loadFromFile(std::ifstream& inFile);
};

class Military {
private:
    int soldiers;
    int training;
    int morale;
    int casualties;
    double maintenanceCost;
    double corruptionLevel;
    int victories;
    int defeats;

public:
    Military(int initialSoldiers = 0, int initialTraining = 50, int initialMorale = 50);
    ~Military();

    int getSoldiers() const;
    void setSoldiers(int amount);
    int getTraining() const;
    void setTraining(int level);
    int getMorale() const;
    void setMorale(int level);
    double getMaintenanceCost() const;
    void setMaintenanceCost(double cost);
    double getCorruptionLevel() const;
    void setCorruptionLevel(double level);

    void recruit(int newSoldiers);
    void train(int resources);
    bool battle(Military& enemyArmy);
    void updateMorale(double payment, double food);
    int calculateStrength() const;
    void payArmy(double amount);
    void feedArmy(int foodUnits);
    void updateCorruption(double leaderInfluence);

    void saveToFile(std::ofstream& outFile) const;
    void loadFromFile(std::ifstream& inFile);
};

class Economy {
private:
    double treasury;
    double taxRate;
    double inflationRate;
    double publicServiceCost;
    double tradeBalance;
    std::map<std::string, double> budgetAllocation;

public:
    Economy(double initialFunds = 1000.0, double initTaxRate = 0.1);
    ~Economy();

    double getTreasury() const;
    void setTreasury(double amount);
    double getTaxRate() const;
    void setTaxRate(double rate);
    double getInflationRate() const;
    void setInflationRate(double rate);
    double getPublicServiceCost() const;
    void setPublicServiceCost(double cost);

    void collectTaxes(const Population& population);
    bool allocateBudget(const std::string& category, double percentage);
    double getBudgetFor(const std::string& category) const;
    void updateInflation(double marketActivity, double resourceScarcity);
    bool payExpense(double amount, const std::string& reason);
    void addIncome(double amount, const std::string& source);
    void updateTradeBalance(double exports, double imports);

    void saveToFile(std::ofstream& outFile) const;
    void loadFromFile(std::ifstream& inFile);
};

class Bank {
private:
    double reserves;
    double interestRate;
    double fraudLevel;

    struct Loan {
        std::string borrower;
        double amount;
        double interestRate;
        int termMonths;
        int monthsPaid;
        bool defaulted;
    };

    Loan** loans;
    int numLoans;
    int maxLoans;

public:
    Bank(double initialReserves = 5000.0, double initInterestRate = 0.05);
    ~Bank();

    double getReserves() const;
    void setReserves(double amount);
    double getInterestRate() const;
    void setInterestRate(double rate);
    double getFraudLevel() const;
    void setFraudLevel(double level);

    bool issueLoan(const std::string& borrower, double amount, int termMonths);
    double calculateMonthlyPayment(const Loan& loan) const;
    void processLoanPayments(Economy& economy);
    void auditBank(double thoroughness);
    void updateInterestRate(double economicStability);
    double getTotalLoans() const;
    void handleFraud(double leaderCorruptionInfluence);

    void saveToFile(std::ofstream& outFile) const;
    void loadFromFile(std::ifstream& inFile);
};

class Leader {
private:
    std::string name;
    std::string title;
    int charisma;
    int intelligence;
    int corruption;
    int leadership;
    double popularity;
    std::string leadershipStyle;
    int monthsInPower;

public:
    Leader(const std::string& name, const std::string& title);
    ~Leader();

    std::string getName() const;
    std::string getTitle() const;
    void setTitle(const std::string& newTitle);
    int getCharisma() const;
    void setCharisma(int value);
    int getIntelligence() const;
    void setIntelligence(int value);
    int getCorruption() const;
    void setCorruption(int value);
    int getLeadership() const;
    void setLeadership(int value);
    double getPopularity() const;
    void setPopularity(double value);
    std::string getLeadershipStyle() const;
    void setLeadershipStyle(const std::string& style);
    int getMonthsInPower() const;
    void incrementMonthsInPower();

    void makeTaxDecision(Economy& economy, const Population& population);
    void makeWarDecision(Military& military, Economy& economy, bool isUnderThreat);
    void handleCrisis(const std::string& crisisType, Kingdom& kingdom);
    bool handleRebellion(const Population& population, Military& military);
    void updatePopularity(const Population& population);
    bool isCoupLikely(const Military& military, const Population& population) const;

    void saveToFile(std::ofstream& outFile) const;
    void loadFromFile(std::ifstream& inFile);
};

class Event {
private:
    std::string name;
    std::string description;
    std::string type;
    int duration;
    int currentDuration;
    std::map<std::string, double> effects;

public:
    Event(const std::string& name, const std::string& desc, const std::string& type, int duration);
    ~Event();

    std::string getName() const;
    std::string getDescription() const;
    std::string getType() const;
    int getDuration() const;
    int getCurrentDuration() const;
    void incrementDuration();
    bool isActive() const;

    void addEffect(const std::string& target, double multiplier);
    double getEffect(const std::string& target) const;
    void applyEffects(Kingdom& kingdom);

    static Event generateRandomEvent();
    static Event generateDisaster();
    static Event generateEconomicEvent();
    static Event generateMilitaryEvent();
    static Event generatePoliticalEvent();

    void saveToFile(std::ofstream& outFile) const;
    void loadFromFile(std::ifstream& inFile);
};

class Kingdom {
private:
    std::string name;
    Population* population;
    Economy* economy;
    Military* military;
    Bank* bank;
    Leader* currentLeader;
    std::map<std::string, Resource*> resources;
    Event** activeEvents;
    int numEvents;
    int maxEvents;
    int turn;

public:
    Kingdom(const std::string& name);
    ~Kingdom();

    std::string getName() const;
    Population& getPopulation();
    Economy& getEconomy();
    Military& getMilitary();
    Bank& getBank();
    Leader* getLeader() const;
    Resource& getResource(const std::string& name);
    int getTurn() const;

    void initialize();
    void simulateTurn();
    void addResource(Resource* resource);
    void setLeader(Leader* leader);
    void addEvent(Event* event);
    void processEvents();
    void produceResources();
    void consumeResources();
    void updateResourceValues();
    void checkRebellion();
    void checkLeadershipChange();
    bool isStable() const;
    double calculateStabilityIndex() const;
    void displayStatus() const;

    bool saveGame(const std::string& filename) const;
    bool loadGame(const std::string& filename);
    void logScore() const;
};

class GameEngine {
private:
    Kingdom* playerKingdom;
    char** gameLog;
    int logSize;
    int maxLogSize;
    int gameDifficulty;
    int maxTurns;
    bool isRunning;

public:
    GameEngine(int difficulty = 1, int turns = 100);
    ~GameEngine();

    void initialize();
    void mainLoop();
    void processInput(const std::string& input);
    void update();
    void render();
    void endGame();
    void saveGame(const std::string& filename) const;
    bool loadGame(const std::string& filename);
    void displayHelp() const;
    void addToLog(const std::string& message);

    bool running() const;
    void quit();
};

#endif
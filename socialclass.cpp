#include "Stronghold.h"
#include <stdexcept>
#include <string>
#include <map>
#include <fstream>
#include <cstdlib>

SocialClass::SocialClass(const std::string& name, int pop, double tax, double happy, double prod)
    : name(name), population(pop), taxRate(tax), happiness(happy), productivity(prod), recruitmentRate(0.05) {
}

SocialClass::~SocialClass() {}

std::string SocialClass::getName() const {
    return name;
}

int SocialClass::getPopulation() const {
    return population;
}

void SocialClass::setPopulation(int pop) {
    if (pop < 0) throw std::invalid_argument("Population cannot be negative");
    population = pop;
}

double SocialClass::getTaxRate() const {
    return taxRate;
}

void SocialClass::setTaxRate(double rate) {
    if (rate < 0 || rate > 1) throw std::invalid_argument("Tax rate must be between 0 and 1");
    taxRate = rate;
}

double SocialClass::getHappiness() const {
    return happiness;
}

void SocialClass::setHappiness(double happy) {
    if (happy < 0 || happy > 1) throw std::invalid_argument("Happiness must be between 0 and 1");
    happiness = happy;
}

double SocialClass::getProductivity() const {
    return productivity;
}

void SocialClass::setProductivity(double prod) {
    if (prod < 0) throw std::invalid_argument("Productivity cannot be negative");
    productivity = prod;
}

double SocialClass::getRecruitmentRate() const {
    return recruitmentRate;
}

void SocialClass::setRecruitmentRate(double rate) {
    if (rate < 0 || rate > 1) throw std::invalid_argument("Recruitment rate must be between 0 and 1");
    recruitmentRate = rate;
}

void SocialClass::addResourceConsumption(const std::string& resource, double amount) {
    if (amount < 0) throw std::invalid_argument("Resource consumption cannot be negative");
    resourceConsumption[resource] = amount;
}

double SocialClass::getResourceConsumption(const std::string& resource) const {
    auto it = resourceConsumption.find(resource);
    if (it != resourceConsumption.end()) {
        return it->second;
    }
    return 0.0;
}

int SocialClass::calculateTaxRevenue(double baseTaxValue) const {
    return static_cast<int>(population * taxRate * baseTaxValue * productivity);
}

void SocialClass::updateHappiness(double foodSatisfaction, double safetySatisfaction, double wealthSatisfaction) {
    double weightFood, weightSafety, weightWealth;
    if (name == "Peasants") {
        weightFood = 0.7; weightSafety = 0.2; weightWealth = 0.1;
    }
    else if (name == "Merchants") {
        weightFood = 0.3; weightSafety = 0.3; weightWealth = 0.4;
    }
    else if (name == "Nobles") {
        weightFood = 0.1; weightSafety = 0.4; weightWealth = 0.5;
    }
    else {
        weightFood = 0.33; weightSafety = 0.33; weightWealth = 0.34;
    }
    double newHappiness = (foodSatisfaction * weightFood) +
        (safetySatisfaction * weightSafety) +
        (wealthSatisfaction * weightWealth);
    if (newHappiness > 1.0) newHappiness = 1.0;
    else if (newHappiness < 0.0) newHappiness = 0.0;
    happiness = happiness * 0.7 + newHappiness * 0.3;
}

bool SocialClass::checkRebellion() const {
    double rebellionChance = (1.0 - happiness) * (1.0 - happiness) * 0.5;
    if (name == "Peasants") rebellionChance *= 0.8;
    else if (name == "Merchants") rebellionChance *= 1.0;
    else if (name == "Nobles") rebellionChance *= 1.5;
    double randomValue = static_cast<double>(rand()) / RAND_MAX;
    return randomValue < rebellionChance;
}

int SocialClass::getAvailableRecruits() const {
    return static_cast<int>(population * recruitmentRate * happiness);
}

void SocialClass::saveToFile(std::ofstream& outFile) const {
    outFile << name << "\n";
    outFile << population << "\n";
    outFile << taxRate << "\n";
    outFile << happiness << "\n";
    outFile << productivity << "\n";
    outFile << recruitmentRate << "\n";
    outFile << resourceConsumption.size() << "\n";
    for (auto it = resourceConsumption.begin(); it != resourceConsumption.end(); ++it) {
        outFile << it->first << "\n";
        outFile << it->second << "\n";
    }
}

void SocialClass::loadFromFile(std::ifstream& inFile) {
    std::string line;
    std::getline(inFile, name);
    std::getline(inFile, line); population = std::stoi(line);
    std::getline(inFile, line); taxRate = std::stod(line);
    std::getline(inFile, line); happiness = std::stod(line);
    std::getline(inFile, line); productivity = std::stod(line);
    std::getline(inFile, line); recruitmentRate = std::stod(line);
    std::getline(inFile, line);
    int numResources = std::stoi(line);
    resourceConsumption.clear();
    for (int i = 0; i < numResources; ++i) {
        std::string resourceName;
        double amount;
        std::getline(inFile, resourceName);
        std::getline(inFile, line);
        amount = std::stod(line);
        resourceConsumption[resourceName] = amount;
    }
}

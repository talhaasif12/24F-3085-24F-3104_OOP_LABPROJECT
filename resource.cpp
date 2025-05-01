#include "Stronghold.h"
Resource::Resource()
    : name(""), amount(0), productionRate(0), consumptionRate(0), marketValue(1.0) {
    
}

Resource::Resource(const std::string& name, int initialAmount, int prodRate, int consRate, double value)
    : name(name), amount(initialAmount), productionRate(prodRate),
    consumptionRate(consRate), marketValue(value) {
}

Resource::~Resource() {
    
}

std::string Resource::getName() const {
    return name;
}

int Resource::getAmount() const {
    return amount;
}

void Resource::setAmount(int newAmount) {
    if (newAmount < 0) {
        throw std::invalid_argument("Resource amount cannot be negative");
    }
    amount = newAmount;
}

int Resource::getProductionRate() const {
    return productionRate;
}

void Resource::setProductionRate(int rate) {
    productionRate = rate;
}

int Resource::getConsumptionRate() const {
    return consumptionRate;
}

void Resource::setConsumptionRate(int rate) {
    if (rate < 0) {
        throw std::invalid_argument("Consumption rate cannot be negative");
    }
    consumptionRate = rate;
}

double Resource::getMarketValue() const {
    return marketValue;
}

void Resource::setMarketValue(double value) {
    if (value <= 0) {
        throw std::invalid_argument("Market value must be positive");
    }
    marketValue = value;
}

void Resource::produce() {
    amount += productionRate;
}

bool Resource::consume(int consumeAmount) {
    if (consumeAmount > amount) {
        return false; 
    }

    amount -= consumeAmount;
    return true;
}

void Resource::trade(Resource& other, int tradeAmount, double exchangeRate) {
    if (tradeAmount <= 0) {
        throw std::invalid_argument("Trade amount must be positive");
    }

    if (tradeAmount > amount) {
        throw std::invalid_argument("Not enough resources to trade");
    }

    double value = tradeAmount * marketValue;
    double otherValue = value / exchangeRate;
    int otherAmount = static_cast<int>(otherValue / other.marketValue);

    amount -= tradeAmount;
    other.amount += otherAmount;
}

void Resource::updateValue(double inflationRate) {
    marketValue *= (1 + inflationRate);
}

void Resource::saveToFile(std::ofstream& outFile) const {
    outFile << name << "\n";
    outFile << amount << "\n";
    outFile << productionRate << "\n";
    outFile << consumptionRate << "\n";
    outFile << marketValue << "\n";
}

void Resource::loadFromFile(std::ifstream& inFile) {
    std::getline(inFile, name);

    std::string line;

    std::getline(inFile, line);
    amount = std::stoi(line);

    std::getline(inFile, line);
    productionRate = std::stoi(line);

    std::getline(inFile, line);
    consumptionRate = std::stoi(line);

    std::getline(inFile, line);
    marketValue = std::stod(line);
}
#include "Stronghold.h"

Economy::Economy(double initialFunds, double initTaxRate)
    : treasury(initialFunds), taxRate(initTaxRate), inflationRate(0.01),
    publicServiceCost(100.0), tradeBalance(0.0) {

    budgetAllocation["Military"] = 0.3;
    budgetAllocation["Infrastructure"] = 0.2;
    budgetAllocation["Healthcare"] = 0.15;
    budgetAllocation["Education"] = 0.15;
    budgetAllocation["Reserve"] = 0.2;
}

Economy::~Economy() {
}

double Economy::getTreasury() const {
    return treasury;
}

void Economy::setTreasury(double amount) {
    if (amount < 0) {
        throw std::invalid_argument("Treasury cannot be negative");
    }
    treasury = amount;
}

double Economy::getTaxRate() const {
    return taxRate;
}

void Economy::setTaxRate(double rate) {
    if (rate < 0 || rate > 1) {
        throw std::invalid_argument("Tax rate must be between 0 and 1");
    }
    taxRate = rate;
}

double Economy::getInflationRate() const {
    return inflationRate;
}

void Economy::setInflationRate(double rate) {
    if (rate < -0.05 || rate > 0.3) {
        throw std::invalid_argument("Inflation rate must be between -5% and 30%");
    }
    inflationRate = rate;
}

double Economy::getPublicServiceCost() const {
    return publicServiceCost;
}

void Economy::setPublicServiceCost(double cost) {
    if (cost < 0) {
        throw std::invalid_argument("Public service cost cannot be negative");
    }
    publicServiceCost = cost;
}

void Economy::collectTaxes(const Population& population) {
    double totalTax = 0.0;
    double baseTaxValue = 10.0 * (1.0 + inflationRate);
    for (int i = 0; i < population.getSocialClassCount(); ++i) {
        try {
            double classTax = population.getSocialClassByIndex(i).calculateTaxRevenue(baseTaxValue);
            totalTax += classTax;
        }
        catch (const std::invalid_argument& e) {
            std::cerr << "Error collecting taxes: " << e.what() << std::endl;
        }
    }
    totalTax *= taxRate;
    treasury += totalTax;
}

bool Economy::allocateBudget(const std::string& category, double percentage) {
    if (percentage < 0 || percentage > 1) {
        throw std::invalid_argument("Budget percentage must be between 0 and 1");
    }
    double totalAllocation = percentage;
    for (auto it = budgetAllocation.begin(); it != budgetAllocation.end(); ++it) {
        if (it->first != category) {
            totalAllocation += it->second;
        }
    }
    if (totalAllocation > 1.0) {
        return false;
    }
    budgetAllocation[category] = percentage;
    return true;
}

double Economy::getBudgetFor(const std::string& category) const {
    auto it = budgetAllocation.find(category);
    if (it != budgetAllocation.end()) {
        return it->second * treasury;
    }
    return 0.0;
}

void Economy::updateInflation(double marketActivity, double resourceScarcity) {
    double newInflation = inflationRate;
    newInflation -= marketActivity * 0.01;
    newInflation += resourceScarcity * 0.02;
    double randomFactor = (static_cast<double>(rand()) / RAND_MAX - 0.5) * 0.01;
    newInflation += randomFactor;
    if (newInflation < -0.05) {
        newInflation = -0.05;
    }
    else if (newInflation > 0.3) {
        newInflation = 0.3;
    }
    inflationRate = newInflation;
}

bool Economy::payExpense(double amount, const std::string& reason) {
    if (amount < 0) {
        throw std::invalid_argument("Expense amount cannot be negative");
    }
    if (amount > treasury) {
        return false;
    }
    treasury -= amount;
    return true;
}

void Economy::addIncome(double amount, const std::string& source) {
    if (amount < 0) {
        throw std::invalid_argument("Income amount cannot be negative");
    }
    treasury += amount;
}

void Economy::updateTradeBalance(double exports, double imports) {
    tradeBalance = exports - imports;
    if (tradeBalance > 0) {
        inflationRate -= tradeBalance * 0.001;
    }
    else {
        inflationRate += std::abs(tradeBalance) * 0.001;
    }
    if (inflationRate < -0.05) {
        inflationRate = -0.05;
    }
    else if (inflationRate > 0.3) {
        inflationRate = 0.3;
    }
}

void Economy::saveToFile(std::ofstream& outFile) const {
    outFile << treasury << "\n";
    outFile << taxRate << "\n";
    outFile << inflationRate << "\n";
    outFile << publicServiceCost << "\n";
    outFile << tradeBalance << "\n";
    outFile << budgetAllocation.size() << "\n";
    for (auto it = budgetAllocation.begin(); it != budgetAllocation.end(); ++it) {
        outFile << it->first << "\n";
        outFile << it->second << "\n";
    }
}

void Economy::loadFromFile(std::ifstream& inFile) {
    std::string line;
    std::getline(inFile, line);
    treasury = std::stod(line);
    std::getline(inFile, line);
    taxRate = std::stod(line);
    std::getline(inFile, line);
    inflationRate = std::stod(line);
    std::getline(inFile, line);
    publicServiceCost = std::stod(line);
    std::getline(inFile, line);
    tradeBalance = std::stod(line);
    std::getline(inFile, line);
    int numCategories = std::stoi(line);
    budgetAllocation.clear();
    for (int i = 0; i < numCategories; ++i) {
        std::string category;
        double percentage;
        std::getline(inFile, category);
        std::getline(inFile, line);
        percentage = std::stod(line);
        budgetAllocation[category] = percentage;
    }
}

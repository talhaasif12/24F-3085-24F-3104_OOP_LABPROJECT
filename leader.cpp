#include "stronghold.h"

Leader::Leader(const std::string& name, const std::string& title)
    : name(name), title(title), monthsInPower(0) {
    charisma = 30 + (rand() % 41);
    intelligence = 30 + (rand() % 41);
    corruption = 30 + (rand() % 41);
    leadership = 30 + (rand() % 41);
    popularity = 0.5;
    const std::string styles[] = { "Benevolent", "Authoritarian", "Democratic", "Militaristic", "Diplomatic" };
    leadershipStyle = styles[rand() % 5];
}

Leader::~Leader() {
}

std::string Leader::getName() const {
    return name;
}

std::string Leader::getTitle() const {
    return title;
}

void Leader::setTitle(const std::string& newTitle) {
    title = newTitle;
}

int Leader::getCharisma() const {
    return charisma;
}

void Leader::setCharisma(int value) {
    if (value < 0 || value > 100) {
        throw std::invalid_argument("Charisma must be between 0 and 100");
    }
    charisma = value;
}

int Leader::getIntelligence() const {
    return intelligence;
}

void Leader::setIntelligence(int value) {
    if (value < 0 || value > 100) {
        throw std::invalid_argument("Intelligence must be between 0 and 100");
    }
    intelligence = value;
}

int Leader::getCorruption() const {
    return corruption;
}

void Leader::setCorruption(int value) {
    if (value < 0 || value > 100) {
        throw std::invalid_argument("Corruption must be between 0 and 100");
    }
    corruption = value;
}

int Leader::getLeadership() const {
    return leadership;
}

void Leader::setLeadership(int value) {
    if (value < 0 || value > 100) {
        throw std::invalid_argument("Leadership must be between 0 and 100");
    }
    leadership = value;
}

double Leader::getPopularity() const {
    return popularity;
}

void Leader::setPopularity(double value) {
    if (value < 0 || value > 1.0) {
        throw std::invalid_argument("Popularity must be between 0 and 1");
    }
    popularity = value;
}

std::string Leader::getLeadershipStyle() const {
    return leadershipStyle;
}

void Leader::setLeadershipStyle(const std::string& style) {
    leadershipStyle = style;
}

int Leader::getMonthsInPower() const {
    return monthsInPower;
}

void Leader::incrementMonthsInPower() {
    monthsInPower++;
}

void Leader::makeTaxDecision(Economy& economy, const Population& population) {
    double currentTaxRate = economy.getTaxRate();
    double overallHappiness = population.getOverallHappiness();
    double desiredTaxChange = 0.0;

    if (corruption > 70) {
        desiredTaxChange += 0.02;
    }

    if (intelligence > 60) {
        if (overallHappiness < 0.3) {
            desiredTaxChange -= 0.03;
        }
        else if (overallHappiness > 0.7 && currentTaxRate < 0.25) {
            desiredTaxChange += 0.01;
        }
    }

    if (leadershipStyle == "Benevolent") {
        desiredTaxChange -= 0.01;
    }
    else if (leadershipStyle == "Authoritarian") {
        desiredTaxChange += 0.02;
    }

    double taxChange = desiredTaxChange + (((double)rand() / RAND_MAX) * 0.04 - 0.02);
    double newTaxRate = currentTaxRate + taxChange;

    if (newTaxRate < 0.05) newTaxRate = 0.05;
    if (newTaxRate > 0.35) newTaxRate = 0.35;

    economy.setTaxRate(newTaxRate);
}

void Leader::makeWarDecision(Military& military, Economy& economy, bool isUnderThreat) {
    bool increaseForces = false;

    if (leadershipStyle == "Militaristic") {
        increaseForces = true;
    }
    else if (isUnderThreat) {
        increaseForces = true;
    }
    else if (leadership > 70 && intelligence > 60) {
        increaseForces = (military.getSoldiers() < 1000);
    }
    else {
        increaseForces = (((double)rand() / RAND_MAX) < 0.3);
    }

    if (increaseForces) {
        int desiredRecruits = 50 + (leadership * 2);
        double recruitCost = desiredRecruits * 10;

        if (economy.getTreasury() > recruitCost * 2) {
            bool paymentSuccess = economy.payExpense(recruitCost, "Military Recruitment");
            if (paymentSuccess) {
                military.recruit(desiredRecruits);
            }
        }

        if (economy.getTreasury() > 500) {
            bool trainingPayment = economy.payExpense(200, "Military Training");
            if (trainingPayment) {
                military.train(20);
            }
        }
    }
}

void Leader::handleCrisis(const std::string& crisisType, Kingdom& kingdom) {
    Economy& economy = kingdom.getEconomy();
    Population& population = kingdom.getPopulation();
    Military& military = kingdom.getMilitary();

    if (crisisType == "famine") {
        try {
            Resource& food = kingdom.getResource("food");
            if (economy.getTreasury() > 1000) {
                economy.payExpense(1000, "Emergency Food Import");
                food.setAmount(food.getAmount() + 500);
            }
            economy.setTaxRate(economy.getTaxRate() * 0.8);
        }
        catch (std::exception& e) {
            std::cerr << "Error during famine crisis: " << e.what() << std::endl;
        }
    }
    else if (crisisType == "rebellion") {
        if (leadershipStyle == "Benevolent") {
            economy.setTaxRate(economy.getTaxRate() * 0.7);
            try {
                Resource& food = kingdom.getResource("food");
                if (food.getAmount() > 200) {
                    food.setAmount(food.getAmount() - 200);
                }
            }
            catch (std::exception& e) {
            }
        }
        else if (leadershipStyle == "Authoritarian" || leadershipStyle == "Militaristic") {
            military.setMorale(military.getMorale() - 10);
        }
    }
    else if (crisisType == "plague") {
        economy.payExpense(economy.getTreasury() * 0.2, "Medical Response");
    }
    else if (crisisType == "invasion") {
        makeWarDecision(military, economy, true);
        economy.allocateBudget("military", 0.5);
    }

    updatePopularity(population);
}

bool Leader::handleRebellion(const Population& population, Military& military) {
    double rebellionStrength = (1.0 - population.getOverallHappiness()) * population.getTotalPopulation() * 0.01;
    double militaryEffectiveness = military.getSoldiers() * (military.getMorale() / 100.0) * (military.getTraining() / 100.0);
    double leadershipFactor = leadership / 100.0;
    double styleModifier = 1.0;
    if (leadershipStyle == "Benevolent") styleModifier = 0.8;
    if (leadershipStyle == "Authoritarian") styleModifier = 1.2;

    bool rebellionSucceeds = (rebellionStrength > militaryEffectiveness * leadershipFactor * styleModifier);

    if (rebellionSucceeds) {
        popularity = 0.0;
    }
    else {
        popularity *= 0.8;
    }

    return !rebellionSucceeds;
}

void Leader::updatePopularity(const Population& population) {
    double basePopularity = population.getOverallHappiness() * 0.6;
    double charismaFactor = charisma / 100.0;
    double styleModifier = 0.0;
    if (leadershipStyle == "Benevolent") styleModifier = 0.2;
    if (leadershipStyle == "Democratic") styleModifier = 0.1;
    if (leadershipStyle == "Authoritarian") styleModifier = -0.1;

    double timeFactor = 1.0 - (monthsInPower / 120.0);
    if (timeFactor < 0.5) timeFactor = 0.5;

    double newPopularity = basePopularity + (charismaFactor * 0.3) + styleModifier;
    newPopularity *= timeFactor;
    newPopularity += ((double)rand() / RAND_MAX) * 0.1 - 0.05;

    if (newPopularity < 0.0) newPopularity = 0.0;
    if (newPopularity > 1.0) newPopularity = 1.0;

    popularity = newPopularity;
}

bool Leader::isCoupLikely(const Military& military, const Population& population) const {
    bool unstableCivilian = (popularity < 0.3);
    bool corruptLeadership = (corruption > 70);
    bool militaryDissatisfaction = (military.getMorale() < 40);

    double coupProbability = 0.0;

    if (unstableCivilian) coupProbability += 0.3;
    if (corruptLeadership) coupProbability += 0.2;
    if (militaryDissatisfaction) coupProbability += 0.4;

    if (leadershipStyle == "Militaristic") coupProbability -= 0.2;
    if (leadershipStyle == "Authoritarian") coupProbability += 0.1;

    if (monthsInPower < 12) coupProbability *= 0.5;

    return (((double)rand() / RAND_MAX) < coupProbability);
}

void Leader::saveToFile(std::ofstream& outFile) const {
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to save leader data: File not open");
    }

    outFile << name << std::endl;
    outFile << title << std::endl;
    outFile << charisma << std::endl;
    outFile << intelligence << std::endl;
    outFile << corruption << std::endl;
    outFile << leadership << std::endl;
    outFile << popularity << std::endl;
    outFile << leadershipStyle << std::endl;
    outFile << monthsInPower << std::endl;
}

void Leader::loadFromFile(std::ifstream& inFile) {
    if (!inFile.is_open()) {
        throw std::runtime_error("Failed to load leader data: File not open");
    }

    std::string dummy;
    std::getline(inFile, dummy);

    std::getline(inFile, name);
    std::getline(inFile, title);
    inFile >> charisma;
    inFile >> intelligence;
    inFile >> corruption;
    inFile >> leadership;
    inFile >> popularity;

    std::getline(inFile, dummy);

    std::getline(inFile, leadershipStyle);
    inFile >> monthsInPower;
}
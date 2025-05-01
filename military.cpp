#include "Stronghold.h"

Military::Military(int initialSoldiers, int initialTraining, int initialMorale)
    : soldiers(initialSoldiers), training(initialTraining), morale(initialMorale),
      casualties(0), maintenanceCost(1.0), corruptionLevel(0.0), victories(0), defeats(0) {
}

Military::~Military() {
    
}

int Military::getSoldiers() const {
    return soldiers;
}

void Military::setSoldiers(int amount) {
    if (amount < 0) {
        throw std::invalid_argument("Number of soldiers cannot be negative");
    }
    soldiers = amount;
}

int Military::getTraining() const {
    return training;
}

void Military::setTraining(int level) {
    if (level < 0 || level > 100) {
        throw std::invalid_argument("Training level must be between 0 and 100");
    }
    training = level;
}

int Military::getMorale() const {
    return morale;
}

void Military::setMorale(int level) {
    if (level < 0 || level > 100) {
        throw std::invalid_argument("Morale level must be between 0 and 100");
    }
    morale = level;
}

double Military::getMaintenanceCost() const {
    return maintenanceCost;
}

void Military::setMaintenanceCost(double cost) {
    if (cost < 0) {
        throw std::invalid_argument("Maintenance cost cannot be negative");
    }
    maintenanceCost = cost;
}

double Military::getCorruptionLevel() const {
    return corruptionLevel;
}

void Military::setCorruptionLevel(double level) {
    if (level < 0 || level > 1) {
        throw std::invalid_argument("Corruption level must be between 0 and 1");
    }
    corruptionLevel = level;
}

void Military::recruit(int newSoldiers) {
    if (newSoldiers < 0) {
        throw std::invalid_argument("Cannot recruit negative number of soldiers");
    }
    
   
    if (soldiers + newSoldiers > 0) {
        training = (training * soldiers + 10 * newSoldiers) / (soldiers + newSoldiers);
        
        
        if (training < 10) {
            training = 10;
        } else if (training > 100) {
            training = 100;
        }
    }
    
    soldiers += newSoldiers;
}

void Military::train(int resources) {
    if (resources < 0) {
        throw std::invalid_argument("Training resources cannot be negative");
    }
    
    if (soldiers == 0) {
        return; 
    }
    
    
    int trainingIncrease = resources / (soldiers / 10 + 1);
    training += trainingIncrease;
    
    
    if (training > 100) {
        training = 100;
    }
}

bool Military::battle(Military& enemyArmy) {
    if (soldiers == 0) {
        return false; 
    }
    
    int ourStrength = calculateStrength();
    int enemyStrength = enemyArmy.calculateStrength();
    
    
    double casualtyRate = 0.05 + (static_cast<double>(enemyStrength) / ourStrength) * 0.1;
    if (casualtyRate > 0.5) {
        casualtyRate = 0.5; 
    }
    
    int ourCasualties = static_cast<int>(soldiers * casualtyRate);
    double enemyCasualtyRate = 0.05 + (static_cast<double>(ourStrength) / enemyStrength) * 0.1;
    if (enemyCasualtyRate > 0.5) {
        enemyCasualtyRate = 0.5;
    }
    
    int enemyCasualties = static_cast<int>(enemyArmy.getSoldiers() * enemyCasualtyRate);
    
    
    soldiers -= ourCasualties;
    enemyArmy.setSoldiers(enemyArmy.getSoldiers() - enemyCasualties);
    casualties += ourCasualties;
    
    
    bool victory = ourStrength > enemyStrength;
    
    
    if (victory) {
        morale += 10;
        if (morale > 100) {
            morale = 100;
        }
        victories++;
    } else {
        morale -= 15;
        if (morale < 0) {
            morale = 0;
        }
        defeats++;
    }
    
    return victory;
}

void Military::updateMorale(double payment, double food) {
    
    double paymentFactor = payment / (soldiers * maintenanceCost);
    if (paymentFactor > 1.0) {
        paymentFactor = 1.0;
    }
    
    double foodFactor = food / soldiers;
    if (foodFactor > 1.0) {
        foodFactor = 1.0;
    }
    
    
    int moraleChange = static_cast<int>((paymentFactor * 10) + (foodFactor * 10) - 10);
    
    
    morale += moraleChange;
    
    
    if (morale < 0) {
        morale = 0;
    } else if (morale > 100) {
        morale = 100;
    }
}

int Military::calculateStrength() const {
    
    double strength = soldiers * (0.5 + (training / 100.0) * 0.5) * (0.5 + (morale / 100.0) * 0.5);
    
    
    strength *= (1.0 - corruptionLevel * 0.5);
    
    return static_cast<int>(strength);
}

void Military::payArmy(double amount) {
    double expectedPayment = soldiers * maintenanceCost;
    double paymentRatio = amount / expectedPayment;
    
    if (paymentRatio < 0.7) {
       
        int moraleDecrease = static_cast<int>((0.7 - paymentRatio) * 30);
        morale -= moraleDecrease;
        
        
        if (paymentRatio < 0.5) {
            int deserters = static_cast<int>(soldiers * (0.5 - paymentRatio) * 0.4);
            soldiers -= deserters;
        }
    } else if (paymentRatio > 1.0) {
        
        int moraleIncrease = static_cast<int>((paymentRatio - 1.0) * 20);
        morale += moraleIncrease;
    }
    
    
    if (morale < 0) {
        morale = 0;
    } else if (morale > 100) {
        morale = 100;
    }
    
    
    if (soldiers < 0) {
        soldiers = 0;
    }
}

void Military::feedArmy(int foodUnits) {
    double foodPerSoldier = static_cast<double>(foodUnits) / soldiers;
    
    if (foodPerSoldier < 0.7) {
        
        int moraleDecrease = static_cast<int>((0.7 - foodPerSoldier) * 30);
        morale -= moraleDecrease;
        
        
        if (foodPerSoldier < 0.5) {
            int casualties = static_cast<int>(soldiers * (0.5 - foodPerSoldier) * 0.2);
            soldiers -= casualties;
            this->casualties += casualties;
        }
    } else if (foodPerSoldier > 1.0) {
        
        int moraleIncrease = static_cast<int>((foodPerSoldier - 1.0) * 10);
        morale += moraleIncrease;
    }
    
    
    if (morale < 0) {
        morale = 0;
    } else if (morale > 100) {
        morale = 100;
    }
    
    
    if (soldiers < 0) {
        soldiers = 0;
    }
}

void Military::updateCorruption(double leaderInfluence) {
    
    corruptionLevel = corruptionLevel * 0.8 + leaderInfluence * 0.2;
    
    
    if (corruptionLevel < 0) {
        corruptionLevel = 0;
    } else if (corruptionLevel > 1) {
        corruptionLevel = 1;
    }
}

void Military::saveToFile(std::ofstream& outFile) const {
    outFile << soldiers << "\n";
    outFile << training << "\n";
    outFile << morale << "\n";
    outFile << casualties << "\n";
    outFile << maintenanceCost << "\n";
    outFile << corruptionLevel << "\n";
    outFile << victories << "\n";
    outFile << defeats << "\n";
}

void Military::loadFromFile(std::ifstream& inFile) {
    std::string line;
    
    std::getline(inFile, line);
    soldiers = std::stoi(line);
    
    std::getline(inFile, line);
    training = std::stoi(line);
    
    std::getline(inFile, line);
    morale = std::stoi(line);
    
    std::getline(inFile, line);
    casualties = std::stoi(line);
    
    std::getline(inFile, line);
    maintenanceCost = std::stod(line);
    
    std::getline(inFile, line);
    corruptionLevel = std::stod(line);
    
    std::getline(inFile, line);
    victories = std::stoi(line);
    
    std::getline(inFile, line);
    defeats = std::stoi(line);
}
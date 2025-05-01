#include "Stronghold.h"

Population::Population(int initialPop)
    : totalPopulation(initialPop), growthRate(0.01), deathRate(0.005),
    migrationRate(0.0), healthLevel(0.8), numSocialClasses(0), maxSocialClasses(10) {

    socialClasses = new SocialClass * [maxSocialClasses];
    for (int i = 0; i < maxSocialClasses; ++i) {
        socialClasses[i] = nullptr;
    }
}

Population::~Population() {
    for (int i = 0; i < numSocialClasses; ++i) {
        delete socialClasses[i];
    }
    delete[] socialClasses;
}

int Population::getTotalPopulation() const {
    return totalPopulation;
}

double Population::getGrowthRate() const {
    return growthRate;
}

void Population::setGrowthRate(double rate) {
    if (rate < -0.1 || rate > 0.1) {
        throw std::invalid_argument("Growth rate must be between -0.1 and 0.1");
    }
    growthRate = rate;
}

double Population::getDeathRate() const {
    return deathRate;
}

void Population::setDeathRate(double rate) {
    if (rate < 0 || rate > 0.1) {
        throw std::invalid_argument("Death rate must be between 0 and 0.1");
    }
    deathRate = rate;
}

double Population::getHealthLevel() const {
    return healthLevel;
}

void Population::setHealthLevel(double level) {
    if (level < 0 || level > 1) {
        throw std::invalid_argument("Health level must be between 0 and 1");
    }
    healthLevel = level;
}

void Population::addSocialClass(const SocialClass& socialClass) {
    if (numSocialClasses >= maxSocialClasses) {
        int newSize = maxSocialClasses * 2;
        SocialClass** newArray = new SocialClass * [newSize];

        for (int i = 0; i < numSocialClasses; ++i) {
            newArray[i] = socialClasses[i];
        }

        for (int i = numSocialClasses; i < newSize; ++i) {
            newArray[i] = nullptr;
        }

        delete[] socialClasses;
        socialClasses = newArray;
        maxSocialClasses = newSize;
    }

    socialClasses[numSocialClasses] = new SocialClass(socialClass);
    numSocialClasses++;
}

SocialClass& Population::getSocialClass(const std::string& name) {
    for (int i = 0; i < numSocialClasses; ++i) {
        if (socialClasses[i]->getName() == name) {
            return *socialClasses[i];
        }
    }

    throw std::invalid_argument("Social class not found: " + name);
}

SocialClass& Population::getSocialClassByIndex(int index) const {
    if (index < 0 || index >= numSocialClasses) {
        throw std::out_of_range("Social class index out of range");
    }
    return *socialClasses[index];
}

int Population::getSocialClassCount() const {
    return numSocialClasses;
}

void Population::updatePopulation(bool isFamine, bool isDisease, bool isWar) {
    double effectiveGrowthRate = growthRate;
    double effectiveDeathRate = deathRate;

    if (isFamine) {
        effectiveGrowthRate *= 0.5;
        effectiveDeathRate *= 2.0;
    }

    if (isDisease) {
        effectiveDeathRate += (1.0 - healthLevel) * 0.05;
    }

    if (isWar) {
        effectiveDeathRate += 0.02;
        effectiveGrowthRate *= 0.8;
    }

    int births = static_cast<int>(totalPopulation * effectiveGrowthRate);
    int deaths = static_cast<int>(totalPopulation * effectiveDeathRate);
    int migration = static_cast<int>(totalPopulation * migrationRate);

    int populationChange = births - deaths + migration;
    totalPopulation += populationChange;

    if (totalPopulation < 0) {
        totalPopulation = 0;
    }

    for (int i = 0; i < numSocialClasses; ++i) {
        if (totalPopulation > 0) {
            double classProportion = static_cast<double>(socialClasses[i]->getPopulation()) /
                (totalPopulation - populationChange);
            int classChange = static_cast<int>(populationChange * classProportion);

            int newPopulation = socialClasses[i]->getPopulation() + classChange;
            if (newPopulation < 0) {
                newPopulation = 0;
            }

            socialClasses[i]->setPopulation(newPopulation);
        }
        else {
            socialClasses[i]->setPopulation(0);
        }
    }
}

double Population::getOverallHappiness() const {
    if (numSocialClasses == 0) {
        return 0.5;
    }

    double totalHappiness = 0.0;
    int totalPeople = 0;

    for (int i = 0; i < numSocialClasses; ++i) {
        totalHappiness += socialClasses[i]->getHappiness() * socialClasses[i]->getPopulation();
        totalPeople += socialClasses[i]->getPopulation();
    }

    if (totalPeople == 0) {
        return 0.0;
    }

    return totalHappiness / totalPeople;
}

void Population::distributeResources(std::map<std::string, Resource>& resources) {
    for (int i = 0; i < numSocialClasses; ++i) {
        SocialClass* currentClass = socialClasses[i];
        int classPopulation = currentClass->getPopulation();

        for (auto& resourcePair : resources) {
            const std::string& resourceName = resourcePair.first;
            Resource& resource = resourcePair.second;

            double perCapitaConsumption = currentClass->getResourceConsumption(resourceName);
            int totalConsumption = static_cast<int>(perCapitaConsumption * classPopulation);

            if (!resource.consume(totalConsumption)) {
            }
        }
    }
}

bool Population::checkRebellion() const {
    for (int i = 0; i < numSocialClasses; ++i) {
        if (socialClasses[i]->checkRebellion()) {
            return true;
        }
    }
    return false;
}

int Population::getAvailableRecruits() const {
    int totalRecruits = 0;

    for (int i = 0; i < numSocialClasses; ++i) {
        totalRecruits += socialClasses[i]->getAvailableRecruits();
    }

    return totalRecruits;
}

void Population::updateSocialClassHappiness(double foodSatisfaction, double safetySatisfaction, double wealthSatisfaction) {
    for (int i = 0; i < numSocialClasses; ++i) {
        socialClasses[i]->updateHappiness(foodSatisfaction, safetySatisfaction, wealthSatisfaction);
    }
}

void Population::saveToFile(std::ofstream& outFile) const {
    outFile << totalPopulation << "\n";
    outFile << growthRate << "\n";
    outFile << deathRate << "\n";
    outFile << migrationRate << "\n";
    outFile << healthLevel << "\n";

    outFile << numSocialClasses << "\n";
    for (int i = 0; i < numSocialClasses; ++i) {
        socialClasses[i]->saveToFile(outFile);
    }
}

void Population::loadFromFile(std::ifstream& inFile) {
    std::string line;

    std::getline(inFile, line);
    totalPopulation = std::stoi(line);

    std::getline(inFile, line);
    growthRate = std::stod(line);

    std::getline(inFile, line);
    deathRate = std::stod(line);

    std::getline(inFile, line);
    migrationRate = std::stod(line);

    std::getline(inFile, line);
    healthLevel = std::stod(line);

    for (int i = 0; i < numSocialClasses; ++i) {
        delete socialClasses[i];
    }

    std::getline(inFile, line);
    int classCount = std::stoi(line);

    if (classCount > maxSocialClasses) {
        delete[] socialClasses;
        maxSocialClasses = classCount * 2;
        socialClasses = new SocialClass * [maxSocialClasses];
        for (int i = 0; i < maxSocialClasses; ++i) {
            socialClasses[i] = nullptr;
        }
    }

    numSocialClasses = classCount;
    for (int i = 0; i < numSocialClasses; ++i) {
        socialClasses[i] = new SocialClass("Temp", 0);
        socialClasses[i]->loadFromFile(inFile);
    }
}

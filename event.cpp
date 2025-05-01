#include "Stronghold.h"

Event::Event(const std::string& name, const std::string& desc, const std::string& type, int duration)
    : name(name), description(desc), type(type), duration(duration), currentDuration(0) {
}

Event::~Event() {
}

std::string Event::getName() const {
    return name;
}

std::string Event::getDescription() const {
    return description;
}

std::string Event::getType() const {
    return type;
}

int Event::getDuration() const {
    return duration;
}

int Event::getCurrentDuration() const {
    return currentDuration;
}

void Event::incrementDuration() {
    currentDuration++;
}

bool Event::isActive() const {
    return (currentDuration < duration);
}

void Event::addEffect(const std::string& target, double multiplier) {
    effects[target] = multiplier;
}

double Event::getEffect(const std::string& target) const {
    auto it = effects.find(target);
    if (it != effects.end()) {
        return it->second;
    }
    return 1.0;
}

void Event::applyEffects(Kingdom& kingdom) {
    for (auto& effect : effects) {
        const std::string& target = effect.first;
        double multiplier = effect.second;

        try {
            size_t resourcePos = target.find("resource_");
            if (resourcePos != std::string::npos) {
                std::string resourceName = target.substr(9);
                Resource& resource = kingdom.getResource(resourceName);
                int oldRate = resource.getProductionRate();
                resource.setProductionRate(static_cast<int>(oldRate * multiplier));
                continue;
            }

            if (target == "economy_treasury") {
                Economy& economy = kingdom.getEconomy();
                double oldTreasury = economy.getTreasury();
                economy.setTreasury(oldTreasury * multiplier);
            }
            else if (target == "economy_inflation") {
                Economy& economy = kingdom.getEconomy();
                double oldRate = economy.getInflationRate();
                economy.setInflationRate(oldRate * multiplier);
            }

            if (target == "population_growth") {
                Population& population = kingdom.getPopulation();
                double oldRate = population.getGrowthRate();
                population.setGrowthRate(oldRate * multiplier);
            }
            else if (target == "population_health") {
                Population& population = kingdom.getPopulation();
                double oldHealth = population.getHealthLevel();
                population.setHealthLevel(oldHealth * multiplier);
            }

            if (target == "military_morale") {
                Military& military = kingdom.getMilitary();
                int oldMorale = military.getMorale();
                military.setMorale(static_cast<int>(oldMorale * multiplier));
            }
            else if (target == "military_training") {
                Military& military = kingdom.getMilitary();
                int oldTraining = military.getTraining();
                military.setTraining(static_cast<int>(oldTraining * multiplier));
            }

        }
        catch (std::exception& e) {
            std::cerr << "Error applying event effect: " << e.what() << std::endl;
        }
    }
}

Event Event::generateRandomEvent() {
    int eventType = rand() % 4;

    switch (eventType) {
    case 0: return generateDisaster();
    case 1: return generateEconomicEvent();
    case 2: return generateMilitaryEvent();
    case 3: return generatePoliticalEvent();
    default: return generateDisaster();
    }
}

Event Event::generateDisaster() {
    const std::string disasters[][3] = {
        {"Plague", "A deadly disease is spreading through your kingdom.", "disaster"},
        {"Famine", "Crops are failing across your lands.", "disaster"},
        {"Earthquake", "A powerful earthquake has damaged buildings and infrastructure.", "disaster"},
        {"Flood", "Heavy rains have caused rivers to overflow.", "disaster"},
        {"Drought", "A severe lack of rain has dried up wells and fields.", "disaster"},
        {"Fire", "A massive fire has spread through parts of your kingdom.", "disaster"},
        {"Locust Swarm", "Insects are destroying crops throughout the region.", "disaster"},
        {"Harsh Winter", "An unusually cold winter is causing suffering.", "disaster"}
    };

    int index = rand() % 8;

    Event event(disasters[index][0], disasters[index][1], disasters[index][2], 3 + (rand() % 4));

    if (disasters[index][0] == "Plague") {
        event.addEffect("population_growth", 0.5);
        event.addEffect("population_health", 0.7);
        event.addEffect("military_morale", 0.8);
    }
    else if (disasters[index][0] == "Famine") {
        event.addEffect("resource_food", 0.6);
        event.addEffect("population_growth", 0.7);
        event.addEffect("military_morale", 0.9);
    }
    else if (disasters[index][0] == "Earthquake") {
        event.addEffect("resource_stone", 0.8);
        event.addEffect("resource_gold", 0.9);
        event.addEffect("population_growth", 0.9);
    }
    else if (disasters[index][0] == "Flood") {
        event.addEffect("resource_food", 0.7);
        event.addEffect("resource_wood", 0.8);
        event.addEffect("population_health", 0.9);
    }
    else if (disasters[index][0] == "Drought") {
        event.addEffect("resource_food", 0.6);
        event.addEffect("population_health", 0.8);
        event.addEffect("military_morale", 0.9);
    }
    else if (disasters[index][0] == "Fire") {
        event.addEffect("resource_wood", 0.6);
        event.addEffect("economy_treasury", 0.9);
        event.addEffect("population_growth", 0.9);
    }
    else if (disasters[index][0] == "Locust Swarm") {
        event.addEffect("resource_food", 0.5);
        event.addEffect("economy_treasury", 0.9);
    }
    else if (disasters[index][0] == "Harsh Winter") {
        event.addEffect("resource_food", 0.8);
        event.addEffect("population_health", 0.8);
        event.addEffect("military_training", 0.9);
    }

    return event;
}

Event Event::generateEconomicEvent() {
    const std::string economicEvents[][3] = {
        {"Trade Boom", "Foreign merchants have arrived bringing new trade opportunities.", "economic"},
        {"Market Crash", "A financial panic has caused markets to collapse.", "economic"},
        {"New Tax Method", "A new tax collection system has been implemented.", "economic"},
        {"Currency Devaluation", "Your currency has lost value against foreign currencies.", "economic"},
        {"Treasure Discovery", "A forgotten treasury has been discovered.", "economic"},
        {"Guild Formation", "Craftsmen have organized into a powerful guild.", "economic"},
        {"Foreign Investment", "Foreign nobles want to invest in your kingdom.", "economic"},
        {"Trade Route Disruption", "A key trade route has been disrupted.", "economic"}
    };

    int index = rand() % 8;

    Event event(economicEvents[index][0], economicEvents[index][1], economicEvents[index][2], 2 + (rand() % 5));

    if (economicEvents[index][0] == "Trade Boom") {
        event.addEffect("economy_treasury", 1.2);
        event.addEffect("resource_gold", 1.3);
    }
    else if (economicEvents[index][0] == "Market Crash") {
        event.addEffect("economy_treasury", 0.8);
        event.addEffect("economy_inflation", 1.2);
    }
    else if (economicEvents[index][0] == "New Tax Method") {
        event.addEffect("economy_treasury", 1.1);
        event.addEffect("population_growth", 0.95);
    }
    else if (economicEvents[index][0] == "Currency Devaluation") {
        event.addEffect("economy_inflation", 1.4);
        event.addEffect("resource_gold", 0.8);
    }
    else if (economicEvents[index][0] == "Treasure Discovery") {
        event.addEffect("economy_treasury", 1.5);
        event.addEffect("military_morale", 1.1);
    }
    else if (economicEvents[index][0] == "Guild Formation") {
        event.addEffect("resource_gold", 1.2);
        event.addEffect("economy_inflation", 0.9);
    }
    else if (economicEvents[index][0] == "Foreign Investment") {
        event.addEffect("economy_treasury", 1.3);
        event.addEffect("resource_gold", 1.1);
    }
    else if (economicEvents[index][0] == "Trade Route Disruption") {
        event.addEffect("economy_treasury", 0.9);
        event.addEffect("resource_food", 0.9);
    }

    return event;
}

Event Event::generateMilitaryEvent() {
    const std::string militaryEvents[][3] = {
        {"Veteran Recruiters", "Seasoned veterans are helping recruit new soldiers.", "military"},
        {"Weapons Innovation", "A new weapon design is improving your army's effectiveness.", "military"},
        {"Desertion", "Several soldiers have deserted your army.", "military"},
        {"Foreign Mercenaries", "Foreign mercenaries are available for hire.", "military"},
        {"Military Scandal", "A scandal has tarnished the reputation of your military leaders.", "military"},
        {"Border Skirmish", "Minor conflicts have broken out along your borders.", "military"},
        {"Military Training Reform", "New training methods are being implemented.", "military"},
        {"War Heroes", "Celebrated war heroes are boosting military morale.", "military"}
    };

    int index = rand() % 8;

    Event event(militaryEvents[index][0], militaryEvents[index][1], militaryEvents[index][2], 2 + (rand() % 4));

    if (militaryEvents[index][0] == "Veteran Recruiters") {
        event.addEffect("military_training", 1.2);
        event.addEffect("military_morale", 1.1);
    }
    else if (militaryEvents[index][0] == "Weapons Innovation") {
        event.addEffect("military_training", 1.3);
        event.addEffect("economy_treasury", 0.95);
    }
    else if (militaryEvents[index][0] == "Desertion") {
        event.addEffect("military_morale", 0.8);
        event.addEffect("military_training", 0.9);
    }
    else if (militaryEvents[index][0] == "Foreign Mercenaries") {
        event.addEffect("military_training", 1.1);
        event.addEffect("economy_treasury", 0.9);
    }
    else if (militaryEvents[index][0] == "Military Scandal") {
        event.addEffect("military_morale", 0.7);
        event.addEffect("population_growth", 0.95);
    }
    else if (militaryEvents[index][0] == "Border Skirmish") {
        event.addEffect("military_morale", 1.1);
        event.addEffect("military_training", 1.1);
        event.addEffect("economy_treasury", 0.95);
    }
    else if (militaryEvents[index][0] == "Military Training Reform") {
        event.addEffect("military_training", 1.2);
        event.addEffect("economy_treasury", 0.9);
    }
    else if (militaryEvents[index][0] == "War Heroes") {
        event.addEffect("military_morale", 1.3);
        event.addEffect("military_training", 1.1);
    }

    return event;
}

Event Event::generatePoliticalEvent() {
    const std::string politicalEvents[][3] = {
        {"Foreign Embassy", "A foreign kingdom has established an embassy in your capital.", "political"},
        {"Noble Conspiracy", "Nobles are conspiring against your rule.", "political"},
        {"Religious Revival", "A religious revival is sweeping through your kingdom.", "political"},
        {"Popular Uprising", "Common people are protesting against certain policies.", "political"},
        {"Royal Marriage", "A marriage alliance has been proposed with a neighboring kingdom.", "political"},
        {"Diplomatic Crisis", "A diplomatic incident threatens relations with a neighbor.", "political"},
        {"New Adviser", "A brilliant adviser offers their services to your court.", "political"},
        {"Succession Dispute", "Questions about succession are causing political tension.", "political"}
    };

    int index = rand() % 8;

    Event event(politicalEvents[index][0], politicalEvents[index][1], politicalEvents[index][2], 3 + (rand() % 5));

    if (politicalEvents[index][0] == "Foreign Embassy") {
        event.addEffect("economy_treasury", 1.1);
        event.addEffect("population_growth", 1.05);
    }
    else if (politicalEvents[index][0] == "Noble Conspiracy") {
        event.addEffect("military_morale", 0.9);
        event.addEffect("economy_treasury", 0.95);
    }
    else if (politicalEvents[index][0] == "Religious Revival") {
        event.addEffect("population_growth", 1.1);
        event.addEffect("economy_treasury", 1.05);
    }
    else if (politicalEvents[index][0] == "Popular Uprising") {
        event.addEffect("economy_treasury", 0.9);
        event.addEffect("population_growth", 0.95);
    }
    else if (politicalEvents[index][0] == "Royal Marriage") {
        event.addEffect("economy_treasury", 1.2);
        event.addEffect("military_morale", 1.1);
    }
    else if (politicalEvents[index][0] == "Diplomatic Crisis") {
        event.addEffect("economy_treasury", 0.9);
        event.addEffect("military_morale", 0.95);
    }
    else if (politicalEvents[index][0] == "New Adviser") {
        event.addEffect("economy_treasury", 1.1);
        event.addEffect("military_training", 1.1);
    }
    else if (politicalEvents[index][0] == "Succession Dispute") {
        event.addEffect("military_morale", 0.9);
        event.addEffect("economy_treasury", 0.95);
    }

    return event;
}

void Event::saveToFile(std::ofstream& outFile) const {
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to save event data: File not open");
    }

    outFile << name << std::endl;
    outFile << description << std::endl;
    outFile << type << std::endl;
    outFile << duration << std::endl;
    outFile << currentDuration << std::endl;

    outFile << effects.size() << std::endl;

    for (const auto& effect : effects) {
        outFile << effect.first << std::endl;
        outFile << effect.second << std::endl;
    }
}

void Event::loadFromFile(std::ifstream& inFile) {
    if (!inFile.is_open()) {
        throw std::runtime_error("Failed to load event data: File not open");
    }

    std::string dummy;
    std::getline(inFile, dummy);

    std::getline(inFile, name);
    std::getline(inFile, description);
    std::getline(inFile, type);
    inFile >> duration;
    inFile >> currentDuration;

    effects.clear();

    int numEffects;
    inFile >> numEffects;

    std::getline(inFile, dummy);

    for (int i = 0; i < numEffects; i++) {
        std::string target;
        double multiplier;

        std::getline(inFile, target);
        inFile >> multiplier;

        effects[target] = multiplier;

        std::getline(inFile, dummy);
    }
}
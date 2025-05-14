#include "stronghold.h"
#include <iostream>
Market::Market() : offerCount(0) {
    for (int i = 0; i < 50; i++) {
        offers[i] = Offer(); // Correct: Initialize with `Offer`
    }
}


bool Market::acceptTrade(Kingdom& buyer, Kingdom& seller, int offerIndex) {
    try {
        if (offerIndex < 0 || offerIndex >= offerCount) {
            throw GameException("Invalid offer index.");
        }
        if (offers[offerIndex].sellerName != seller.getName()) {
            throw GameException("Offer not from specified seller.");
        }
        if (buyer.getName() == seller.getName()) {
            throw GameException("Cannot trade with self.");
        }
        Resource& sellerResource = seller.getResource(offers[offerIndex].item.getItemType());
        Resource& buyerResource = buyer.getResource(offers[offerIndex].item.getItemType());
        if (sellerResource.getAmount() < offers[offerIndex].item.getQuantity()) {
            throw GameException("Seller has insufficient " + offers[offerIndex].item.getItemType() + ".");
        }
        if (buyer.getEconomy().getTreasury() < offers[offerIndex].item.getGoldCost()) {
            throw GameException("Buyer has insufficient gold.");
        }
        sellerResource.setAmount(sellerResource.getAmount() - offers[offerIndex].item.getQuantity());
        buyerResource.setAmount(buyerResource.getAmount() + offers[offerIndex].item.getQuantity());
        buyer.getEconomy().payExpense(offers[offerIndex].item.getGoldCost(), "Trade purchase");
        seller.getEconomy().addIncome(offers[offerIndex].item.getGoldCost(), "Trade sale");
        std::cout << buyer.getName() << " accepts trade: " << offers[offerIndex].item.getQuantity()
            << " " << offers[offerIndex].item.getItemType() << " for " << offers[offerIndex].item.getGoldCost()
            << " gold from " << seller.getName() << ".\n";
        for (int i = offerIndex; i < offerCount - 1; i++) {
            offers[i] = offers[i + 1];
        }
        offers[offerCount - 1] = Offer();
        offerCount--;
    }
    catch (const GameException& e) {
        std::cout << "Error accepting trade: " << e.getMessage() << std::endl;
    }
    catch (...) {
        std::cout << "Unknown error in acceptTrade.\n";
    }
}

bool Market::smuggle(Kingdom& smuggler, Kingdom& receiver, const TradeItem<int>& item) {
    try {
        if (item.getQuantity() <= 0) {
            throw GameException("Smuggle quantity must be positive.");
        }
        if (smuggler.getName() == receiver.getName()) {
            throw GameException("Cannot smuggle to self.");
        }

        Resource& smugglerResource = smuggler.getResource(item.getItemType());
        Resource& receiverResource = receiver.getResource(item.getItemType());

        if (smugglerResource.getAmount() < item.getQuantity()) {
            throw GameException("Insufficient " + item.getItemType() + " to smuggle.");
        }

        int risk = rand() % 100;
        int riskThreshold = 20 + (smuggler.getLeader()->getCorruption() / 5);

        if (risk < riskThreshold) {
            std::cout << smuggler.getName() << " caught smuggling " << item.getQuantity()
                << " " << item.getItemType() << " to " << receiver.getName() << "!\n";
            smugglerResource.setAmount(smugglerResource.getAmount() - item.getQuantity());
            smuggler.getEconomy().payExpense(100.0, "Smuggling fine");
            return false;  // Smuggling failed
        }

        // Successful smuggling
        smugglerResource.setAmount(smugglerResource.getAmount() - item.getQuantity());
        receiverResource.setAmount(receiverResource.getAmount() + item.getQuantity());

        std::cout << smuggler.getName() << " successfully smuggled " << item.getQuantity()
            << " " << item.getItemType() << " to " << receiver.getName() << ".\n";

        return true;  // Smuggling successful

    }
    catch (const GameException& e) {
        std::cout << "Error smuggling: " << e.getMessage() << std::endl;
        return false;  // Smuggling failed due to exception
    }
    catch (...) {
        std::cout << "Unknown error in smuggle.\n";
        return false; 
    }
}


void Market::offerTrade(Kingdom& seller, const TradeItem<int>& item) {
    try 
    {
        if (offerCount >= 50) {
            throw GameException("Market offer storage full.");
        }
        // ... other checks ...
        offers[offerCount].sellerName = seller.getName();
        offers[offerCount].item = item;
        offerCount++;
        std::cout << seller.getName() << " offers " << item.getQuantity() << " " << item.getItemType()
            << " for " << item.getGoldCost() << " gold.\n";
    }
    catch (const GameException& e) {
        std::cout << "Error offering trade: " << e.getMessage() << std::endl;
    }
}
void Market::displayMarket() const {
    std::cout << "\nMarket Offers:\n";
    if (offerCount == 0) {
        std::cout << "No offers available.\n";
        return;
    }
    for (int i = 0; i < offerCount; i++) {
        std::cout << (i + 1) << ". " << offers[i].sellerName << " offers "
            << offers[i].item.getQuantity() << " " << offers[i].item.getItemType()
            << " for " << offers[i].item.getGoldCost() << " gold.\n";
    }
}
bool Market::saveMarketLog(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile) {
        return false;
    }

    for (int i = 0; i < offerCount; i++) {
        outFile << offers[i].sellerName << " "
            << offers[i].item.getItemType() << " "
            << offers[i].item.getQuantity() << " "
            << offers[i].item.getGoldCost() << "\n";
    }

    outFile.close();
    return true;
}
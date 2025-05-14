#include "stronghold.h"

Communication::Communication() : messageCount(0) {
    for (int i = 0; i < 100; i++) {
        messages[i] = Message();
    }
}

void Communication::sendMessage(const Kingdom& sender, Kingdom& receiver, const std::string& message) {
    try {
        if (messageCount >= 100) {
            throw GameException("Message storage full.");
        }
        if (message.empty()) {
            throw GameException("Message cannot be empty.");
        }
        messages[messageCount].senderName = sender.getName();
        messages[messageCount].receiverName = receiver.getName();
        messages[messageCount].content = message;
        messageCount++;
        std::cout << sender.getName() << " sent message to " << receiver.getName() << ": " << message << "\n";
    }
    catch (const GameException& e) {
        std::cout << "Error sending message: " << e.getMessage() << std::endl;
    }
}

void Communication::displayMessages(const Kingdom& kingdom) const {
    std::cout << "\nMessages for " << kingdom.getName() << ":\n";
    bool hasMessages = false;
    for (int i = 0; i < messageCount; i++) {
        if (messages[i].receiverName == kingdom.getName() || messages[i].senderName == kingdom.getName()) {
            std::cout << messages[i].senderName << " to " << messages[i].receiverName << ": " << messages[i].content << "\n";
            hasMessages = true;
        }
    }
    if (!hasMessages) {
        std::cout << "No messages.\n";
    }
}

bool Communication::saveChatLog(const std::string& filename) const {
    try {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw GameException("Cannot open chat log file.");
        }
        for (int i = 0; i < messageCount; i++) {
            file << messages[i].senderName << " " << messages[i].receiverName << " " << messages[i].content << "\n";
        }
        file.close();
        std::cout << "Chat log saved to " << filename << std::endl;
        return true;
    }
    catch (const GameException& e) {
        std::cout << "Error saving chat log: " << e.getMessage() << std::endl;
        return false;
    }
}
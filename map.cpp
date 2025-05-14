#include "stronghold.h"

Map::Map(int width, int height) : width(width), height(height), positionCount(0) {
    for (int i = 0; i < 10; i++) {
        positions[i] = Position();
    }
}

void Map::placeKingdom(Kingdom& kingdom, int x, int y) {
    try {
        if (x < 0 || x >= width || y < 0 || y >= height) {
            throw GameException("Invalid map coordinates.");
        }
        if (positionCount >= 10) {
            throw GameException("Map position storage full.");
        }
        for (int i = 0; i < positionCount; i++) {
            if (positions[i].x == x && positions[i].y == y) {
                throw GameException("Position already occupied.");
            }
        }
        kingdom.setMapPosition(x, y);
        positions[positionCount].kingdomName = kingdom.getName();
        positions[positionCount].x = x;
        positions[positionCount].y = y;
        positionCount++;
        std::cout << kingdom.getName() << " placed at (" << x << ", " << y << ").\n";
    }
    catch (const GameException& e) {
        std::cout << "Error placing kingdom: " << e.getMessage() << std::endl;
    }
}

int Map::getDistance(const Kingdom& kingdom1, const Kingdom& kingdom2) const {
    try {
        int x1 = kingdom1.getMapX();
        int y1 = kingdom1.getMapY();
        int x2 = kingdom2.getMapX();
        int y2 = kingdom2.getMapY();
        return static_cast<int>(std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2)));
    }
    catch (const std::exception& e) {
        std::cout << "Error calculating distance: " << e.what() << std::endl;
        return -1;
    }
}

void Map::moveKingdom(Kingdom& kingdom, int newX, int newY) {
    try {
        if (newX < 0 || newX >= width || newY < 0 || newY >= height) {
            throw GameException("Invalid new map coordinates.");
        }
        for (int i = 0; i < positionCount; i++) {
            if (positions[i].x == newX && positions[i].y == newY) {
                throw GameException("New position already occupied.");
            }
        }
        for (int i = 0; i < positionCount; i++) {
            if (positions[i].kingdomName == kingdom.getName()) {
                positions[i].x = newX;
                positions[i].y = newY;
                kingdom.setMapPosition(newX, newY);
                std::cout << kingdom.getName() << " moved to (" << newX << ", " << newY << ").\n";
                return;
            }
        }
        throw GameException("Kingdom not found on map.");
    }
    catch (const GameException& e) {
        std::cout << "Error moving kingdom: " << e.getMessage() << std::endl;
    }
}

void Map::displayMap() const {
    std::cout << "\nGame Map (" << width << "x" << height << "):\n";
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bool occupied = false;
            for (int i = 0; i < positionCount; i++) {
                if (positions[i].x == x && positions[i].y == y) {
                    std::cout << positions[i].kingdomName[0] << " ";
                    occupied = true;
                    break;
                }
            }
            if (!occupied) {
                std::cout << ". ";
            }
        }
        std::cout << "\n";
    }
}
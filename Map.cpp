#include "Map.hpp"
#include "Game.hpp"
#include <fstream>
#include <iostream>
#include "./ECS/ECS.hpp"
#include "ECS/Components.hpp"

Map::Map(std::string tID, int ms, int ts, Manager& man) : texID(tID), mapScale(ms), tileSize(ts), manager(man) {
    scaledSize = ms * ts;
}

Map::~Map() {

}

void Map::LoadMap(std::string path, int sizeX, int sizeY) {
    char c;
    std::fstream mapFile;
    mapFile.open(path);
    
    if (!mapFile.is_open()) {
        std::cout << "Failed to open map file: " << path << std::endl;
        return;
    }

    int srcX, srcY;

    for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
            int tileId;
            mapFile >> tileId;
            
            srcY = (tileId / 10) * tileSize;
            srcX = (tileId % 10) * tileSize;
            
            AddTile(srcX, srcY, x * scaledSize, y * scaledSize);
            
            if (mapFile.peek() == ',') 
                mapFile.ignore(1, ',');
            else if (mapFile.peek() == ';') 
                mapFile.ignore(1, ';');
        }
        
        if (mapFile.peek() == '\n' || mapFile.peek() == '\r') {
            while (mapFile.peek() == '\n' || mapFile.peek() == '\r') {
                mapFile.ignore(1, mapFile.peek());
            }
        }
    }

    for(int y = 0; y < sizeY; y++) {
        for(int x = 0; x < sizeX; x++) {
            mapFile.get(c);
            if(c == '1') {
                auto& tcol(manager.addEntity());
                tcol.addComponent<ColliderComponent>("terrain", x * scaledSize, y * scaledSize, scaledSize);
                tcol.addGroup(Game::groupColliders);
            }
            mapFile.ignore();
        }
    }
    mapFile.close();
}

void Map::AddTile(int srcX, int srcY, int xpos, int ypos) {
    auto& tile(manager.addEntity());
    tile.addComponent<TileComponent>(srcX, srcY, xpos, ypos, tileSize, mapScale, texID);
    tile.addGroup(Game::groupMap);
}
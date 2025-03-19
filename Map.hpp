#pragma once

#include <string>
#include "ECS/ECS.hpp"

class Map {
    public:
        Map(std::string tID, int ms, int ts, Manager& man);
        ~Map();

        void LoadMap(std::string path, int sizeX, int sizeY);
        void AddTile(int srcX, int srcY, int xpos, int ypos);

    private:
        std::string texID;
        int mapScale;
        int tileSize;
        int scaledSize;
        Manager& manager;
};
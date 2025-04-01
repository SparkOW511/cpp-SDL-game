all:
	g++ -I ./src/include -L ./src/lib -o main main.cpp Game.cpp TextureManager.cpp Map.cpp Vector2D.cpp Collision.cpp ECS.cpp AssetManager.cpp RandomPositionManager.cpp TransitionManager.cpp -lmingw32 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2 -O3
d:
	g++ -I ./src/include -L ./src/lib -o main main.cpp Game.cpp TextureManager.cpp Map.cpp Vector2D.cpp Collision.cpp ECS.cpp AssetManager.cpp RandomPositionManager.cpp TransitionManager.cpp -lmingw32 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2 -Og
	./main.exe


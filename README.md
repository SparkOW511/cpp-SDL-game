# Johnny English Game

A 2D action-adventure game built with C++ and SDL2, featuring multiple levels, enemy AI, collectibles, and a complete game progression system.

## 🎮 Game Overview

Johnny English is a top-down 2D game where players navigate through multiple levels, collect clues, battle enemies, and rescue a scientist. The game features a complete menu system, save/load functionality, leaderboards, and replay system.

## ✨ Features

### Core Gameplay
- **Multi-level progression** - 4 unique levels with increasing difficulty
- **Combat system** - Fight enemies with projectile-based combat
- **Collectibles** - Gather clues, magazines, and health potions
- **Boss battles** - Face challenging final boss encounters
- **NPC interactions** - Question-based puzzle system with multiple choice answers

### Game Systems
- **Entity Component System (ECS)** - Modular and efficient game architecture
- **Save/Load system** - Continue your progress anytime
- **Replay system** - Watch replays of your gameplay
- **Leaderboard** - Track high scores and completion times
- **Audio system** - Background music and sound effects with volume control
- **Collision detection** - Precise collision system for gameplay interactions

### User Interface
- **Main menu** - New game, load game, settings, leaderboard
- **Pause menu** - In-game pause with save functionality
- **Settings menu** - Volume control and keybind information
- **Game over/victory screens** - Complete game state management

## 🛠️ Technical Details

### Architecture
- **Language**: C++
- **Graphics**: SDL2, SDL2_image
- **Audio**: SDL2_mixer
- **Text Rendering**: SDL2_ttf
- **Build System**: Makefile with MinGW

### Key Components
- **Game Engine**: Custom game loop with 60 FPS target
- **ECS System**: Entity Component System for game objects
- **Asset Management**: Centralized texture and audio loading
- **State Management**: Multiple game states (menu, gameplay, pause, etc.)
- **Camera System**: 2D camera with player following
- **Animation System**: Sprite-based animations

## 📋 Prerequisites

### Windows (MinGW)
- MinGW-w64 compiler
- SDL2 development libraries
- Make utility

### Required SDL2 Libraries
- SDL2
- SDL2_image
- SDL2_ttf
- SDL2_mixer

## 🚀 Building and Running

### Build Commands
```bash
# Build optimized version
make all

# Build debug version and run
make d
```

### Manual Build
```bash
g++ -I ./src/include -L ./src/lib -o JohnnyEnglish main.cpp Game.cpp TextureManager.cpp Map.cpp Vector2D.cpp Collision.cpp ECS.cpp AssetManager.cpp RandomPositionManager.cpp TransitionManager.cpp -lmingw32 -lSDL2main -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lSDL2 -O3
```

### Running the Game
```bash
./JohnnyEnglish.exe
```

## 🎯 How to Play

### Controls
- **Movement**: Arrow keys or WASD
- **Shoot**: Spacebar
- **Pause**: ESC
- **Menu Navigation**: Arrow keys + Enter

### Objective
1. Navigate through 4 levels
2. Collect clues by answering questions correctly
3. Gather magazines for ammunition
4. Find health potions to restore health
5. Defeat enemies and the final boss
6. Rescue the scientist to complete the game

### Game Progression
- **Level 1-3**: Collect clues and battle enemies
- **Level 4**: Face the final boss and rescue the scientist
- **Victory Condition**: Defeat the final boss and rescue the scientist

## 🎨 Assets

The game includes:
- **Sprites**: Character animations, enemies, objects
- **Audio**: Background music and sound effects
- **Fonts**: Custom game font (MINECRAFT.TTF)
- **Maps**: Level layouts and collision data
- **UI Elements**: Menu backgrounds and interface graphics

## 💾 Save System

- **Auto-save**: Game progress is automatically saved
- **Manual save**: Save from pause menu
- **Save data**: Includes level progress, collected items, and game state
- **Load game**: Continue from main menu

## 🏆 Leaderboard

- Tracks completion times
- Stores player names and scores
- Persistent storage in `leaderboard.txt`
- Accessible from main menu

## 📝 License

This project is for educational purposes. SDL2 libraries are used under their respective licenses.

## 🤝 Contributing

This appears to be a personal/educational project. For modifications:
1. Fork the repository
2. Create feature branches
3. Test thoroughly
4. Submit pull requests

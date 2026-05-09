#pragma once
#include "Common.h"
#include "InputManager.h"
#include "GameAssets.h"
#include "SoundManager.h"
#include "GameObject.h"

class Player;
class Boss;

class Game {
private:
    bool running;
    GameState state;
    InputManager input;
    GameAssets assets;
    SoundManager sounds;

    std::vector<std::unique_ptr<GameObject>> objects;
    std::vector<std::unique_ptr<GameObject>> pendingObjects;

    Player* playerRef;
    Boss* bossRef;

    int shakeTimer, shakeIntensity;
    ExMessage lastMouseMsg;
    bool mouseClickedThisFrame;
    int fadeAlpha;

    bool bgmEnabled;

public:
    Game() : running(true), state(MENU), playerRef(nullptr), bossRef(nullptr), shakeTimer(0), shakeIntensity(0), mouseClickedThisFrame(false), fadeAlpha(0), bgmEnabled(true) {
        memset(&lastMouseMsg, 0, sizeof(ExMessage));
    }

    bool isRunning() const { return running; }
    void setRunning(bool r) { running = r; }
    GameState getState() const { return state; }
    void setState(GameState s) { state = s; }
    InputManager& getInput() { return input; }
    GameAssets& getAssets() { return assets; }
    SoundManager& getSounds() { return sounds; }
    const ExMessage& getLastMouseMsg() const { return lastMouseMsg; }
    bool isMouseClicked() const { return mouseClickedThisFrame; }
    const std::vector<std::unique_ptr<GameObject>>& getObjects() const { return objects; }

    bool isBgmEnabled() const { return bgmEnabled; }
    void toggleBgm() { bgmEnabled = !bgmEnabled; }

    void triggerShake(int intensity, int frames) { shakeIntensity = intensity; shakeTimer = frames; }
    void addObject(std::unique_ptr<GameObject> obj) { pendingObjects.push_back(std::move(obj)); }

    Player* getPlayer() const { return playerRef; }

    void init();
    void update();
    void resolveCollisions();
    void draw();
    void run();
};

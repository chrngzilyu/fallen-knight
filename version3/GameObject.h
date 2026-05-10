#pragma once
#include "Common.h"

class Game;

class GameObject {
protected:
    Game* game;
    float x, y, width, height;
    bool isDead;
public:
    GameObject(Game* g, float startX, float startY, float w, float h) : game(g), x(startX), y(startY), width(w), height(h), isDead(false) {}
    virtual ~GameObject() = default;
    virtual void update() = 0;
    virtual void draw() = 0;

    virtual bool shouldRemoveOnDeath() const { return true; }
    bool getIsDead() const { return isDead; }
    void kill() { isDead = true; }

    virtual Rect getHitbox() const { return { x, y, width, height }; }
    float getX() const { return x; }
};

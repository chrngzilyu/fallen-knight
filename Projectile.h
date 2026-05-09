#pragma once
#include "GameObject.h"

class Projectile : public GameObject {
private:
    float vx;
public:
    Projectile(Game* g, float x, float y, float speed) : GameObject(g, x, y, 45, 45), vx(speed) {}
    void update() override {
        x += vx;
        if (x < -300 || x > SCREEN_WIDTH + 300) isDead = true;
    }
    void draw() override {
        setlinecolor(RGB(150, 255, 150));
        setfillcolor(RGB(50, 200, 100));
        fillcircle((int)(x + width / 2), (int)(y + height / 2), 22);
        setfillcolor(WHITE);
        fillcircle((int)(x + width / 2), (int)(y + height / 2), 9);
    }
};

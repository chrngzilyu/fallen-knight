#pragma once
#include "Common.h"

class InputManager {
private:
    bool current[256] = { false };
    bool previous[256] = { false };
public:
    void update() {
        for (int i = 0; i < 256; i++) {
            previous[i] = current[i];
            current[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
        }
    }
    bool isKeyDown(int key) const { return current[key]; }
    bool isKeyPressed(int key) const { return current[key] && !previous[key]; }
    bool isKeyReleased(int key) const { return !current[key] && previous[key]; }
};

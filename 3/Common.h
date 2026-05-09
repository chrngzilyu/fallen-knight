#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <unordered_set>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <graphics.h>
#include <conio.h>
#include <windows.h>
#include <mmsystem.h>
#include <tchar.h>

#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "winmm.lib")

#if defined(_DEBUG)
#define ENABLE_UNIT_TESTS
#endif

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
const float GROUND_Y = 620.0f;

extern TCHAR g_audioErrorMsg[256];

enum GameState { MENU, PLAYING, PLAYER_DYING, GAME_OVER, BOSS_DYING, VICTORY };

namespace BossConfig {
    constexpr int MAX_HP = 180;
    constexpr float WANDER_SPEED = 2.8f;
    constexpr float EVADE_SPEED = 10.1f;
    constexpr float POUNCE_SPEED_X = 7.9f;
    constexpr float POUNCE_SPEED_Y = -13.5f;

    constexpr int SCRATCH_RANGE = 195;
    constexpr int POUNCE_RANGE = 600;
    constexpr float ARENA_MARGIN = 75.0f;

    constexpr int LASER_FRAME = 21;
    constexpr int LASER_DAMAGE_FRAME = 53;
}

struct Rect { float x, y, w, h; };

bool checkAABB(Rect a, Rect b);
void putimageAlpha(int x, int y, IMAGE* img);

#pragma once
#include "Common.h"

struct GameAssets {
    IMAGE bg, knight_r, knight_l, boss_r, boss_l;
    IMAGE arm_proj_r, arm_proj_l;
    IMAGE laser_sheet_r, laser_sheet_l;

    void loadAll() {
        loadimage(&bg, _T("bg.png"), SCREEN_WIDTH, SCREEN_HEIGHT);
        loadimage(&knight_r, _T("knight_r.png"));
        loadimage(&knight_l, _T("knight_l.png"));
        loadimage(&boss_r, _T("boss_r.png"));
        loadimage(&boss_l, _T("boss_l.png"));
        loadimage(&arm_proj_r, _T("arm_projectile_r.png"));
        loadimage(&arm_proj_l, _T("arm_projectile_l.png"));
        loadimage(&laser_sheet_r, _T("Laser_sheet_r.png"));
        loadimage(&laser_sheet_l, _T("Laser_sheet_l.png"));
    }
};

#include "Game.h"
#include "Player.h"
#include "Boss.h"
#include "UI.h"

void Game::init() {
    objects.clear(); pendingObjects.clear();

    auto p = std::make_unique<Player>(this, 200.0f, GROUND_Y - 90.0f);
    playerRef = p.get(); objects.push_back(std::move(p));

    auto b = std::make_unique<Boss>(this, 980.0f, GROUND_Y - 120.0f);
    bossRef = b.get(); objects.push_back(std::move(b));

    shakeTimer = 0; fadeAlpha = 0; state = PLAYING;

    if (bgmEnabled) {
        sounds.playBGM(_T("sounds\\bgm.mp3"));
    }
}

void Game::resolveCollisions() {
    if (playerRef && !playerRef->getIsDead()) {
        playerRef->checkOffensiveCollisions();
        playerRef->checkDefensiveCollisions();
    }
}

void Game::update() {
    if (state == PLAYING) {
        if (bossRef && !bossRef->getIsDead() && playerRef) bossRef->updateTarget(playerRef);
        for (auto& obj : objects) { if (!obj->getIsDead()) obj->update(); }
        resolveCollisions();

        if (playerRef && playerRef->getHp() <= 0) { state = PLAYER_DYING; fadeAlpha = 0; }
        else if (bossRef && bossRef->getHp() <= 0) { state = BOSS_DYING; fadeAlpha = 0; }

        for (auto& obj : pendingObjects) objects.push_back(std::move(obj));
        pendingObjects.clear();
        objects.erase(
            std::remove_if(objects.begin(), objects.end(), [](const std::unique_ptr<GameObject>& obj) { return obj->getIsDead() && obj->shouldRemoveOnDeath(); }),
            objects.end()
        );
    }
    else if (state == PLAYER_DYING) {
        if (playerRef) playerRef->update();
        fadeAlpha = (std::min)(220, fadeAlpha + 2);
        if (fadeAlpha >= 220 && playerRef->isDeathAnimFinished()) state = GAME_OVER;
    }
    else if (state == BOSS_DYING) {
        if (playerRef) playerRef->update();
        if (bossRef) bossRef->update();
        fadeAlpha = (std::min)(180, fadeAlpha + 2);
        if (bossRef->isDeathAnimFinished()) state = VICTORY;
    }
}

void Game::draw() {
    BeginBatchDraw();
    cleardevice();

    if (state == MENU) {
        if (assets.bg.getwidth() > 0) putimageAlpha(0, 0, &assets.bg);
        else { setfillcolor(RGB(200, 200, 200)); solidrectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT); }

        DrawStylizedText(220, 150, _T("F A L L E N   K N I G H T"), RGB(0, 0, 0), 90, _T("Georgia"), 800, true);

        setlinecolor(RGB(80, 80, 80)); setlinestyle(PS_SOLID, 2);
        line(280, 280, 1000, 280);
        setfillcolor(RGB(80, 80, 80));
        solidcircle(640, 280, 4);

        if (DrawPremiumButton(this, 440, 310, 400, 70, _T("开 始 游 戏"))) {
            sounds.playSfx(_T("ui_confirm"));
            init();
        }
        if (DrawPremiumButton(this, 440, 400, 400, 70, bgmEnabled ? _T("音 乐 : 开") : _T("音 乐 : 关"))) {
            sounds.playSfx(_T("ui_confirm"));
            bgmEnabled = !bgmEnabled;
        }
        if (DrawPremiumButton(this, 440, 490, 400, 70, _T("退 出 游 戏"))) {
            sounds.playSfx(_T("ui_cancel"));
            running = false;
        }
        DrawCenteredText(580, _T("◆ 操 作 指 南 ◆"), RGB(60, 60, 60), 24, _T("微软雅黑"), 800, false);
        DrawCenteredText(620, _T("[ WASD / 方向键 ] 移动    [ Space ] 跳跃 / 二段跳    [ K ] 冲刺避险"), RGB(60, 60, 60), 22, _T("微软雅黑"), 400, false);
        DrawCenteredText(650, _T("[ J ] 轻击    [ 长按 J ] 蓄力重斩    [ 空中 S(或↓) + J ] 下劈反弹"), RGB(60, 60, 60), 22, _T("微软雅黑"), 400, false);
        DrawCenteredText(680, _T("[ L ] 完美格挡 (瞬间弹反Boss并回复精力)"), RGB(60, 60, 60), 22, _T("微软雅黑"), 400, false);
    }
    else {
        int offsetX = 0, offsetY = 0;
        if (shakeTimer > 0) {
            shakeTimer--;
            offsetX = (rand() % shakeIntensity) - (shakeIntensity / 2);
            offsetY = (rand() % shakeIntensity) - (shakeIntensity / 2);
        }
        setorigin(offsetX, offsetY);

        if (assets.bg.getwidth() > 0) putimageAlpha(0, 0, &assets.bg);
        else {
            setfillcolor(RGB(200, 200, 200)); solidrectangle(0, (int)GROUND_Y, SCREEN_WIDTH, SCREEN_HEIGHT);
            setlinecolor(RGB(150, 150, 150)); line(0, (int)GROUND_Y + 20, SCREEN_WIDTH, (int)GROUND_Y + 20);
        }

        for (auto& obj : objects) obj->draw();
        setorigin(0, 0);

        if (state == PLAYER_DYING || state == GAME_OVER || state == BOSS_DYING || state == VICTORY) {
            IMAGE fadeImg(1, 1);
            SetWorkingImage(&fadeImg);
            setfillcolor(BLACK); solidrectangle(0, 0, 1, 1);
            SetWorkingImage(NULL);

            BLENDFUNCTION bf = { AC_SRC_OVER, 0, (BYTE)fadeAlpha, AC_SRC_ALPHA };
            AlphaBlend(GetImageHDC(NULL), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetImageHDC(&fadeImg), 0, 0, 1, 1, bf);
        }

        if (state == PLAYING || state == BOSS_DYING) {
            if (playerRef && bossRef) {
                DrawStylizedText(25, 20, _T("HP"), RGB(200, 30, 30), 28, _T("Georgia"), 800, true);
                for (int i = 0; i < playerRef->getMaxHp(); i++) {
                    int cx = 90 + i * 32;
                    int cy = 35;
                    int r = 12;

                    // 绘制血槽底框
                    setlinecolor(RGB(80, 20, 20));
                    setlinestyle(PS_SOLID, 2);
                    setfillcolor(RGB(40, 20, 20));
                    fillcircle(cx, cy, r);

                    if (i < playerRef->getHp()) {
                        setfillcolor(RGB(220, 30, 30));
                        solidcircle(cx, cy, r - 2);
                        setfillcolor(RGB(255, 120, 120)); // 高光提升质感
                        solidcircle(cx - 3, cy - 3, 3);
                    }
                }

                DrawStylizedText(25, 60, _T("SP"), RGB(40, 180, 80), 28, _T("Georgia"), 800, true);
                int spX = 75, spY = 64, spW = 220, spH = 14;

                setlinecolor(RGB(40, 80, 40));
                setlinestyle(PS_SOLID, 2);
                setfillcolor(RGB(20, 40, 20));
                fillroundrect(spX, spY, spX + spW, spY + spH, 10, 10); // 圆角底框

                if (playerRef->getStamina() > 0) {
                    COLORREF spColor = RGB(60, 200, 100);
                    if (playerRef->getStamina() < 15.0f && !input.isKeyDown('L')) {
                        spColor = RGB(220, 50, 50); // 危险状态变红
                    }
                    int curSpW = (int)((spW - 4) * playerRef->getStamina() / playerRef->getMaxStamina());
                    setfillcolor(spColor);
                    solidroundrect(spX + 2, spY + 2, spX + 2 + curSpW, spY + spH - 2, 6, 6);
                }
                if (bossRef->getHp() > 0) {
                    int bw = 800; int bh = 18;
                    int bx = (SCREEN_WIDTH - bw) / 2; int by = SCREEN_HEIGHT - 50;

                    TCHAR bossNameTitle[128];
                    if (bossRef->getCurrentPhase() == 1) _stprintf_s(bossNameTitle, 128, _T("耄 耋"));
                    else if (bossRef->getCurrentPhase() == 2) _stprintf_s(bossNameTitle, 128, _T("耄 耋   [ P H A S E  II ]"));
                    else _stprintf_s(bossNameTitle, 128, _T("耄 耋   [ P H A S E  III ]"));

                    DrawStylizedText(bx + bw / 2 - 130, by - 35, bossNameTitle, RGB(200, 30, 30), 28, _T("Georgia"), 800, true);

                    POINT bgPts[] = {
                        {bx - 20, by + bh}, {bx, by}, {bx + bw, by}, {bx + bw + 20, by + bh}
                    };
                    setlinecolor(RGB(80, 20, 20));
                    setlinestyle(PS_SOLID, 2);
                    setfillcolor(RGB(30, 10, 10));
                    fillpolygon(bgPts, 4);

                    
                    float hpRatio = (float)bossRef->getHp() / bossRef->getMaxHp();
                    int curBw = (int)(bw * hpRatio);
                    if (curBw > 0) {
                        POINT hpPts[] = {
                            {bx - 17, by + bh - 2},
                            {bx + 2, by + 2},
                            {bx + 2 + curBw, by + 2},
                            {bx - 17 + curBw, by + bh - 2}
                        };
                        if (bossRef->getCurrentPhase() == 3) setfillcolor(RGB(255, 40, 40));
                        else setfillcolor(RGB(180, 30, 30));
                        solidpolygon(hpPts, 4);
                    }
                }
            }
        }

        if (state == GAME_OVER) {
            DrawStylizedText(400, 220, _T("Y O U   D I E D"), RGB(180, 20, 20), 100, _T("Georgia"), 800, true);
            setlinecolor(RGB(80, 20, 20)); setlinestyle(PS_SOLID, 2);
            line(350, 360, 930, 360);

            if (DrawPremiumButton(this, 440, 440, 400, 70, _T("重 塑 肉 身"))) {
                sounds.playSfx(_T("ui_confirm"));
                init();
            }
            if (DrawPremiumButton(this, 440, 540, 400, 70, _T("放 弃 使 命"))) {
                sounds.playSfx(_T("ui_cancel"));
                sounds.stopBGM();
                state = MENU;
            }
        }
        else if (state == VICTORY) {
            DrawStylizedText(210, 220, _T("G R E A T   E N E M Y   F E L L E D"), RGB(0, 0, 0), 70, _T("Georgia"), 800, true);
            setlinecolor(RGB(80, 80, 80)); setlinestyle(PS_SOLID, 2);
            line(250, 360, 1030, 360);

            if (DrawPremiumButton(this, 440, 440, 400, 70, _T("再 战 一 次"))) {
                sounds.playSfx(_T("ui_confirm"));
                init();
            }
            if (DrawPremiumButton(this, 440, 540, 400, 70, _T("归 于 沉 寂"))) {
                sounds.playSfx(_T("ui_cancel"));
                sounds.stopBGM();
                state = MENU;
            }
        }

        if (_tcslen(g_audioErrorMsg) > 0) {
            DrawStylizedText(15, 15, g_audioErrorMsg, RGB(255, 50, 50), 20, _T("微软雅黑"), 800, false);
        }
    }

    EndBatchDraw();
}

void Game::run() {
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    LARGE_INTEGER previousTime;
    QueryPerformanceCounter(&previousTime);

    const double LOGIC_FRAME_TIME = 1.0 / 60.0;
    double lag = 0.0;

    while (running) {
        LARGE_INTEGER currentTime;
        QueryPerformanceCounter(&currentTime);
        double elapsedTime = (double)(currentTime.QuadPart - previousTime.QuadPart) / frequency.QuadPart;
        previousTime = currentTime;

        if (elapsedTime > 0.25) elapsedTime = 0.25;
        lag += elapsedTime;

        MSG wmsg;
        while (PeekMessage(&wmsg, NULL, 0, 0, PM_REMOVE)) {
            if (wmsg.message == WM_QUIT) break;
            TranslateMessage(&wmsg);
            DispatchMessage(&wmsg);
        }

        mouseClickedThisFrame = false;
        while (peekmessage(&lastMouseMsg, EM_MOUSE)) {
            if (lastMouseMsg.message == WM_LBUTTONDOWN) mouseClickedThisFrame = true;
        }

        while (lag >= LOGIC_FRAME_TIME) {
            input.update();
            if (input.isKeyPressed(VK_ESCAPE)) running = false;
            update();
            lag -= LOGIC_FRAME_TIME;
        }

        draw();
        Sleep(1);
    }
}

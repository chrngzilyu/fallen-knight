#pragma once
#include "Player.h"
#include "Projectile.h"

class Boss : public Entity {
private:
    enum State { WANDER, SCRATCH, POUNCE, HISS_SHOOT, EVADE, TELEGRAPH_SCRATCH, TELEGRAPH_POUNCE, LASER, DEATH };
    State currentState;
    State lastMove;
    int stateTimer;

    Player* targetPlayer;
    float targetX;
    int currentPhase;

    State chooseNextState(Player* p) {
        if (!p) return TELEGRAPH_POUNCE;
        float dist = fabs(p->getX() - x);

        float sc_scratch = 0, sc_pounce = 0, sc_shoot = 0, sc_laser = 0, sc_evade = 0;

        if (dist < 200.0f) {
            sc_scratch = 80; sc_evade = 40; sc_pounce = 20;
        }
        else if (dist < 500.0f) {
            sc_pounce = 80; sc_shoot = 50; sc_evade = 20;
        }
        else {
            sc_laser = 70; sc_shoot = 70; sc_pounce = 60;
        }

        if (currentPhase == 3) {
            sc_laser += 20;
            sc_pounce += 20;
        }

        if (p->getRecentJumps() > 2.0f) { sc_laser += 30; sc_shoot += 30; }
        if (p->getRecentParries() > 1.5f) { sc_evade += 40; sc_pounce += 30; }
        if (p->getRecentDashes() > 2.0f) { sc_shoot += 40; sc_laser += 30; }
        if (p->getRecentAttacks() > 3.0f) { sc_evade += 60; sc_scratch += 20; }

        if (sc_scratch > 0) sc_scratch += rand() % 30;
        if (sc_pounce > 0) sc_pounce += rand() % 30;
        if (sc_shoot > 0) sc_shoot += rand() % 30;
        if (sc_laser > 0) sc_laser += rand() % 30;
        if (sc_evade > 0) sc_evade += rand() % 30;

        float maxSc = 0; State best = TELEGRAPH_POUNCE;
        if (sc_scratch > maxSc) { maxSc = sc_scratch; best = TELEGRAPH_SCRATCH; }
        if (sc_pounce > maxSc) { maxSc = sc_pounce; best = TELEGRAPH_POUNCE; }
        if (sc_shoot > maxSc) { maxSc = sc_shoot; best = HISS_SHOOT; }
        if (sc_laser > maxSc) { maxSc = sc_laser; best = LASER; }
        if (sc_evade > maxSc) { maxSc = sc_evade; best = EVADE; }

        return best;
    }

public:
    Rect currentAttackBox;

    Boss(Game* g, float startX, float startY) : Entity(g, startX, startY, 90.0f, 120.0f, BossConfig::MAX_HP) {
        currentState = WANDER; lastMove = WANDER; stateTimer = 0; vx = 0;
        targetPlayer = nullptr; targetX = 0; currentPhase = 1;
        currentAttackBox = { 0,0,0,0 };

        animator.init(&g->getAssets().boss_r, &g->getAssets().boss_l, 120, 120, 1.5f);
        animator.addAnimation("Idle", 0, 0, 4, 11, true);
        animator.addAnimation("Walk", 1, 0, 7, 9, true);
        animator.addAnimation("Pounce", 2, 0, 9, 7, false);
        animator.addAnimation("Hiss", 3, 0, 3, 11, false);
        animator.addAnimation("Laser", 3, 0, 3, 21, false);
        animator.addAnimation("Scratch", 4, 3, 4, 7, false);
        animator.addAnimation("Evade", 5, 0, 7, 7, false);
        animator.addAnimation("Hurt", 6, 0, 4, 9, false);
        animator.addAnimation("Death", 7, 0, 7, 11, false);
    }

    bool shouldRemoveOnDeath() const override { return false; }
    int getCurrentPhase() const { return currentPhase; }

    void updateTarget(Player* p) {
        targetPlayer = p;
        if (!targetPlayer) return;
        targetX = targetPlayer->getX();

        if (currentState == HISS_SHOOT || currentState == TELEGRAPH_SCRATCH ||
            currentState == TELEGRAPH_POUNCE || currentState == LASER) {
            facingRight = (targetX > x);
        }
    }

    void update() override {
        if (game->getState() == PLAYER_DYING || game->getState() == GAME_OVER) {
            animator.play("Idle"); animator.update(); return;
        }

        if (currentState == DEATH) {
            animator.play("Death");
            if (animator.getIsFinished()) isDead = true;
            animator.update(); return;
        }

        if (hp <= 0 && currentState != DEATH) {
            currentState = DEATH; currentAttackBox = { 0,0,0,0 };
            animator.play("Death"); hitFreezeTimer = 0; return;
        }

        if (hitFreezeTimer > 0) {
            hitFreezeTimer--;
            if (invincibilityTimer > 0) invincibilityTimer--;
            currentAttackBox = { 0,0,0,0 };
            animator.play("Hurt"); animator.update();
            return;
        }

        float hpRatio = (float)hp / maxHp;
        int decisionLimit = 71;
        float currentSpeed = BossConfig::WANDER_SPEED;

        if (hpRatio <= 0.3f) {
            currentPhase = 3;
            decisionLimit = 40;
            currentSpeed *= 1.4f;
        }
        else if (hpRatio <= 0.6f) {
            currentPhase = 2;
            decisionLimit = 55;
            currentSpeed *= 1.2f;
        }
        else {
            currentPhase = 1;
            decisionLimit = 71;
        }

        if (invincibilityTimer > 0) invincibilityTimer--;
        currentAttackBox = { 0,0,0,0 };

        stateTimer++;
        switch (currentState) {
        case WANDER: {
            animator.play("Walk");
            vy += gravity; y += vy;
            if (y + height >= GROUND_Y) { y = GROUND_Y - height; vy = 0; }

            if (x <= BossConfig::ARENA_MARGIN) {
                x = BossConfig::ARENA_MARGIN; facingRight = true;
            }
            else if (x >= SCREEN_WIDTH - BossConfig::ARENA_MARGIN - width) {
                x = SCREEN_WIDTH - BossConfig::ARENA_MARGIN - width; facingRight = false;
            }
            vx = facingRight ? currentSpeed : -currentSpeed;
            x += vx;

            if (stateTimer > decisionLimit) {
                if (!targetPlayer) return;
                State nextMove = chooseNextState(targetPlayer);
                currentState = nextMove;
                lastMove = nextMove;
                stateTimer = 0; vx = 0;
            }
            break;
        }

        case TELEGRAPH_SCRATCH:
            if (stateTimer == 1) game->getSounds().playSfx(_T("hiss"));
            animator.play("Hiss");
            vx = 0; vy += gravity; y += vy;
            if (y + height >= GROUND_Y) { y = GROUND_Y - height; vy = 0; }
            if (animator.getIsFinished()) { currentState = SCRATCH; stateTimer = 0; }
            break;

        case TELEGRAPH_POUNCE:
            if (stateTimer == 1) game->getSounds().playSfx(_T("hiss"));
            animator.play("Hiss");
            vx = 0; vy += gravity; y += vy;
            if (y + height >= GROUND_Y) { y = GROUND_Y - height; vy = 0; }
            if (animator.getIsFinished()) { currentState = POUNCE; stateTimer = 0; }
            break;

        case SCRATCH:
            animator.play("Scratch");
            vx = 0; vy += gravity; y += vy;
            if (y + height >= GROUND_Y) { y = GROUND_Y - height; vy = 0; }

            if (stateTimer > 4 && stateTimer < 24) {
                currentAttackBox = facingRight ?
                    Rect{ x + width, y + 30.0f, 90.0f, height - 30.0f } :
                    Rect{ x - 90.0f, y + 30.0f, 90.0f, height - 30.0f };
            }
            if (animator.getIsFinished()) { currentState = WANDER; stateTimer = 0; }
            break;

        case POUNCE:
            animator.play("Pounce");
            if (stateTimer == 1) {
                vy = BossConfig::POUNCE_SPEED_Y;
                vx = facingRight ? BossConfig::POUNCE_SPEED_X : -BossConfig::POUNCE_SPEED_X;
            }
            vy += gravity; x += vx; y += vy;

            currentAttackBox = { x - 15.0f, y, width + 30.0f, height };

            if (y + height >= GROUND_Y && vy > 0) {
                y = GROUND_Y - height; vy = 0; vx = 0;
                game->triggerShake(8, 10);
                if (animator.getIsFinished()) { currentState = WANDER; stateTimer = 0; }
            }
            break;

        case EVADE:
            animator.play("Evade");
            if (stateTimer == 1) {
                vx = facingRight ? -BossConfig::EVADE_SPEED : BossConfig::EVADE_SPEED;
            }
            vx *= 0.95f; x += vx;
            vy += gravity; y += vy;
            if (y + height >= GROUND_Y) { y = GROUND_Y - height; vy = 0; }

            invincibilityTimer = 4;
            if (animator.getIsFinished()) { currentState = WANDER; stateTimer = 0; vx = 0; }
            break;

        case HISS_SHOOT:
            if (stateTimer == 1) game->getSounds().playSfx(_T("hiss"));
            animator.play("Hiss");
            vx = 0; vy += gravity; y += vy;
            if (y + height >= GROUND_Y) { y = GROUND_Y - height; vy = 0; }

            if (stateTimer == 27) {
                game->addObject(std::make_unique<Projectile>(game, facingRight ? x + width : x - 60, y + 45, facingRight ? 9.0f : -9.0f));
            }
            if (animator.getIsFinished()) { currentState = WANDER; stateTimer = 0; }
            break;

        case LASER:
            animator.play("Laser");
            if (stateTimer > BossConfig::LASER_DAMAGE_FRAME) {
                float lx = facingRight ? x + 135.0f : x - 495.0f;
                currentAttackBox = { lx, y + 45.0f, 450.0f, 60.0f };
                game->triggerShake(4, 10);
            }
            else {
                currentAttackBox = { 0,0,0,0 };
            }
            if (animator.getIsFinished()) { currentState = WANDER; stateTimer = 0; }
            break;

        default: break;
        }

        if (x < 0) x = 0;
        if (x + width > SCREEN_WIDTH) x = SCREEN_WIDTH - width;

        animator.update();
    }

    void draw() override {
        int drawX = (int)x - 45; int drawY = (int)y - 60;

        if (game->getAssets().boss_r.getwidth() <= 0) {
            setfillcolor(RGB(200, 100, 150));
            solidrectangle((int)x, (int)y, (int)(x + width), (int)(y + height));
        }
        else animator.draw(drawX, drawY, facingRight);

        if (currentState == LASER && stateTimer > BossConfig::LASER_FRAME) {
            IMAGE* laserImg = facingRight ? &game->getAssets().laser_sheet_r : &game->getAssets().laser_sheet_l;
            if (laserImg->getwidth() > 0) {
                int totalFrames = 14;
                int frameH = 100;
                int currentSprFrame = ((stateTimer - BossConfig::LASER_FRAME) / 3);
                if (currentSprFrame >= totalFrames) currentSprFrame = totalFrames - 1;
                if (currentSprFrame < 0) currentSprFrame = 0;

                int lx = facingRight ? (int)x + 135 : (int)x - 495;
                int ly = (int)y + 15;

                HDC dstDC = GetImageHDC(NULL);
                HDC srcDC = GetImageHDC(laserImg);
                BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
                AlphaBlend(dstDC, lx, ly, 450, 150, srcDC, 0, currentSprFrame * frameH, 300, frameH, bf);
            }
            else {
                if (currentAttackBox.w > 0) {
                    setfillcolor(RGB(255, 50, 50));
                    solidrectangle((int)currentAttackBox.x, (int)currentAttackBox.y,
                        (int)(currentAttackBox.x + currentAttackBox.w), (int)(currentAttackBox.y + currentAttackBox.h));
                }
            }
        }
    }
};

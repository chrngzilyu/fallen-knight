#pragma once
#include "Game.h"
#include "Entity.h"

enum class PlayerAction { Idle, Run, Jump, Fall, LightAttack, HeavyAttack, PogoDrop, PogoRebound, Dash, Block, Hurt, Dead };

class Player : public Entity {
private:
    float jumpForce;
    int dashCooldown;
    int actionTimer, attackDir, chargeTimer, jumpCount;
    float stamina, maxStamina;

    int blockTimer;
    int parryEffectTimer;
    float parryEffectX, parryEffectY;

    PlayerAction currentAction;
    std::unordered_set<Entity*> hitEnemies;

    float recentJumps = 0.0f;
    float recentDashes = 0.0f;
    float recentParries = 0.0f;
    float recentAttacks = 0.0f;

    void handleInput();
    void updateState();
    void updatePhysics();
    void updateAnimation();

public:
    Rect currentAttackBox;

    float habitJump;
    float habitDash;
    float habitBlock;
    float habitHurt;

    Player(Game* g, float startX, float startY) : Entity(g, startX, startY, 60.0f, 90.0f, 6) {
        jumpForce = -14.6f; maxSpeed = 6.8f; dashCooldown = actionTimer = attackDir = chargeTimer = 0;
        jumpCount = 1; stamina = maxStamina = 100.0f;
        blockTimer = 0; parryEffectTimer = 0; parryEffectX = 0; parryEffectY = 0;

        habitJump = 0.0f; habitDash = 0.0f; habitBlock = 0.0f; habitHurt = 0.0f;

        currentAction = PlayerAction::Idle;
        currentAttackBox = { 0,0,0,0 };

        animator.init(&g->getAssets().knight_r, &g->getAssets().knight_l, 96, 96, 1.4f);
        animator.addAnimation("Idle", 0, 0, 7, 3, true);
        animator.addAnimation("Run", 0, 9, 15, 4, true);
        animator.addAnimation("Jump", 4, 0, 3, 15, false);
        animator.addAnimation("Fall", 4, 3, 5, 15, false);
        animator.addAnimation("Attack", 7, 6, 3, 4, false);
        animator.addAnimation("Pogo", 7, 1, 5, 5, false);
        animator.addAnimation("Dash", 3, 1, 6, 5, true);
        animator.addAnimation("Hurt", 8, 3, 7, 4, false);
        animator.addAnimation("Death", 9, 4, 9, 12, false);
    }

    bool shouldRemoveOnDeath() const override { return false; }
    float getStamina() const { return stamina; }
    float getMaxStamina() const { return maxStamina; }

    float getRecentJumps() const { return recentJumps; }
    float getRecentDashes() const { return recentDashes; }
    float getRecentParries() const { return recentParries; }
    float getRecentAttacks() const { return recentAttacks; }

    Rect getHitbox() const override { return { x + 15.0f, y + 22.0f, width - 30.0f, height - 22.0f }; }

    void checkOffensiveCollisions();
    void checkDefensiveCollisions();

    void updateAttackBox() {
        if (currentAction == PlayerAction::LightAttack) {
            if (attackDir == 0) currentAttackBox = { x - 90, y, 90, 90 };
            else if (attackDir == 1) currentAttackBox = { x + width, y, 90, 90 };
        }
        else if (currentAction == PlayerAction::HeavyAttack) {
            if (attackDir == 0) currentAttackBox = { x - 135, y - 30, 135, 120 };
            else if (attackDir == 1) currentAttackBox = { x + width, y - 30, 135, 120 };
        }
        else if (currentAction == PlayerAction::PogoDrop) {
            currentAttackBox = { x - 15, y + height, width + 30, 60 };
        }
        else {
            currentAttackBox = { 0, 0, 0, 0 };
        }
    }

    void takeDamage(int damage, float knockbackX = 0, int freezeFrames = 0) override {
        if (currentAction == PlayerAction::Dead || invincibilityTimer > 0) return;

        Entity::takeDamage(damage, knockbackX, freezeFrames);
        game->getSounds().playSfx(_T("hurt"));

        habitHurt += 1.0f;

        if (hp <= 0 && currentAction != PlayerAction::Dead) {
            hp = 0;
            currentAction = PlayerAction::Dead;
            hitFreezeTimer = 0;
            invincibilityTimer = 9999;
            animator.play("Death");
        }
    }

    void update() override {
        if (currentAction == PlayerAction::Dead) {
            updatePhysics();
            updateAnimation();
            return;
        }

        if (hitFreezeTimer > 0) {
            hitFreezeTimer--;
            if (invincibilityTimer > 0) invincibilityTimer--;
            currentAction = PlayerAction::Hurt;
            chargeTimer = 0;
            updateAnimation();
            return;
        }

        recentJumps *= 0.995f;
        recentDashes *= 0.995f;
        recentParries *= 0.995f;
        recentAttacks *= 0.995f;

        handleInput();
        updateState();
        updatePhysics();
        updateAttackBox();
        updateAnimation();
    }

    void draw() override {
        int drawX = (int)x - 37; int drawY = (int)y - 44;
        if (game->getAssets().knight_r.getwidth() <= 0) {
            if (currentAction == PlayerAction::Dead) setfillcolor(RGB(50, 50, 50));
            else setfillcolor(RGB(100, 150, 250));
            solidrectangle((int)x, (int)y, (int)(x + width), (int)(y + height));
        }
        else animator.draw(drawX, drawY, facingRight);

        if (chargeTimer > 12) {
            setlinecolor(RGB(255, 200, 50));
            setlinestyle(PS_SOLID, 3);
            circle((int)(x + width / 2), (int)(y + height / 2), 45 + (int)((chargeTimer - 12) * 1.5f));
        }

        if (parryEffectTimer > 0) {
            parryEffectTimer--;
            setlinecolor(RGB(255, 255, 200));
            setfillcolor(RGB(255, 255, 255));
            fillcircle((int)parryEffectX, (int)parryEffectY, 15 + (15 - parryEffectTimer));
            setlinecolor(RGB(150, 255, 255));
            circle((int)parryEffectX, (int)parryEffectY, 25 + (15 - parryEffectTimer) * 2);
        }
    }

    void pogoBounce() {
        vy = jumpForce * 1.1f; jumpCount = 1;
        stamina = (std::min)(maxStamina, stamina + 20.0f);
        currentAction = PlayerAction::PogoRebound;
        actionTimer = 27;
        currentAttackBox = { 0,0,0,0 };
    }
    PlayerAction getCurrentAction() const { return currentAction; }
};

#pragma once
#include "GameObject.h"
#include "Animator.h"

class Entity : public GameObject {
protected:
    float vx, vy;
    float gravity, maxSpeed;
    bool isGrounded, facingRight;
    int hp, maxHp;
    int invincibilityTimer, knockbackTimer, hitFreezeTimer;
    Animator animator;

public:
    Entity(Game* g, float x, float y, float w, float h, int health)
        : GameObject(g, x, y, w, h), vx(0), vy(0), gravity(0.56f), maxSpeed(6.8f),
        isGrounded(false), facingRight(true), hp(health), maxHp(health),
        invincibilityTimer(0), knockbackTimer(0), hitFreezeTimer(0) {
    }

    virtual void takeDamage(int damage, float knockbackX = 0, int freezeFrames = 0) {
        if (invincibilityTimer <= 0) {
            hp -= damage;
            invincibilityTimer = 61;
            vx = knockbackX;
            knockbackTimer = 17;
            hitFreezeTimer = freezeFrames;
            if (hp <= 0) { hp = 0; }
        }
    }
    int getHp() const { return hp; }
    int getMaxHp() const { return maxHp; }
    bool isDeathAnimFinished() const { return animator.getIsFinished(); }
};

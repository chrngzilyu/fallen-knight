#include "Player.h"
#include "Boss.h"
#include "Projectile.h"

void Player::handleInput() {
    bool isStateLocked = (currentAction == PlayerAction::Dash || currentAction == PlayerAction::LightAttack ||
        currentAction == PlayerAction::HeavyAttack || currentAction == PlayerAction::PogoDrop ||
        currentAction == PlayerAction::Hurt || currentAction == PlayerAction::Dead);

    if (!isStateLocked) {
        bool wantsToBlock = game->getInput().isKeyDown('L');
        bool canMaintainBlock = (currentAction == PlayerAction::Block && stamina > 0.0f);
        bool canStartBlock = (currentAction != PlayerAction::Block && stamina >= 15.0f);

        if (wantsToBlock && (canMaintainBlock || canStartBlock)) {
            if (currentAction != PlayerAction::Block) {
                blockTimer = 0;
                habitBlock += 1.0f;
            }
            currentAction = PlayerAction::Block;
            vx = 0;
            stamina -= 0.02f;
        }
        else {
            if (currentAction == PlayerAction::Block) {
                currentAction = PlayerAction::Idle;
            }
            bool isLeft = game->getInput().isKeyDown('A') || game->getInput().isKeyDown(VK_LEFT);
            bool isRight = game->getInput().isKeyDown('D') || game->getInput().isKeyDown(VK_RIGHT);
            bool isDown = game->getInput().isKeyDown('S') || game->getInput().isKeyDown(VK_DOWN);

            if (isLeft) { vx = -maxSpeed; facingRight = false; }
            else if (isRight) { vx = maxSpeed; facingRight = true; }
            else { vx = 0; }

            if (game->getInput().isKeyPressed(VK_SPACE)) {
                if (isGrounded) { vy = jumpForce; isGrounded = false; currentAction = PlayerAction::Jump; habitJump += 1.0f; }
                else if (jumpCount > 0) { vy = jumpForce; jumpCount--; currentAction = PlayerAction::Jump; habitJump += 1.0f; }
            }
            if (!game->getInput().isKeyDown(VK_SPACE) && vy < 0) vy *= 0.5f;

            if (game->getInput().isKeyPressed('K') && dashCooldown == 0 && stamina >= 20.0f) {
                stamina -= 20.0f;
                currentAction = PlayerAction::Dash;
                actionTimer = 21; dashCooldown = 71;
                vx = facingRight ? 18.6f : -18.6f;
                invincibilityTimer = 28;
                habitDash += 1.0f;
                game->getSounds().playSfx(_T("dash"));
            }

            if (game->getInput().isKeyPressed('J')) {
                if (!isGrounded && isDown) {
                    if (stamina >= 15.0f) {
                        stamina -= 15.0f; currentAction = PlayerAction::PogoDrop;
                        vy = 25.4f;
                          attackDir = 2; hitEnemies.clear();
                    }
                }
                else if (stamina >= 10.0f) {
                    stamina -= 10.0f; currentAction = PlayerAction::LightAttack;
                    actionTimer = 17; chargeTimer = 1; attackDir = facingRight ? 1 : 0; hitEnemies.clear();
                }
            }
        }
    }

    if (chargeTimer > 0) {
        if (game->getInput().isKeyDown('J')) {
            chargeTimer++;
        }
        else {
            if (chargeTimer >= 35 && stamina >= 30.0f) {
                if (!isStateLocked || currentAction == PlayerAction::LightAttack) {
                    stamina -= 30.0f;
                    currentAction = PlayerAction::HeavyAttack;
                    actionTimer = 35; attackDir = facingRight ? 1 : 0;
                    hitEnemies.clear();
                    game->triggerShake(6, 8);
                }
            }
            chargeTimer = 0;
        }
    }
}

void Player::updateState() {
    habitJump *= 0.99f;
    habitDash *= 0.99f;
    habitBlock *= 0.99f;
    habitHurt *= 0.98f;

    if (currentAction != PlayerAction::Dash && currentAction != PlayerAction::LightAttack &&
        currentAction != PlayerAction::HeavyAttack && currentAction != PlayerAction::PogoDrop &&
        currentAction != PlayerAction::Block && currentAction != PlayerAction::Dead) {
        stamina = (std::min)(maxStamina, stamina + 0.11f);
    }

    if (invincibilityTimer > 0) invincibilityTimer--;
    if (dashCooldown > 0) dashCooldown--;
    if (knockbackTimer > 0) { knockbackTimer--; vx *= 0.89f; }

    switch (currentAction) {
    case PlayerAction::Hurt:
        if (knockbackTimer <= 0) currentAction = isGrounded ? PlayerAction::Idle : PlayerAction::Fall;
        break;
    case PlayerAction::Block:
        blockTimer++;
        if (!game->getInput().isKeyDown('L') || stamina <= 0) currentAction = PlayerAction::Idle;
        break;
    case PlayerAction::PogoRebound:
        actionTimer--;
        if (actionTimer <= 0) currentAction = PlayerAction::Fall;
        if (isGrounded) currentAction = PlayerAction::Idle;
        break;
    case PlayerAction::Dash:
        actionTimer--;
        if (actionTimer <= 0) { vx *= 0.4f; currentAction = isGrounded ? PlayerAction::Idle : PlayerAction::Fall; }
        break;
    case PlayerAction::LightAttack:
    case PlayerAction::HeavyAttack:
        actionTimer--;
        if (actionTimer <= 0) currentAction = isGrounded ? PlayerAction::Idle : PlayerAction::Fall;
        break;
    default: break;
    }
}

void Player::updatePhysics() {
    switch (currentAction) {
    case PlayerAction::PogoDrop: vy = 25.4f; break; 
    case PlayerAction::Dash: vy = 0; break;
    case PlayerAction::Dead:
        if (vy > 0) vy += gravity * 1.5f; else vy += gravity;
        if (vy > 15.2f) vy = 15.2f;
        vx = 0; chargeTimer = 0;
        break;
    default:
        if (currentAction == PlayerAction::LightAttack || currentAction == PlayerAction::HeavyAttack) {
            if (!isGrounded) vy += gravity * 0.5f;
        }
        else {
            if (vy > 0) vy += gravity * 1.5f; else vy += gravity;
        }
        if (vy > 15.2f) vy = 15.2f;

        if (isGrounded) {
            if (currentAction != PlayerAction::Hurt && currentAction != PlayerAction::Block &&
                currentAction != PlayerAction::LightAttack && currentAction != PlayerAction::HeavyAttack &&
                currentAction != PlayerAction::PogoRebound && currentAction != PlayerAction::Dead) {
                currentAction = (fabs(vx) > 0) ? PlayerAction::Run : PlayerAction::Idle;
            }
        }
        else {
            if (currentAction == PlayerAction::Idle || currentAction == PlayerAction::Run) {
                currentAction = (vy < -2.0f) ? PlayerAction::Jump : PlayerAction::Fall;
            }
        }
        break;
    }

    x += vx; y += vy;

    if (y + height >= GROUND_Y) {
        y = GROUND_Y - height; vy = 0; isGrounded = true; jumpCount = 1;
        if (currentAction == PlayerAction::PogoDrop) {
            currentAction = PlayerAction::Idle;
            game->triggerShake(6, 8);
        }
        if (currentAction != PlayerAction::Dash && currentAction != PlayerAction::Dead) {
            dashCooldown -= 2; if (dashCooldown < 0) dashCooldown = 0;
        }
    }
    else { isGrounded = false; }

    if (x < 0) { x = 0; vx = 0; }
    if (x + width > SCREEN_WIDTH) { x = SCREEN_WIDTH - width; vx = 0; }
}

void Player::updateAnimation() {
    switch (currentAction) {
    case PlayerAction::LightAttack: animator.play("Attack"); break;
    case PlayerAction::HeavyAttack: animator.play("Pogo"); break;
    case PlayerAction::PogoDrop:    animator.play("Pogo"); break;
    case PlayerAction::PogoRebound: animator.play("Pogo"); break;
    case PlayerAction::Dash:        animator.play("Dash"); break;
    case PlayerAction::Block:       animator.play("Dash"); break;
    case PlayerAction::Jump:        animator.play("Jump"); break;
    case PlayerAction::Fall:        animator.play("Fall"); break;
    case PlayerAction::Run:         animator.play("Run"); break;
    case PlayerAction::Idle:        animator.play("Idle"); break;
    case PlayerAction::Hurt:        animator.play("Hurt"); break;
    case PlayerAction::Dead:        animator.play("Death"); break;
    }
    animator.update();
}

void Player::checkOffensiveCollisions() {
    if (currentAction != PlayerAction::LightAttack && currentAction != PlayerAction::HeavyAttack && currentAction != PlayerAction::PogoDrop) return;

    for (auto& obj : game->getObjects()) {
        Entity* enemy = dynamic_cast<Entity*>(obj.get());
        if (enemy && enemy != this && !enemy->getIsDead()) {
            if (checkAABB(currentAttackBox, enemy->getHitbox())) {
                if (hitEnemies.find(enemy) == hitEnemies.end()) {
                    int dmg = (currentAction == PlayerAction::HeavyAttack) ? 25 : 10;
                    enemy->takeDamage(dmg, getHitbox().x < enemy->getHitbox().x ? 3.0f : -3.0f, 16);
                    hitEnemies.insert(enemy);

                    recentAttacks += 1.0f;

                    game->triggerShake(4, 6);
                    stamina = (std::min)(maxStamina, stamina + 15.0f);

                    if (currentAction == PlayerAction::LightAttack) {
                        game->getSounds().playSfx(_T("light_attack"));
                    }
                    else {
                        game->getSounds().playSfx(_T("heavy_attack"));
                    }

                    if (currentAction == PlayerAction::PogoDrop) {
                        pogoBounce();
                        game->triggerShake(8, 10);
                        return;
                    }
                }
            }
        }
    }
}

void Player::checkDefensiveCollisions() {
    if (currentAction == PlayerAction::Dash) return;

    for (auto& obj : game->getObjects()) {
        if (obj.get() != this && !obj->getIsDead()) {

            Boss* b = dynamic_cast<Boss*>(obj.get());
            if (b && b->currentAttackBox.w > 0 && checkAABB(getHitbox(), b->currentAttackBox)) {

                bool isFacingAttack = (b->getHitbox().x + b->getHitbox().w / 2 > getHitbox().x + getHitbox().w / 2) ? facingRight : !facingRight;

                if (currentAction == PlayerAction::Block && isFacingAttack) {
                    if (blockTimer <= 12) {
                        stamina = (std::min)(maxStamina, stamina + 30.0f);
                        invincibilityTimer = 40;
                        game->triggerShake(6, 6);

                        b->takeDamage(0, facingRight ? 4.0f : -4.0f, 60);

                        parryEffectTimer = 15;
                        parryEffectX = facingRight ? x + width : x;
                        parryEffectY = y + height / 2;

                        
                        game->getSounds().playSfx(_T("block"));
                        recentParries += 1.0f;
                        continue;
                    }
                    else if (stamina > 0.0f) {
                        stamina = 0.0f;
                        vx = facingRight ? -6.0f : 6.0f;
                        knockbackTimer = 10;
                        invincibilityTimer = 40;
                        game->triggerShake(3, 5);
                        continue;
                    }
                }

                takeDamage(1, b->getHitbox().x > getHitbox().x ? -7.5f : 7.5f, 16);
                game->triggerShake(10, 12);
            }

            Projectile* p = dynamic_cast<Projectile*>(obj.get());
            if (p && checkAABB(getHitbox(), p->getHitbox())) {
                bool isFacingAttack = (p->getHitbox().x + p->getHitbox().w / 2 > getHitbox().x + getHitbox().w / 2) ? facingRight : !facingRight;

                if (currentAction == PlayerAction::Block && isFacingAttack) {
                    if (blockTimer <= 12) {
                        stamina = (std::min)(maxStamina, stamina + 15.0f);
                        invincibilityTimer = 20;
                        game->triggerShake(4, 6);

                        parryEffectTimer = 15;
                        parryEffectX = facingRight ? x + width : x;
                        parryEffectY = y + height / 2;

                        
                        game->getSounds().playSfx(_T("block"));
                        recentParries += 1.0f;
                        p->kill();
                        continue;
                    }
                    else if (stamina > 0.0f) {
                        stamina = 0.0f;
                        invincibilityTimer = 20;
                        p->kill();
                        continue;
                    }
                }

                takeDamage(1, 0, 11); p->kill();
            }
        }
    }
}

#pragma once
#include "Common.h"

struct AnimData {
    int startRow, startCol, frameCount, speed; bool loop;
};

class Animator {
private:
    IMAGE* sheetR; IMAGE* sheetL;
    int frameW, frameH; float scale;
    std::map<std::string, AnimData> animations;
    std::string currentAnim;
    int currentFrame, frameTimer;
    bool isFinished;

public:
    Animator() : sheetR(nullptr), sheetL(nullptr), frameW(0), frameH(0), scale(1.0f), currentFrame(0), frameTimer(0), isFinished(false) {}

    void init(IMAGE* r, IMAGE* l, int fw, int fh, float s = 1.0f) {
        sheetR = r; sheetL = l; frameW = fw; frameH = fh; scale = s;
    }

    void addAnimation(const std::string& name, int startRow, int startCol, int frameCount, int speed, bool loop) {
        animations[name] = { startRow, startCol, frameCount, speed, loop };
    }

    void play(const std::string& name) {
        if (currentAnim == name) return;
        if (animations.find(name) == animations.end()) return;
        currentAnim = name; currentFrame = 0; frameTimer = 0; isFinished = false;
    }

    void update() {
        if (currentAnim.empty() || isFinished) return;
        AnimData& anim = animations[currentAnim];
        frameTimer++;
        if (frameTimer >= anim.speed) {
            frameTimer = 0; currentFrame++;
            if (currentFrame >= anim.frameCount) {
                if (anim.loop) currentFrame = 0;
                else { currentFrame = anim.frameCount - 1; isFinished = true; }
            }
        }
    }

    void draw(int x, int y, bool facingRight) {
        if (currentAnim.empty() || !sheetR || sheetR->getwidth() <= 0) return;
        IMAGE* currentSheet = facingRight ? sheetR : sheetL;
        if (!currentSheet || currentSheet->getwidth() <= 0) currentSheet = sheetR;

        AnimData& anim = animations[currentAnim];

        int totalCols = (std::max<int>)(1, currentSheet->getwidth() / frameW);
        int flatIndex = anim.startRow * totalCols + anim.startCol + currentFrame;
        int currentRow = flatIndex / totalCols;
        int currentCol = flatIndex % totalCols;

        if (!facingRight && sheetL && sheetL->getwidth() > 0) {
            currentCol = (totalCols - 1) - (flatIndex % totalCols);
        }

        int srcX = currentCol * frameW;
        int srcY = currentRow * frameH;

        HDC dstDC = GetImageHDC(NULL); HDC srcDC = GetImageHDC(currentSheet);
        BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

        AlphaBlend(dstDC, x, y, (int)(frameW * scale), (int)(frameH * scale), srcDC, srcX, srcY, frameW, frameH, bf);
    }

    bool getIsFinished() const { return isFinished; }
};

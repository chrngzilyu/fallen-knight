#include "Common.h"

TCHAR g_audioErrorMsg[256] = { 0 };

bool checkAABB(Rect a, Rect b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

void putimageAlpha(int x, int y, IMAGE* img) {
    if (img->getwidth() <= 0) return;
    HDC dstDC = GetImageHDC(NULL), srcDC = GetImageHDC(img);
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    AlphaBlend(dstDC, x, y, img->getwidth(), img->getheight(), srcDC, 0, 0, img->getwidth(), img->getheight(), bf);
}

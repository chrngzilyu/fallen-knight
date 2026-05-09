#include "UI.h"
#include "Game.h"

void DrawStylizedText(int x, int y, const TCHAR* text, COLORREF mainColor, int size, const TCHAR* fontName, int weight, bool hasGlow) {
    setbkmode(TRANSPARENT);
    settextstyle(size, 0, fontName, 0, 0, weight, false, false, false);

    if (hasGlow) {
        settextcolor(RGB(255, 255, 255));
        outtextxy(x + 1, y + 1, text);
    }

    settextcolor(mainColor);
    outtextxy(x, y, text);
}

void DrawCenteredText(int y, const TCHAR* text, COLORREF color, int size, const TCHAR* fontName, int weight, bool hasGlow) {
    settextstyle(size, 0, fontName, 0, 0, weight, false, false, false);
    int tw = textwidth(text);
    DrawStylizedText((SCREEN_WIDTH - tw) / 2, y, text, color, size, fontName, weight, hasGlow);
}

bool DrawPremiumButton(Game* game, int x, int y, int w, int h, const TCHAR* text) {
    const ExMessage& msg = game->getLastMouseMsg();
    bool isHover = (msg.x >= x && msg.x <= x + w && msg.y >= y && msg.y <= y + h);

    settextstyle(36, 0, _T("微软雅黑"), 0, 0, 400, false, false, false);

    int tx = x + (w - textwidth(text)) / 2;
    int ty = y + (h - textheight(text)) / 2;

    if (isHover) {
        DrawStylizedText(tx, ty, text, RGB(0, 0, 0), 36, _T("微软雅黑"), 800, true);

        int textW = textwidth(text);
        setlinecolor(RGB(0, 0, 0));
        setlinestyle(PS_SOLID, 2);
        line(tx - 25, ty + 45, tx + textW + 25, ty + 45);
        setfillcolor(RGB(0, 0, 0));
        solidcircle(tx - 35, ty + 20, 4);
        solidcircle(tx + textW + 35, ty + 20, 4);
    }
    else {
        DrawStylizedText(tx, ty, text, RGB(80, 80, 80), 36, _T("微软雅黑"), 400, false);
    }

    return (isHover && game->isMouseClicked());
}

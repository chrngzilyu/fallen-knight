#pragma once
#include "Common.h"

class Game;

void DrawStylizedText(int x, int y, const TCHAR* text, COLORREF mainColor, int size, const TCHAR* fontName, int weight = 400, bool hasGlow = true);
void DrawCenteredText(int y, const TCHAR* text, COLORREF color, int size, const TCHAR* fontName, int weight = 400, bool hasGlow = true);
bool DrawPremiumButton(Game* game, int x, int y, int w, int h, const TCHAR* text);

#pragma once
#include <windows.h>

void DrawCircleDirect(HDC hdc, int xc, int yc, int R, COLORREF color);
void DrawCirclePolar(HDC hdc, int xc, int yc, int R, COLORREF color);
void DrawCircleIterativePolar(HDC hdc, int xc, int yc, int R, COLORREF color);
void DrawCircleMidpoint(HDC hdc, int xc, int yc, int R, COLORREF color);
void DrawCircleModifiedMidpoint(HDC hdc, int xc, int yc, int R, COLORREF color);
#pragma once
#include <windows.h>

void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
void DrawLineParametric(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
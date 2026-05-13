#pragma once
#include <windows.h>
#include <cmath>


void Draw4Points(HDC hdc, int xc, int yc, int x, int y, COLORREF c);

// Direct Ellipse
void DrawDirectEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF c);

// Polar Ellipse
void DrawPolarEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF c);


// Midpoint Ellipse
void DrawMidpointEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF c);
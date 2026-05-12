#include "LineAlgorithms.h"
#include <cmath>

// Helper functions (local, non serve anche la parola static per evitar conflitti)
static int Round(double x) { return (int)(x + 0.5); }
static void DrawPixel(HDC hdc, int x, int y, COLORREF color) { SetPixel(hdc, x, y, color); }

void DrawLineDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    int dx = x2 - x1, dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));
    float xInc = dx / (float)steps, yInc = dy / (float)steps;
    float x = (float)x1, y = (float)y1;
    for (int i = 0; i <= steps; i++) {
        DrawPixel(hdc, Round(x), Round(y), color);
        x += xInc; y += yInc;
    }
}

void DrawLineMidpoint(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1, sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    while (true) {
        DrawPixel(hdc, x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }
    }
}

void DrawLineParametric(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    int dx = x2 - x1, dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));
    for (int i = 0; i <= steps; i++) {
        float t = i / (float)steps;
        int x = Round(x1 + t * dx), y = Round(y1 + t * dy);
        DrawPixel(hdc, x, y, color);
    }
}
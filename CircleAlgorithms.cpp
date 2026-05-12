#include "CircleAlgorithms.h"
#include <cmath>

const double PI = 3.14159265358979323846;

static int Round(double x) { return (int)(x + 0.5); }
static void DrawPixel(HDC hdc, int x, int y, COLORREF color) { SetPixel(hdc, x, y, color); }

static void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF color) {
    DrawPixel(hdc, xc + x, yc + y, color);
    DrawPixel(hdc, xc - x, yc + y, color);
    DrawPixel(hdc, xc + x, yc - y, color);
    DrawPixel(hdc, xc - x, yc - y, color);
    DrawPixel(hdc, xc + y, yc + x, color);
    DrawPixel(hdc, xc - y, yc + x, color);
    DrawPixel(hdc, xc + y, yc - x, color);
    DrawPixel(hdc, xc - y, yc - x, color);
}

void DrawCircleDirect(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int x = 0, y = R;
    while (x <= y) {
        Draw8Points(hdc, xc, yc, x, y, color);
        x++;
        y = Round(sqrt((double)(R*R - x*x)));
    }
}

void DrawCirclePolar(HDC hdc, int xc, int yc, int R, COLORREF color) {
    double dtheta = 1.0 / R;
    for (double theta = 0; theta <= PI/4.0; theta += dtheta) {
        int x = Round(R * cos(theta)), y = Round(R * sin(theta));
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}

void DrawCircleIterativePolar(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int steps = (int)(R * PI / 4.0) + 1;
    for (int i = 0; i <= steps; i++) {
        double theta = i * (PI/4.0) / steps;
        int x = Round(R * cos(theta)), y = Round(R * sin(theta));
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}

void DrawCircleMidpoint(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int x = 0, y = R, p = 1 - R;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (x < y) {
        x++;
        if (p < 0) p += 2*x + 1;
        else { y--; p += 2*(x - y) + 1; }
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}

void DrawCircleModifiedMidpoint(HDC hdc, int xc, int yc, int R, COLORREF color) {
    int x = 0, y = R, d = 1 - R;
    int deltaE = 3, deltaSE = -2*R + 5;
    Draw8Points(hdc, xc, yc, x, y, color);
    while (y > x) {
        if (d < 0) { d += deltaE; deltaE += 2; deltaSE += 2; x++; }
        else { d += deltaSE; deltaE += 2; deltaSE += 4; x++; y--; }
        Draw8Points(hdc, xc, yc, x, y, color);
    }
}
#pragma once
#include <windows.h>
#include "shape.h"
#include "EllipseAlgorithms.h"
#include "Curves.h"
#include <vector>
#include <cmath>

using namespace std;

// Midpoint Circle
void DrawMidpointCircle(HDC hdc, int xc, int yc, int r, COLORREF c);


// Filled Circle
void FillCircle(HDC hdc, int xc, int yc, int r, COLORREF c);

// Face Parts
void DrawEye(HDC hdc, int ex, int ey);

void DrawEyebrow(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c);

void DrawMouth(HDC hdc, int cx, int cy, int hw, int depth, bool happy, COLORREF c);


// Happy Face
void DrawHappyFace(HDC hdc, int cx, int cy, COLORREF c);


// Sad Face
void DrawSadFace(HDC hdc, int cx, int cy, COLORREF c);
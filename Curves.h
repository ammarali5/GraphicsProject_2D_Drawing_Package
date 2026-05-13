#pragma once
#include <windows.h>
#include <cmath>
#include <cstdio>
#include <vector>

#include "shape.h"
using namespace std;

void hermiteBasis(double t, double& h1, double& h2, double& h3, double& h4);

void DrawCardinalSpline(HDC hdc, const vector<Point>& pts, COLORREF c);
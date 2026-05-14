#pragma once
#include <vector>
#include <windows.h>
#include <algorithm>
#include <cmath>
#include "shape.h"
using namespace std;

struct ClipRect {
    int xMin, yMin, xMax, yMax;
};

// Rectangle Clipping Point
bool ClipPoint_Rectangle(int x, int y, const ClipRect& rect) {
    return x >= rect.xMin && x <= rect.xMax && y >= rect.yMin && y <= rect.yMax;
}

// Cohen-Sutherland region codes
const int INSIDE = 0; // 0000
const int LEFT = 1;   // 0001
const int RIGHT = 2;  // 0010
const int BOTTOM = 4; // 0100
const int TOP = 8;    // 1000

int ComputeOutCode(int x, int y, const ClipRect& rect) {
    int code = INSIDE;
    if (x < rect.xMin) code |= LEFT;
    else if (x > rect.xMax) code |= RIGHT;
    if (y < rect.yMin) code |= TOP;
    else if (y > rect.yMax) code |= BOTTOM;
    return code;
}

bool ClipLine_CohenSutherland(int& x1, int& y1, int& x2, int& y2, const ClipRect& rect) {
    int outcode1 = ComputeOutCode(x1, y1, rect);
    int outcode2 = ComputeOutCode(x2, y2, rect);
    bool accept = false;

    while (true) {
        if (!(outcode1 | outcode2)) { // Both inside
            accept = true;
            break;
        } else if (outcode1 & outcode2) { // Logical AND != 0, outside
            break;
        } else {
            int x, y;
            int outcodeOut = outcode1 ? outcode1 : outcode2;

            if (outcodeOut & TOP) {
                x = x1 + (rect.yMin - y1) * (x2 - x1) / (y2 - y1);
                y = rect.yMin;
            } else if (outcodeOut & BOTTOM) {
                x = x1 + (rect.yMax - y1) * (x2 - x1) / (y2 - y1);
                y = rect.yMax;
            } else if (outcodeOut & RIGHT) {
                y = y1 + (rect.xMax - x1) * (y2 - y1) / (x2 - x1);
                x = rect.xMax;
            } else {
                y = y1 + (rect.xMin - x1) * (y2 - y1) / (x2 - x1);
                x = rect.xMin;
            }

            if (outcodeOut == outcode1) {
                x1 = x;
                y1 = y;
                outcode1 = ComputeOutCode(x1, y1, rect);
            } else {
                x2 = x;
                y2 = y;
                outcode2 = ComputeOutCode(x2, y2, rect);
            }
        }
    }
    return accept;
}

// Sutherland-Hodgman polygon clipping against rectangle edges
vector<Point> ClipPolygon_SutherlandHodgman(const vector<Point>& poly, const ClipRect& rect) {
    vector<Point> input = poly;

    auto clipEdge = [&](int edge) {
        vector<Point> output;
        auto inside = [&](Point p) {
            switch(edge) {
                case 0: return p.x >= rect.xMin; // Left
                case 1: return p.x <= rect.xMax; // Right
                case 2: return p.y >= rect.yMin; // Top
                case 3: return p.y <= rect.yMax; // Bottom
            }
            return true;
        };
        auto intersect = [&](Point p1, Point p2) {
            Point inter;
            if (edge == 0) { // Left edge
                inter.x = rect.xMin;
                inter.y = p1.y + (p2.y - p1.y) * (rect.xMin - p1.x) / (p2.x - p1.x);
            } else if (edge == 1) { // Right edge
                inter.x = rect.xMax;
                inter.y = p1.y + (p2.y - p1.y) * (rect.xMax - p1.x) / (p2.x - p1.x);
            } else if (edge == 2) { // Top edge
                inter.y = rect.yMin;
                inter.x = p1.x + (p2.x - p1.x) * (rect.yMin - p1.y) / (p2.y - p1.y);
            } else { // Bottom edge
                inter.y = rect.yMax;
                inter.x = p1.x + (p2.x - p1.x) * (rect.yMax - p1.y) / (p2.y - p1.y);
            }
            return inter;
        };

        for (int i = 0; i < (int)input.size(); i++) {
            Point cur = input[i];
            Point prev = input[(i + (int)input.size() - 1) % input.size()];
            bool curInside = inside(cur);
            bool prevInside = inside(prev);

            if (curInside) {
                if (!prevInside) {
                    output.push_back(intersect(prev, cur));
                }
                output.push_back(cur);
            } else if (prevInside) {
                output.push_back(intersect(prev, cur));
            }
        }
        input = output;
    };

    // Clip polygon against each edge
    for (int edge = 0; edge < 4; edge++)
        clipEdge(edge);

    return input;
}

// Circle Clipping Point: True if point inside circle
bool ClipPoint_Circle(int x, int y, int cx, int cy, int r) {
    int dx = x - cx, dy = y - cy;
    return dx*dx + dy*dy <= r*r;
}

// Clip line to circle by generating points inside circle
vector<Point> ClipLine_Circle(int x1, int y1, int x2, int y2, int cx, int cy, int r) {
    vector<Point> clippedPoints;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));
    if (steps == 0) return clippedPoints;

    for (int i = 0; i <= steps; i++) {
        int px = x1 + dx * i / steps;
        int py = y1 + dy * i / steps;
        if (ClipPoint_Circle(px, py, cx, cy, r)) {
            clippedPoints.push_back({px, py});
        }
    }
    return clippedPoints;
}

// Draw rectangle clipping window outline
void DrawClipRect(HDC hdc, const ClipRect& r, COLORREF c) {
    for (int x = r.xMin; x <= r.xMax; x++) {
        SetPixel(hdc, x, r.yMin, c);
        SetPixel(hdc, x, r.yMax, c);
    }
    for (int y = r.yMin; y <= r.yMax; y++) {
        SetPixel(hdc, r.xMin, y, c);
        SetPixel(hdc, r.xMax, y, c);
    }
}

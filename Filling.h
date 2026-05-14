#include <Windows.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <stack>
#include <algorithm>

#include "shape.h"
using namespace std;

// for non-convex
struct Node {
    double x1;
    int y2;
    double mi;

    Node(double x = 0, int y = 0, double m = 0): x1(x), y2(y), mi(m){}
};

// for convex
struct Rec {
    double xLeft, xRight;
    Rec(double x = 0, double y = 0) : xLeft(x), xRight(y) {}
};

typedef Rec Table[800];
typedef deque<Node> NonConvTable[800];

// Debugging Functions

// used when a user clicks somewhere on the screen to visually represent where they clicked.
void DrawBoldPoint(HDC hdc, int x, int y, COLORREF c) {
    for (int i = 0; i < 4; i++) {
        SetPixel(hdc, x, y, c);
        SetPixel(hdc, x + i, y, c);
        SetPixel(hdc, x, y + i, c);
        SetPixel(hdc, x + i, y + i, c);
        SetPixel(hdc, x - i, y, c);
        SetPixel(hdc, x, y - i, c);
        SetPixel(hdc, x - i, y - i, c);
        SetPixel(hdc, x - i, y + i, c);
        SetPixel(hdc, x + i, y - i, c);
    }
}

// FloodFill Algorithms 
void NonRecursiveFloodFill(HDC hdc, int x, int y, COLORREF fc) {
    COLORREF bc = GetPixel(hdc, x, y);
    stack<Point> stk;
    stk.push({x, y});
    
    while (!stk.empty()) {

        Point p = stk.top();
        stk.pop();

        COLORREF c = GetPixel(hdc, p.x, p.y);

        if (c != bc) {
            continue;
        }

        SetPixel(hdc, p.x, p.y, fc);
        stk.push({p.x, p.y + 1});
        stk.push({p.x - 1, p.y});
        stk.push({p.x, p.y - 1});
        stk.push({p.x + 1, p.y});

    }
}

void RecursiveFloodFill(HDC hdc, int x, int y, COLORREF bc, COLORREF fc) {
    if (x < 0 || y < 0 || x >= 1000 || y >= 700) return;

    if (GetPixel(hdc, x, y) != bc) return;

    SetPixel(hdc, x, y, fc);
    RecursiveFloodFill(hdc, x + 1, y, bc, fc);
    RecursiveFloodFill(hdc, x - 1, y, bc, fc);
    RecursiveFloodFill(hdc, x, y + 1, bc, fc);
    RecursiveFloodFill(hdc, x, y - 1, bc, fc);
}

void RecursiveFloodFillCall(HDC hdc, int x, int y, COLORREF fc) {
    
    COLORREF bc = GetPixel(hdc, x, y);
    RecursiveFloodFill(hdc, x, y, bc, fc);
}

// Convex Fill Algorithm
void Init(Table t) {
    for (int i = 0; i < 800; i++) {
        t[i].xLeft = INT_MAX;
        t[i].xRight = INT_MIN;
    }
}

void EdgeToTable(Point p1, Point p2, Table t) {
    if (p1.y == p2.y) return;
    if (p1.y > p2.y) swap(p1, p2);
    int y = p1.y;
    double x = p1.x;
    double mi = (double)(p2.x - p1.x) / (p2.y - p1.y);
    while (y < p2.y) {
        if (x < t[y].xLeft) t[y].xLeft = x;
        if (x > t[y].xRight) t[y].xRight = x;
        y++;
        x += mi;
    }
}

void PolygonToTable(vector<Point> p, Table t) {
    Point v1 = p[p.size() - 1];
    for (int i = 0; i < p.size(); i++) {
        Point v2 = p[i];
        EdgeToTable(v1, v2, t);
        v1 = v2;
    }
}

void DrawLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c) {

    for (int x = x1; x <= x2; x++) {
        SetPixel(hdc, x, y1, c);
    }
}

void TableToScreen(HDC hdc, Table t, COLORREF c) {
    for (int i = 0; i < 800; i++) {
        if (t[i].xLeft < t[i].xRight) {
            DrawLine(hdc, t[i].xLeft, i, t[i].xRight, i, c);
        }
    }
}

void ConvexFill(HDC hdc, vector<Point> p, COLORREF c) {
    Table t;
    Init(t);
    PolygonToTable(p, t);
    TableToScreen(hdc, t, c);
}

// Non-Convex Fill Algorithm

void EdgeToList(Point p1, Point p2, NonConvTable& t) {
    if (p1.y == p2.y) return;

    if (p1.y > p2.y) swap(p1, p2);

    double mi = (double)(p2.x - p1.x) / (p2.y - p1.y);
    t[p1.y + 1].push_back(Node(p1.x, p2.y, mi));
}

void NonConvexFill(HDC hdc, vector<Point>p, COLORREF c) {
    
    NonConvTable t;

    // Polygon to list
    Point v1 = p[p.size() - 1];
    for (int i = 0; i < p.size(); i++) {
        Point v2 = p[i];
        EdgeToList(v1, v2, t);
        v1 = v2;
    }

    deque<Node> active;
    int y = 0;
    while (y < 800 && t[y].empty()) {
        y++;
    }

    if (y >= 800) return;

    active = t[y];
    while (!active.empty()) {

        std::sort(active.begin(), active.end(),
            [](const Node& a, const Node& b) {
                return a.x1 < b.x1;
            });

        for (auto it = active.begin(); it + 1 < active.end(); it += 2) {
            DrawLine(hdc,
                it->x1, y,
                (it + 1)->x1, y,
                c);
        }

        // remove ended edges
        for (auto it = active.begin(); it != active.end(); ) {
            if (it->y2 == y)
                it = active.erase(it);
            else
                ++it;
        }
        y++;

        // update x intersections
        for (auto& node : active) {
            node.x1 += node.mi;
        }

        // add new edges
        if (y < 800) {
            while (!t[y].empty()) {
                active.push_back(t[y].front());
                t[y].pop_front();
            }
        }
    }
}

// Hermite Curve Drawing
void DrawHermiteCurve(HDC hdc, double x1, double y1, double x2, double y2,
    double u1, double v1, double u2, double v2, COLORREF c)
{
    vector<vector<double>> Hermite = {
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {-3, -2, 3, -1},
        {2, 1, -2, 1}
    };
    vector<vector<double>> Given = {
        {x1, y1},
        {u1, v1},
        {x2, y2},
        {u2, v2}
    };
    vector<vector<double>> Constants(4, vector<double>(2, 0));
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            Constants[i][0] += Hermite[i][j] * Given[j][0];
            Constants[i][1] += Hermite[i][j] * Given[j][1];
        }
    }

    for (double t = 0; t <= 1; t += 0.0005) {
        double x = Constants[0][0] + Constants[1][0] * t + Constants[2][0] * t * t + Constants[3][0] * t * t * t;
        double y = Constants[0][1] + Constants[1][1] * t + Constants[2][1] * t * t + Constants[3][1] * t * t * t;
        SetPixel(hdc, round(x), round(y), c);
    }
}

// Square Hermite Filling

void SquareHermiteFilling(HDC hdc, vector<Point> p, COLORREF c, int step = 5) {

    double xLeft = p[0].x, xRight = p[0].x;
    double yTop = p[0].y, yBottom = p[0].y;

    for (auto& pt : p)
    {
        xLeft = min(xLeft, (double)pt.x);
        xRight = max(xRight, (double)pt.x);
        yTop = min(yTop, (double)pt.y);
        yBottom = max(yBottom, (double)pt.y);
    }

    for (double x = xLeft; x <= xRight; x += step)
    {
        double u1 = 200, v1 = 30;
        double u2 = 270, v2 = -30;

        DrawHermiteCurve(
            hdc,
            x, yBottom,   // start point
            x, yTop,      // end point
            u1, v1,       // start tangent
            u2, v2,       // end tangent
            c
        );
    }
}

// Rectangle Bezier Filling

// Combination Calculator
long long C(int n, int k) {
    if (k > n - k) k = n - k;
    long long res = 1;
    for (int i = 0; i < k; i++) {
        res *= (n - i);
        res /= (i + 1);
    }
    return res;
}

void DrawBezierCurve(HDC hdc, vector<pair<double, double>> v, COLORREF c)
{

    for (double t = 0; t <= 1; t += 0.0005) {
        double x = 0, y = 0;
        for (int r = 0; r < 4; r++) {
            x += C(3, r) * pow(t, r) * pow(1 - t, 3 - r) * v[r].first;
            y += C(3, r) * pow(t, r) * pow(1 - t, 3 - r) * v[r].second;
        }

        SetPixel(hdc, round(x), round(y), c);
    }
}

void FillRectangleBezier(HDC hdc, vector<Point> p, COLORREF c, int step = 5)
{
    double xLeft = p[0].x, xRight = p[0].x;
    double yTop = p[0].y, yBottom = p[0].y;

    for (auto& pt : p)
    {
        xLeft = min(xLeft, (double)pt.x);
        xRight = max(xRight, (double)pt.x);
        yTop = min(yTop, (double)pt.y);
        yBottom = max(yBottom, (double)pt.y);
    }

    for (double y = yTop; y <= yBottom; y += step)
    {
        vector<pair<double, double>> v(4);

        v[0] = { xLeft, y };
        v[3] = { xRight, y };

        v[1] = { xLeft + (xRight - xLeft) * 0.25, y - 100 };
        v[2] = { xLeft + (xRight - xLeft) * 0.75, y + 100 };

        DrawBezierCurve(hdc, v, c);
    }
}

// Circle Task
void Draw8Points(HDC hdc, int xc, int yc, int x, int y, COLORREF c, int quarter = 0)
{
    if (!quarter) {
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc + y, yc - x, c);

        SetPixel(hdc, xc - x, yc - y, c);
        SetPixel(hdc, xc - y, yc - x, c);

        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc - y, yc + x, c);

        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc + y, yc + x, c);

    }
    else if (quarter == 4) {
        SetPixel(hdc, xc + x, yc + y, c);
        SetPixel(hdc, xc + y, yc + x, c);
    }
    else if (quarter == 3) {
        SetPixel(hdc, xc - x, yc + y, c);
        SetPixel(hdc, xc - y, yc + x, c);
    }
    else if (quarter == 2) {
        SetPixel(hdc, xc - x, yc - y, c);
        SetPixel(hdc, xc - y, yc - x, c);
    }
    else if (quarter == 1) {
        SetPixel(hdc, xc + x, yc - y, c);
        SetPixel(hdc, xc + y, yc - x, c);
    }
    

   /* SetPixel(hdc, xc - x, yc + y, c);
    SetPixel(hdc, xc - x, yc - y, c);
    SetPixel(hdc, xc + x, yc - y, c);
    SetPixel(hdc, xc - y, yc + x, c);
    SetPixel(hdc, xc + y, yc - x, c);
    SetPixel(hdc, xc - y, yc - x, c);*/
}

void DrawCircleBres(HDC hdc, int xc, int yc, int R, COLORREF c, int quarter = 0)
{
    int x = 0, y = R, d = 1 - R, ch1 = 3, ch2 = 5 - 2 * R;
    Draw8Points(hdc, xc, yc, x, y, c, quarter);
    while (x < y)
    {
        if (d < 0)
        {
            d += ch1;
            ch2 += 2;
        }
        else
        {
            d += ch2;
            ch2 += 4;
            y--;
        }
        ch1 += 2;
        x++;
        Draw8Points(hdc, xc, yc, x, y, c, quarter);

    }
}

void FillCircleWithLines(HDC hdc,
    int cx, int cy, int r,
    int quarter,
    COLORREF c, int step = 5)
{
    for (int y = cy - r; y <= cy + r; y += step)
    {
        double dy = y - cy;
        double temp = r * r - dy * dy;
        if (temp < 0) continue;

        double dx = sqrt(temp);

        int x1 = cx - dx;
        int x2 = cx + dx;

        // clip to quarter
        if (quarter == 1) { if (y > cy) continue; x1 = cx; }
        if (quarter == 2) { if (y > cy) continue; x2 = cx; }
        if (quarter == 3) { if (y < cy) continue; x2 = cx; }
        if (quarter == 4) { if (y < cy) continue; x1 = cx; }

        DrawLine(hdc, x1, y, x2, y, c);
    }
}


void FillCircleWithCircles(HDC hdc,
    int cx, int cy, int R,
    int quarter,
    COLORREF c, int step = 5)
{
    for (int r = R; r > 0; r -= step)
    {
        DrawCircleBres(hdc, cx, cy, r, c, quarter);
    }
}


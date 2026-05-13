#include "Faces.h"

// Midpoint Circle
void DrawMidpointCircle(HDC hdc, int xc, int yc, int r, COLORREF c)
{
    DrawMidpointEllipse(hdc, xc, yc, r, r, c);
}


// Filled Circle
void FillCircle(HDC hdc, int xc, int yc, int r, COLORREF c)
{
    HPEN pen = CreatePen(PS_SOLID, 1, c);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    for (int y = -r; y <= r; y++)
    {
        int dx = (int)sqrt(r * r - y * y);

        MoveToEx(hdc, xc - dx, yc + y, NULL);
        LineTo(hdc, xc + dx, yc + y);
    }

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

// Face Parts
void DrawEye(HDC hdc, int ex, int ey)
{
    FillCircle(hdc, ex, ey, 22, RGB(255,255,255));
    DrawMidpointCircle(hdc, ex, ey, 22, RGB(0,0,0));

    FillCircle(hdc, ex, ey, 10, RGB(20,20,20));
}

void DrawEyebrow(HDC hdc, int x1, int y1, int x2, int y2, COLORREF c)
{
    HPEN pen = CreatePen(PS_SOLID, 3, c);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

void DrawMouth(HDC hdc, int cx, int cy, int hw, int depth, bool happy, COLORREF c)
{
    vector<Point> pts(3);

    pts[0] = { cx - hw, cy };
    pts[1] = { cx, happy ? cy + depth : cy - depth };
    pts[2] = { cx + hw, cy };

    DrawCardinalSpline(hdc, pts, c);
}


// Happy Face
void DrawHappyFace(HDC hdc, int cx, int cy, COLORREF c)
{
    DrawMidpointEllipse(hdc, cx, cy, 140, 160, c);
    int eyeY = cy - 45;

    DrawEye(hdc, cx - 50, eyeY);
    DrawEye(hdc, cx + 50, eyeY);

    DrawEyebrow(hdc, cx - 75, eyeY - 35, cx - 28, eyeY - 45, c);
    DrawEyebrow(hdc, cx + 28, eyeY - 45, cx + 75, eyeY - 35, c);

    DrawMouth(hdc, cx, cy + 30, 70, 50, true, c);
}


// Sad Face
void DrawSadFace(HDC hdc, int cx, int cy, COLORREF c)
{
    DrawMidpointEllipse(hdc, cx, cy, 140, 160, c);

    int eyeY = cy - 35;

    DrawEye(hdc, cx - 50, eyeY);
    DrawEye(hdc, cx + 50, eyeY);

    DrawEyebrow(hdc, cx - 75, eyeY - 35, cx - 28, eyeY - 45, c);
    DrawEyebrow(hdc, cx + 28, eyeY - 45, cx + 75, eyeY - 35, c);

    DrawMouth(hdc, cx, cy + 55, 65, 40, false, c);
}
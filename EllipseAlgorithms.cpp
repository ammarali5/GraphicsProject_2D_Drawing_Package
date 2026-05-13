#include "EllipseAlgorithms.h"
using namespace std;

void Draw4Points(HDC hdc, int xc, int yc, int x, int y, COLORREF c)
{
    SetPixel(hdc, xc + x, yc + y, c);
    SetPixel(hdc, xc - x, yc + y, c);
    SetPixel(hdc, xc + x, yc - y, c);
    SetPixel(hdc, xc - x, yc - y, c);
}

// Direct Ellipse
void DrawDirectEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF c)
{
    double a2 = (double)a * a;
    double b2 = (double)b * b;

    // Draw using x
    for (int x = -a; x <= a; x++)
    {
        double y = b * sqrt(1.0 - ((double)x * x) / a2);

        SetPixel(hdc, xc + x, yc + round(y), c);
        SetPixel(hdc, xc + x, yc - round(y), c);
    }

    // Draw using y
    for (int y = -b; y <= b; y++)
    {
        double x = a * sqrt(1.0 - ((double)y * y) / b2);

        SetPixel(hdc, xc + round(x), yc + y, c);
        SetPixel(hdc, xc - round(x), yc + y, c);
    }
}


// Polar Ellipse
void DrawPolarEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF c)
{
    double step = 1.0 / max(a, b);

    for (double theta = 0; theta <= 2 * 3.14159265; theta += step)
    {
        int x = round(a * cos(theta));
        int y = round(b * sin(theta));

        SetPixel(hdc, xc + x, yc + y, c);
    }
}


// Midpoint Ellipse
void DrawMidpointEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF c)
{
    long long a2 = 1LL * a * a;
    long long b2 = 1LL * b * b;

    int x = 0;
    int y = b;

    long long dx = 2 * b2 * x;
    long long dy = 2 * a2 * y;

    // Region 1
    long long d1 = b2 - a2 * b + a2 / 4;

    while (dx < dy)
    {
        Draw4Points(hdc, xc, yc, x, y, c);

        if (d1 < 0)
        {
            x++;
            dx += 2 * b2;
            d1 += dx + b2;
        }
        else
        {
            x++;
            y--;

            dx += 2 * b2;
            dy -= 2 * a2;

            d1 += dx - dy + b2;
        }
    }

    // Region 2
    double d2 =
        b2 * (x + 0.5) * (x + 0.5) +
        a2 * (y - 1) * (y - 1) -
        a2 * b2;

    while (y >= 0)
    {
        Draw4Points(hdc, xc, yc, x, y, c);

        if (d2 > 0)
        {
            y--;
            dy -= 2 * a2;
            d2 += a2 - dy;
        }
        else
        {
            x++;
            y--;

            dx += 2 * b2;
            dy -= 2 * a2;

            d2 += dx - dy + a2;
        }
    }
}
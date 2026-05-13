#include "Curves.h"
#include "shape.h"
using namespace std;

void hermiteBasis(double t, double& h1, double& h2, double& h3, double& h4)
{
    double t2 = t * t;
    double t3 = t2 * t;

    h1 =  2 * t3 - 3 * t2 + 1;
    h2 = -2 * t3 + 3 * t2;
    h3 =      t3 - 2 * t2 + t;
    h4 =      t3 - t2;
}

void DrawCardinalSpline(HDC hdc, const vector<Point>& pts, COLORREF c)
{
    if (pts.size() < 2)
        return;

    HPEN pen = CreatePen(PS_SOLID, 2, c);
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    double tension = 0.5;
    int subdiv = 50;

    int n = pts.size();

    for (int seg = 0; seg < n - 1; seg++)
    {
        Point p0, p1, p2, p3;

        p1 = pts[seg];
        p2 = pts[seg + 1];

        // First extra point
        if (seg == 0)
        {
            p0.x = p1.x - (p2.x - p1.x);
            p0.y = p1.y - (p2.y - p1.y);
        }
        else
        {
            p0 = pts[seg - 1];
        }

        // Last extra point
        if (seg == n - 2)
        {
            p3.x = p2.x + (p2.x - p1.x);
            p3.y = p2.y + (p2.y - p1.y);
        }
        else
        {
            p3 = pts[seg + 2];
        }

        // Tangents
        double scale = (1.0 - tension) * 0.5;

        double t1x = scale * (p2.x - p0.x);
        double t1y = scale * (p2.y - p0.y);

        double t2x = scale * (p3.x - p1.x);
        double t2y = scale * (p3.y - p1.y);

        double prevX = p1.x;
        double prevY = p1.y;

        for (int k = 1; k <= subdiv; k++)
        {
            double t = (double)k / subdiv;

            double h1, h2, h3, h4;
            hermiteBasis(t, h1, h2, h3, h4);

            double x =
                h1 * p1.x +
                h2 * p2.x +
                h3 * t1x +
                h4 * t2x;

            double y =
                h1 * p1.y +
                h2 * p2.y +
                h3 * t1y +
                h4 * t2y;

            MoveToEx(hdc, round(prevX), round(prevY), NULL);
            LineTo(hdc, round(x), round(y));

            prevX = x;
            prevY = y;
        }
    }

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}
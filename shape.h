#pragma once
#include <vector>
#include <windows.h>
using namespace std;


// ==================== SHAPE TYPES ====================
enum ShapeType {
    LINE_DDA, LINE_MIDPOINT, LINE_PARAMETRIC,
    CIRCLE_DIRECT, CIRCLE_POLAR, CIRCLE_ITERPOLAR,
    CIRCLE_MIDPOINT, CIRCLE_MODMID,
    ELLIPSE_DIRECT, ELLIPSE_POLAR, ELLIPSE_MIDPOINT,
    CARDINAL,
    FILL_CIRCLE_LINES, FILL_CIRCLE_CIRCLES,
    FILL_SQUARE_HERMITE, FILL_RECT_BEZIER,
    FILL_CONVEX, FILL_NONCONVEX,
    FLOOD_RECURSIVE, FLOOD_NONRECURSIVE,
    CLIP_RECT_POINT, CLIP_RECT_LINE, CLIP_RECT_POLYGON,
    CLIP_SQ_POINT, CLIP_SQ_LINE,
    CLIP_CIRCLE_POINT, CLIP_CIRCLE_LINE,
    SMILEY_HAPPY, SMILEY_SAD
};

struct Point {
    int x, y;
    Point(int x = 0, int y = 0) : x(x), y(y) {}
};

class Shape 
{
    public:
        ShapeType type;
        COLORREF color;
        vector<Point> points;
        int quarter = 0;   // quarter for filling
};


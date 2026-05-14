#include <Windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <math.h>
#include <iostream>

#include "shape.h"
#include "LineAlgorithms.h"
#include "CircleAlgorithms.h"
#include "EllipseAlgorithms.h"
#include "Curves.h"
#include "Filling.h"
#include "clipping.h"
#include "Faces.h"
using namespace std;


#define WIN32_LEAN_AND_MEAN

// ==================== MENU IDs ====================
// File Menu
#define ID_FILE_CLEAR       1001
#define ID_FILE_SAVE        1002
#define ID_FILE_LOAD        1003

// Preferences Menu
#define ID_PREF_BGWHITE      2001
#define ID_PREF_BGDefault    2002
#define ID_PREF_CURSOR_arrow 2003
#define ID_PREF_CURSOR_CROSS 2004
#define ID_PREF_CURSOR_HAND  2005
#define ID_PREF_COLOR        2006

// Lines Menu
#define ID_LINE_DDA         3001
#define ID_LINE_MIDPOINT    3002
#define ID_LINE_PARAMETRIC  3003

// Circles Menu
#define ID_CIRCLE_DIRECT    4001
#define ID_CIRCLE_POLAR     4002
#define ID_CIRCLE_ITERPOLAR 4003
#define ID_CIRCLE_MIDPOINT  4004
#define ID_CIRCLE_MODMID    4005

// Ellipse Menu
#define ID_ELLIPSE_DIRECT   5001
#define ID_ELLIPSE_POLAR    5002
#define ID_ELLIPSE_MIDPOINT 5003

// Curves Menu
#define ID_CURVE_CARDINAL   6001

// Filling Menu
#define ID_FILL_CIRCLE_LINES    7001
#define ID_FILL_CIRCLE_CIRCLES  7002
#define ID_FILL_SQUARE_HERMITE  7003
#define ID_FILL_RECT_BEZIER     7004
#define ID_FILL_CONVEX          7005
#define ID_FILL_NONCONVEX       7006
#define ID_FILL_RECURSIVE_FLOOD 7007
#define ID_FILL_NONREC_FLOOD    7008

// Clipping Menu
#define ID_CLIP_RECT_POINT   8001
#define ID_CLIP_RECT_LINE    8002
#define ID_CLIP_RECT_POLYGON 8003
#define ID_CLIP_SQ_POINT     8004
#define ID_CLIP_SQ_LINE      8005
// Bonus clipping
#define ID_CLIP_CIRCLE_POINT 8006
#define ID_CLIP_CIRCLE_LINE  8007

// Smiley Bonus
#define ID_SMILEY_HAPPY      9001
#define ID_SMILEY_SAD        9002


// ==================== Globals ====================
ShapeType mode = LINE_DDA;
vector<Shape> shapes;
int requiredClicks = 2;
vector<Point> clickPoints;

bool whiteBGcolor = false;
HCURSOR hCustomCursor = NULL;
COLORREF CurrentColor = RGB(0,0,0);


// ==================== Helper Functions ====================
int RequiredClicks(ShapeType t) {
    switch(t) {
    case CLIP_RECT_POINT: case CLIP_SQ_POINT:
    case CLIP_CIRCLE_POINT: case FLOOD_RECURSIVE:
    case FLOOD_NONRECURSIVE: case SMILEY_HAPPY:
    case SMILEY_SAD: return 1;
    case CARDINAL: case FILL_CONVEX:
    case FILL_NONCONVEX: case CLIP_RECT_POLYGON: return -1; // multi
    default: return 2;
    }
}

void setBigPixel(HDC hdc, int x, int y, COLORREF c) {
    SetPixel(hdc, x, y, c);
    SetPixel(hdc, x+1, y, c);
    SetPixel(hdc, x, y+1, c);
    SetPixel(hdc, x-1, y, c);
    SetPixel(hdc, x, y-1, c);
}

// ==================== File Handling ====================
// ==================== SAVE / LOAD ====================
void clear(HWND hwnd) {
    shapes.clear();
    clickPoints.clear();
    RECT rc; GetClientRect(hwnd, &rc);
    InvalidateRect(hwnd, &rc, TRUE);
    return;
}

void save(HWND hwnd)
{
    // Get filename from console
    char filename[256];
    printf("Enter filename to save: ");
    scanf("%255s", filename);

    ofstream file(filename, ios::binary);
    if (!file.is_open()) {
        printf("ERROR: Could not open file '%s' for writing.\n", filename);
        return;
    }

    // Write number of shapes
    int count = (int)shapes.size();
    file.write((char*)&count, sizeof(int));

    for (const Shape& s : shapes) {
        // Write type, color, quarter
        file.write((char*)&s.type,    sizeof(ShapeType));
        file.write((char*)&s.color,   sizeof(COLORREF));
        file.write((char*)&s.quarter, sizeof(int));

        // Write points: count then each point
        int npts = (int)s.points.size();
        file.write((char*)&npts, sizeof(int));
        for (const Point& p : s.points) {
            file.write((char*)&p.x, sizeof(int));
            file.write((char*)&p.y, sizeof(int));
        }
    }

    file.close();
    printf("Saved %d shape(s) to '%s'.\n", count, filename);
}

void load(HWND hwnd)
{
    // Get filename from console
    char filename[256];
    printf("Enter filename to load: ");
    scanf("%255s", filename);

    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        printf("ERROR: Could not open file '%s' for reading.\n", filename);
        return;
    }

    // Read number of shapes
    int count = 0;
    file.read((char*)&count, sizeof(int));
    if (file.fail() || count < 0 || count > 100000) {
        printf("ERROR: File '%s' appears corrupt or invalid.\n", filename);
        return;
    }

    // Replace global shapes vector
    shapes.clear();
    shapes.reserve(count);

    for (int i = 0; i < count; i++) {
        Shape s;

        file.read((char*)&s.type,    sizeof(ShapeType));
        file.read((char*)&s.color,   sizeof(COLORREF));
        file.read((char*)&s.quarter, sizeof(int));

        int npts = 0;
        file.read((char*)&npts, sizeof(int));
        if (file.fail() || npts < 0 || npts > 10000) {
            printf("ERROR: Corrupt shape #%d, aborting load.\n", i);
            shapes.clear();
            return;
        }

        s.points.resize(npts);
        for (int j = 0; j < npts; j++) {
            file.read((char*)&s.points[j].x, sizeof(int));
            file.read((char*)&s.points[j].y, sizeof(int));
        }

        shapes.push_back(s);
    }

    file.close();
    printf("Loaded %d shape(s) from '%s'.\n", count, filename);

    InvalidateRect(hwnd, NULL, TRUE);
}
// ==================== DRAW SHAPE ====================
void DrawShape(HDC hdc, const Shape& s) {
    COLORREF c = s.color;
    if (s.points.empty()) return;

    switch(s.type) {
    case LINE_DDA:
        if (s.points.size()>=2) DrawLineDDA(hdc, s.points[0].x, s.points[0].y, s.points[1].x, s.points[1].y, c);
        break;
    case LINE_MIDPOINT:
        if (s.points.size()>=2) DrawLineMidpoint(hdc, s.points[0].x, s.points[0].y, s.points[1].x, s.points[1].y, c);
        break;
    case LINE_PARAMETRIC:
        if (s.points.size()>=2) DrawLineParametric(hdc, s.points[0].x, s.points[0].y, s.points[1].x, s.points[1].y, c);
        break;
    case CIRCLE_DIRECT:
        if (s.points.size()>=2) {
            int r=(int)sqrt((double)((s.points[1].x-s.points[0].x)*(s.points[1].x-s.points[0].x)+(s.points[1].y-s.points[0].y)*(s.points[1].y-s.points[0].y)));
            DrawCircleDirect(hdc, s.points[0].x, s.points[0].y, r, c);
        }
        break;
    case CIRCLE_POLAR:
        if (s.points.size()>=2) {
            int r=(int)sqrt((double)((s.points[1].x-s.points[0].x)*(s.points[1].x-s.points[0].x)+(s.points[1].y-s.points[0].y)*(s.points[1].y-s.points[0].y)));
            DrawCirclePolar(hdc, s.points[0].x, s.points[0].y, r, c);
        }
        break;
    case CIRCLE_ITERPOLAR:
        if (s.points.size()>=2) {
            int r=(int)sqrt((double)((s.points[1].x-s.points[0].x)*(s.points[1].x-s.points[0].x)+(s.points[1].y-s.points[0].y)*(s.points[1].y-s.points[0].y)));
            DrawCircleIterativePolar(hdc, s.points[0].x, s.points[0].y, r, c);
        }
        break;
    case CIRCLE_MIDPOINT:
        if (s.points.size()>=2) {
            int r=(int)sqrt((double)((s.points[1].x-s.points[0].x)*(s.points[1].x-s.points[0].x)+(s.points[1].y-s.points[0].y)*(s.points[1].y-s.points[0].y)));
            DrawCircleMidpoint(hdc, s.points[0].x, s.points[0].y, r, c);
        }
        break;
    case CIRCLE_MODMID:
        if (s.points.size()>=2) {
            int r=(int)sqrt((double)((s.points[1].x-s.points[0].x)*(s.points[1].x-s.points[0].x)+(s.points[1].y-s.points[0].y)*(s.points[1].y-s.points[0].y)));
            DrawCircleModifiedMidpoint(hdc, s.points[0].x, s.points[0].y, r, c);
        }
        break;
    case ELLIPSE_DIRECT:
        if (s.points.size()>=2) {
            int rx=abs(s.points[1].x-s.points[0].x), ry=abs(s.points[1].y-s.points[0].y);
            DrawDirectEllipse(hdc, s.points[0].x, s.points[0].y, rx, ry, c);
        }
        break;
    case ELLIPSE_POLAR:
        if (s.points.size()>=2) {
            int rx=abs(s.points[1].x-s.points[0].x), ry=abs(s.points[1].y-s.points[0].y);
            DrawPolarEllipse(hdc, s.points[0].x, s.points[0].y, rx, ry, c);
        }
        break;
    case ELLIPSE_MIDPOINT:
        if (s.points.size()>=2) {
            int rx=abs(s.points[1].x-s.points[0].x), ry=abs(s.points[1].y-s.points[0].y);
            DrawMidpointEllipse(hdc, s.points[0].x, s.points[0].y, rx, ry, c);
        }
        break;
    case CARDINAL:
        DrawCardinalSpline(hdc, s.points, c);
        break;
     case FILL_CIRCLE_LINES:
        if(s.points.size() == 1){
                
            } else if(s.points.size() == 2){
                Point p1 = s.points[0];
                Point p2 = s.points[1];
                int radius = sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2));
                DrawCircleBres(hdc, p1.x, p1.y, radius, c);
            } else if(s.points.size() == 3){
                Point center = s.points[0];
                Point quarter = s.points[2];
                int radius = sqrt(pow((s.points[0].x - s.points[1].x), 2) + pow((s.points[0].y - s.points[1].y), 2));

                int q = 0;
                if(quarter.y > center.y){
                    if(quarter.x > center.x){
                        q = 4;
                    } else{
                        q = 3;
                    }
                } else {
                    if(quarter.x > center.x){
                        q = 1;
                    } else q = 2;
                }
                FillCircleWithLines(hdc, center.x, center.y, radius, q, s.color);
            }
        break;
     case FILL_CIRCLE_CIRCLES:
              if(s.points.size() == 1){
                
            } else if(s.points.size() == 2){
                Point p1 = s.points[0];
                Point p2 = s.points[1];
                int radius = sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2));
                DrawCircleBres(hdc, p1.x, p1.y, radius, c);
            } else if(s.points.size() == 3){
                Point center = s.points[0];
                Point quarter = s.points[2];
                int radius = sqrt(pow((s.points[0].x - s.points[1].x), 2) + pow((s.points[0].y - s.points[1].y), 2));

                int q = 0;
                if(quarter.y > center.y){
                    if(quarter.x > center.x){
                        q = 4;
                    } else{
                        q = 3;
                    }
                } else {
                    if(quarter.x > center.x){
                        q = 1;
                    } else q = 2;
                }
                FillCircleWithCircles(hdc, center.x, center.y, radius, q, s.color);
            }
        break;
    case FILL_SQUARE_HERMITE:
        if(s.points.size() == 2){
                Point topLeft = s.points[0];
                Point botRight = s.points[1];

                vector<Point> p =
                {
                        Point(topLeft.x,     topLeft.y),
                        Point(botRight.x,    topLeft.y),  
                        Point(botRight.x,    botRight.y),    
                        Point(topLeft.x,     botRight.y)
                };
                SquareHermiteFilling(hdc, p, s.color);
            }
        break;
    case FILL_RECT_BEZIER:
        if(s.points.size() == 2){
                Point topLeft = s.points[0];
                Point botRight = s.points[1];

                vector<Point> p =
                {
                        Point(topLeft.x,     topLeft.y),
                        Point(botRight.x,    topLeft.y),  
                        Point(botRight.x,    botRight.y),    
                        Point(topLeft.x,     botRight.y)
                };
                FillRectangleBezier(hdc, p, s.color);
            }
        break;
    case FILL_CONVEX:
        ConvexFill(hdc, s.points, s.color);
        break;
    case FILL_NONCONVEX:
        NonConvexFill(hdc, s.points, s.color);
        break;
    case FLOOD_NONRECURSIVE:
        if (s.points.size()>=1) {
           
        }
        break;
    case CLIP_RECT_POINT:
        if (s.points.size()>=1) {
            ClipRect r={200,150,600,450};
            DrawClipRect(hdc, r, RGB(255,0,0));
            // Draw point
            int px=s.points[0].x, py=s.points[0].y;
            if(ClipPoint_Rectangle(px, py, r))
            {
                setBigPixel(hdc,px,py,c);
            }
            printf("Clip Rect Point: %s\n", (ClipPoint_Rectangle(px, py, r))?"inside":"outside");
        }
        break;
    case CLIP_RECT_LINE:
        if (s.points.size()>=2) {
            ClipRect r={200,150,600,450};
            DrawClipRect(hdc, r, RGB(255,0,0));
            // Draw original line faintly
            int x1=s.points[0].x, y1=s.points[0].y, x2=s.points[1].x, y2=s.points[1].y;
            if (ClipLine_CohenSutherland(x1,y1,x2,y2,r))
                DrawLineDDA(hdc, x1, y1, x2, y2, c);
            printf("Clip Rect Line\n");
        }
        break;
    case CLIP_RECT_POLYGON:
        if (s.points.size()>=3) {
            ClipRect r={200,150,600,450};
            DrawClipRect(hdc, r, RGB(255,0,0));
            // Draw original polygon faintly
            for (int i=0;i<(int)s.points.size();i++) {
                int j=(i+1)%s.points.size();
                DrawLineDDA(hdc, s.points[i].x, s.points[i].y, s.points[j].x, s.points[j].y, RGB(200,200,200));
            }
            auto clipped=ClipPolygon_SutherlandHodgman(s.points, r);
            for (int i=0;i<(int)clipped.size();i++) {
                int j=(i+1)%clipped.size();
                DrawLineDDA(hdc, clipped[i].x, clipped[i].y, clipped[j].x, clipped[j].y, c);
            }
            printf("Clip Rect Polygon: %d -> %d vertices\n", (int)s.points.size(), (int)clipped.size());
        }
        break;
    case CLIP_SQ_POINT:
        if (s.points.size()>=1) {
            ClipRect r={250,175,550,425}; // Square
            DrawClipRect(hdc, r, RGB(0,0,255));
            int px=s.points[0].x, py=s.points[0].y;
            if(ClipPoint_Rectangle(px, py, r))
            {
                setBigPixel(hdc,px,py,c);
            }
            printf("Clip Square Point\n");
        }
        break;
    case CLIP_SQ_LINE:
        if (s.points.size()>=2) {
            ClipRect r={250,175,550,425};
            DrawClipRect(hdc, r, RGB(0,0,255));
            int x1=s.points[0].x, y1=s.points[0].y, x2=s.points[1].x, y2=s.points[1].y;
            if (ClipLine_CohenSutherland(x1,y1,x2,y2,r))
                DrawLineDDA(hdc, x1, y1, x2, y2, c);
            printf("Clip Square Line\n");
        }
        break;
    case CLIP_CIRCLE_POINT:
        if (s.points.size()>=1) {
            int cx=400, cy=300, cr=150;
            DrawCircleMidpoint(hdc, cx, cy, cr, RGB(255,0,0));
            int px=s.points[0].x, py=s.points[0].y;
            if(ClipPoint_Circle(px,py,cx,cy,cr) ) 
            {
                setBigPixel(hdc,px,py,c);
            }
            printf("Clip Circle Point (BONUS)\n");
        }
        break;
    case CLIP_CIRCLE_LINE:
        if (s.points.size()>=2) {
            int cx=400, cy=300, cr=150;
            DrawCircleMidpoint(hdc, cx, cy, cr, RGB(255,0,0));
            // Clip line by circle: draw only inside part
            int x1=s.points[0].x, y1=s.points[0].y, x2=s.points[1].x, y2=s.points[1].y;
            int dx=x2-x1, dy=y2-y1;
            int steps=max(abs(dx),abs(dy));
            if (steps>0) {
                for (int i=0;i<=steps;i++) {
                    int px=x1+dx*i/steps, py=y1+dy*i/steps;
                    if (ClipPoint_Circle(px,py,cx,cy,cr)) SetPixel(hdc,px,py,c);
                }
            }
            printf("Clip Circle Line (BONUS)\n");
        }
        break;
    case SMILEY_HAPPY:
        if (s.points.size()>=1) DrawHappyFace(hdc, s.points[0].x, s.points[0].y, c);
        break;
    case SMILEY_SAD:
        if (s.points.size()>=1) DrawSadFace(hdc, s.points[0].x, s.points[0].y, c);
        break;
    }
}

// ==================== Menus Creation ====================
HMENU CreateMainMenu()
{
    HMENU hMenu = CreateMenu();
    
    // File
    HMENU fileMenu = CreatePopupMenu();
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_CLEAR, "Clear");
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_SAVE, "Save");
    AppendMenuA(fileMenu, MF_STRING, ID_FILE_LOAD, "Load");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)fileMenu, "File");
    
    // Preferences
    HMENU PreferencesMenu = CreatePopupMenu();
    HMENU PreferencesBGMenu = CreatePopupMenu();
    AppendMenuA(PreferencesBGMenu, MF_STRING, ID_PREF_BGWHITE, "White");
    AppendMenuA(PreferencesBGMenu, MF_STRING, ID_PREF_BGDefault, "Default");
    AppendMenuA(PreferencesMenu, MF_POPUP, (UINT_PTR)PreferencesBGMenu, "Background Color");
    HMENU PreferencesCursorMenu = CreatePopupMenu();
    AppendMenuA(PreferencesCursorMenu, MF_STRING, ID_PREF_CURSOR_arrow, "Arrow");
    AppendMenuA(PreferencesCursorMenu, MF_STRING, ID_PREF_CURSOR_CROSS, "Cross");
    AppendMenuA(PreferencesCursorMenu, MF_STRING, ID_PREF_CURSOR_HAND, "Hand");
    AppendMenuA(PreferencesMenu, MF_POPUP, (UINT_PTR)PreferencesCursorMenu, "Cursor");
    AppendMenuA(PreferencesMenu, MF_STRING, ID_PREF_COLOR, "Choose Color");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)PreferencesMenu, "Preferences");
    
    // Lines
    HMENU lineMenu = CreatePopupMenu();
    AppendMenuA(lineMenu, MF_STRING, ID_LINE_DDA, "DDA");
    AppendMenuA(lineMenu, MF_STRING, ID_LINE_MIDPOINT, "Midpoint");
    AppendMenuA(lineMenu, MF_STRING, ID_LINE_PARAMETRIC, "Parametric");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)lineMenu, "Lines");

    // Circles
    HMENU circleMenu = CreatePopupMenu();
    AppendMenuA(circleMenu, MF_STRING, ID_CIRCLE_DIRECT, "Direct");
    AppendMenuA(circleMenu, MF_STRING, ID_CIRCLE_POLAR, "Polar");
    AppendMenuA(circleMenu, MF_STRING, ID_CIRCLE_ITERPOLAR, "Iterative Polar");
    AppendMenuA(circleMenu, MF_STRING, ID_CIRCLE_MIDPOINT, "Midpoint");
    AppendMenuA(circleMenu, MF_STRING, ID_CIRCLE_MODMID, "Modified Midpoint");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)circleMenu, "Circles");
    
    // Ellipse
    HMENU EllipseMenu = CreatePopupMenu();
    AppendMenuA(EllipseMenu, MF_STRING, ID_ELLIPSE_DIRECT, "Direct");
    AppendMenuA(EllipseMenu, MF_STRING, ID_ELLIPSE_POLAR, "Polar");
    AppendMenuA(EllipseMenu, MF_STRING, ID_ELLIPSE_MIDPOINT, "Midpoint");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)EllipseMenu, "Ellipse");

    // Curves
    HMENU CurvesMenu = CreatePopupMenu();
    AppendMenuA(CurvesMenu, MF_STRING, ID_CURVE_CARDINAL, "Cardinal Spline");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)CurvesMenu, "Curves");

    // Filling
    HMENU FillingMenu = CreatePopupMenu();
    AppendMenuA(FillingMenu, MF_STRING, ID_FILL_CIRCLE_LINES, "Circle with lines");
    AppendMenuA(FillingMenu, MF_STRING, ID_FILL_CIRCLE_CIRCLES, "Circle with Circles");
    AppendMenuA(FillingMenu, MF_STRING, ID_FILL_SQUARE_HERMITE, "Square with Hermite");
    AppendMenuA(FillingMenu, MF_STRING, ID_FILL_RECT_BEZIER, "Rectangle with Bezier");
    AppendMenuA(FillingMenu, MF_STRING, ID_FILL_CONVEX, "Convex Fill");
    AppendMenuA(FillingMenu, MF_STRING, ID_FILL_NONCONVEX, "Non-Convex Fill");
    AppendMenuA(FillingMenu, MF_STRING, ID_FILL_RECURSIVE_FLOOD, "Recursive Flood Fill");
    AppendMenuA(FillingMenu, MF_STRING, ID_FILL_NONREC_FLOOD, "Non-Recursive Flood Fill");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)FillingMenu, "Filling");

    // Clipping
    HMENU ClippingMenu = CreatePopupMenu();
    HMENU ClippingRectangleMenu = CreatePopupMenu();
    AppendMenuA(ClippingRectangleMenu, MF_STRING, ID_CLIP_RECT_POINT, "Point");
    AppendMenuA(ClippingRectangleMenu, MF_STRING, ID_CLIP_RECT_LINE, "Line");
    AppendMenuA(ClippingRectangleMenu, MF_STRING, ID_CLIP_RECT_POLYGON, "Polygon");
    AppendMenuA(ClippingMenu, MF_POPUP, (UINT_PTR)ClippingRectangleMenu, "Rectangle");
    HMENU ClippingSquareMenu = CreatePopupMenu();
    AppendMenuA(ClippingSquareMenu, MF_STRING, ID_CLIP_SQ_POINT, "Point");
    AppendMenuA(ClippingSquareMenu, MF_STRING, ID_CLIP_SQ_LINE, "Line");
    AppendMenuA(ClippingMenu, MF_POPUP, (UINT_PTR)ClippingSquareMenu, "Square");
    // Bonus circle clipping
    HMENU ClippingCircleMenu = CreatePopupMenu();
    AppendMenuA(ClippingCircleMenu, MF_STRING, ID_CLIP_CIRCLE_POINT, "Point");
    AppendMenuA(ClippingCircleMenu, MF_STRING, ID_CLIP_CIRCLE_LINE, "Line");
    AppendMenuA(ClippingMenu, MF_POPUP, (UINT_PTR)ClippingCircleMenu, "Circle");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)ClippingMenu, "Clipping");

    // Bonus: Smiley
    HMENU hBonus = CreatePopupMenu();
    AppendMenuA(hBonus, MF_STRING, ID_SMILEY_HAPPY, "Happy Face");
    AppendMenuA(hBonus, MF_STRING, ID_SMILEY_SAD, "Sad Face");
    AppendMenuA(hMenu, MF_POPUP, (UINT_PTR)hBonus, "Smiley [BONUS]");

    return hMenu;
}


LRESULT WINAPI WndProc(HWND hwnd, UINT mcode, WPARAM wp, LPARAM lp)
{
	HDC hdc;
	switch (mcode)
	{
    case WM_CREATE:
    {
        printf("===== 2D Drawing Package =====\n");
        printf("Select a tool from the menu, then click on the canvas.\n");
        printf("Right-click finishes multi-point shapes (Cardinal, Polygon fill, etc.)\n");
        break;
    }
    
	case WM_LBUTTONDOWN:
    {
        int mx = LOWORD(lp);
        int my = HIWORD(lp);
        hdc = GetDC(hwnd);
        clickPoints.push_back({mx, my});
        // Draw preview dot
         if (requiredClicks!=1 && mode != CLIP_RECT_POINT && mode != CLIP_SQ_POINT && mode != CLIP_CIRCLE_POINT) {
            setBigPixel(hdc, mx, my, CurrentColor);
        }

        if(mode == ShapeType::FILL_CIRCLE_LINES){
            Shape s;
            s.type = mode;
            s.color = CurrentColor;
            s.quarter = 1;
            s.points = clickPoints;
            shapes.push_back(s);
            DrawShape(hdc, shapes.back());
            if(clickPoints.size() == 3){
                clickPoints.clear();
            }
        } else if(mode == ShapeType::FILL_CIRCLE_CIRCLES){
            Shape s;
            s.type = mode;
            s.color = CurrentColor;
            s.quarter = 1;
            s.points = clickPoints;
            shapes.push_back(s);
            DrawShape(hdc, shapes.back());
            if(clickPoints.size() == 3){
                clickPoints.clear();
            }
        } else if(mode == ShapeType::FILL_SQUARE_HERMITE){
            Shape s;
            s.type = mode;
            s.color = CurrentColor;
            s.quarter = 1;
            s.points = clickPoints;
            shapes.push_back(s);
            DrawShape(hdc, shapes.back());
            if(clickPoints.size() == 2){
                clickPoints.clear();
            }
        } else if(mode == ShapeType::FILL_RECT_BEZIER){
            Shape s;
            s.type = mode;
            s.color = CurrentColor;
            s.quarter = 1;
            s.points = clickPoints;
            shapes.push_back(s);
            DrawShape(hdc, shapes.back());
            if(clickPoints.size() == 2){
                clickPoints.clear();
            }
        } else if(mode == ShapeType::FILL_CONVEX){
            
        } else if(mode == ShapeType::FILL_NONCONVEX){

        } else if(mode == ShapeType::FLOOD_RECURSIVE){
             RecursiveFloodFillCall(hdc, mx + 2, my, CurrentColor);
             clickPoints.clear();
        } else if(mode == ShapeType::FLOOD_NONRECURSIVE){
            NonRecursiveFloodFill(hdc, mx + 2, my, CurrentColor);
        } else if (requiredClicks>0 && (int)clickPoints.size() >= requiredClicks) {
            Shape s;
            s.type = mode;
            s.color = CurrentColor;
            s.quarter = 1;
            s.points = clickPoints;
            
            if (mode==FILL_CIRCLE_LINES||mode==FILL_CIRCLE_CIRCLES) {
                printf("Enter quarter (1=top-right, 2=top-left, 3=bottom-left, 4=bottom-right): ");
                scanf("%d", &s.quarter);
            }
            
            shapes.push_back(s);
            DrawShape(hdc, shapes.back());
            clickPoints.clear();
            printf("Shape complete (%d clicks)\n", requiredClicks);
            printf("Shape placed at (%d,%d)\n", mx, my);
        } else if (requiredClicks==-1) {
            printf("Point added (%d total) - right-click to finish\n", (int)clickPoints.size());
        }

        ReleaseDC(hwnd, hdc);
		break;
    }

    case WM_RBUTTONDOWN:
    {
        // Finish multi-click shapes
        hdc = GetDC(hwnd);
        if(mode == ShapeType::FILL_CONVEX){
            Shape s;
            s.type = mode;
            s.color = CurrentColor;
            s.quarter = 1;
            s.points = clickPoints;
            shapes.push_back(s);
            DrawShape(hdc, shapes.back());
            clickPoints.clear();
            
        } else if(mode == ShapeType::FILL_NONCONVEX){
            Shape s;
            s.type = mode;
            s.color = CurrentColor;
            s.quarter = 1;
            s.points = clickPoints;
            shapes.push_back(s);
            DrawShape(hdc, shapes.back());
            clickPoints.clear();
        } else if (requiredClicks == -1 && clickPoints.size() >= 3) {
            Shape s;
            s.type = mode;
            s.color = CurrentColor;
            s.quarter = 1;
            s.points = clickPoints;
            shapes.push_back(s);
            DrawShape(hdc, shapes.back());
            clickPoints.clear();
            printf("Multi-point shape finalized\n");
        }
        ReleaseDC(hwnd, hdc);
        break;
    }

    case WM_SETCURSOR:
    {
        if (hCustomCursor) {
            SetCursor(hCustomCursor);
            return TRUE;
        }   
        return DefWindowProc(hwnd, mcode, wp, lp);
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wp))
        {
        // File
        case ID_FILE_CLEAR: clear(hwnd); break;
        case ID_FILE_SAVE: save(hwnd); break;
        case ID_FILE_LOAD: load(hwnd); break;
        // Preferences
        case ID_PREF_BGWHITE:
        {
            whiteBGcolor = true;
            SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(RGB(255, 255, 255)));
            printf("Background set to white\n");
            InvalidateRect(hwnd, NULL, TRUE); break;
        }
        case ID_PREF_BGDefault:
        {
            whiteBGcolor = false;
            SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)LTGRAY_BRUSH);
            printf("Background set to default\n");
            InvalidateRect(hwnd, NULL, TRUE); break;
        }
        case ID_PREF_CURSOR_arrow:
        {
            hCustomCursor = LoadCursor(NULL, IDC_ARROW);
            printf("Custom cursor: ARROW\n");
            break;
        }
        case ID_PREF_CURSOR_CROSS:
        {
            hCustomCursor = LoadCursor(NULL, IDC_CROSS);
            printf("Custom cursor: CROSS\n");
            break;
        }
        case ID_PREF_CURSOR_HAND:
        {
            hCustomCursor = LoadCursor(NULL, IDC_HAND);
            printf("Custom cursor: HAND\n");
            break;
        }
        case ID_PREF_COLOR:
        {
            CHOOSECOLORA cc;
            static COLORREF customColors[16];

            ZeroMemory(&cc, sizeof(cc));

            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hwnd;
            cc.lpCustColors = customColors;
            cc.rgbResult = CurrentColor;

            cc.Flags = CC_FULLOPEN | CC_RGBINIT;

            if (ChooseColorA(&cc))
            {
                CurrentColor = cc.rgbResult;
                printf("Color changed to R=%d G=%d B=%d\n", GetRValue(CurrentColor),GetGValue(CurrentColor),GetBValue(CurrentColor));
            }
            break;
        }
        // shapes
        // Lines
        case ID_LINE_DDA: mode=LINE_DDA; requiredClicks=RequiredClicks(LINE_DDA); clickPoints.clear(); printf("Tool: DDA Line\n"); break;
        case ID_LINE_MIDPOINT: mode=LINE_MIDPOINT; requiredClicks=RequiredClicks(LINE_MIDPOINT); clickPoints.clear(); printf("Tool: Midpoint Line\n"); break;
        case ID_LINE_PARAMETRIC: mode=LINE_PARAMETRIC; requiredClicks=RequiredClicks(LINE_PARAMETRIC); clickPoints.clear(); printf("Tool: Parametric Line\n"); break;
        // Circles
        case ID_CIRCLE_DIRECT: mode=CIRCLE_DIRECT; requiredClicks=RequiredClicks(CIRCLE_DIRECT); clickPoints.clear(); printf("Tool: Direct Circle\n"); break;
        case ID_CIRCLE_POLAR: mode=CIRCLE_POLAR; requiredClicks=RequiredClicks(CIRCLE_POLAR); clickPoints.clear(); printf("Tool: Polar Circle\n"); break;
        case ID_CIRCLE_ITERPOLAR: mode=CIRCLE_ITERPOLAR; requiredClicks=RequiredClicks(CIRCLE_ITERPOLAR); clickPoints.clear(); printf("Tool: Iterative Polar Circle\n"); break;
        case ID_CIRCLE_MIDPOINT: mode=CIRCLE_MIDPOINT; requiredClicks=RequiredClicks(CIRCLE_MIDPOINT); clickPoints.clear(); printf("Tool: Midpoint Circle\n"); break;
        case ID_CIRCLE_MODMID: mode=CIRCLE_MODMID; requiredClicks=RequiredClicks(CIRCLE_MODMID); clickPoints.clear(); printf("Tool: Modified Midpoint Circle\n"); break;
        // Ellipse
        case ID_ELLIPSE_DIRECT: mode=ELLIPSE_DIRECT; requiredClicks=RequiredClicks(ELLIPSE_DIRECT); clickPoints.clear(); printf("Tool: Direct Ellipse\n"); break;
        case ID_ELLIPSE_POLAR: mode=ELLIPSE_POLAR; requiredClicks=RequiredClicks(ELLIPSE_POLAR); clickPoints.clear(); printf("Tool: Polar Ellipse\n"); break;
        case ID_ELLIPSE_MIDPOINT: mode=ELLIPSE_MIDPOINT; requiredClicks=RequiredClicks(ELLIPSE_MIDPOINT); clickPoints.clear(); printf("Tool: Midpoint Ellipse\n"); break;
        // Curves
        case ID_CURVE_CARDINAL: mode=CARDINAL; requiredClicks=RequiredClicks(CARDINAL); clickPoints.clear(); printf("Tool: Cardinal Curves\n"); break;
        // Filling
        case ID_FILL_CIRCLE_LINES: mode=FILL_CIRCLE_LINES; requiredClicks=RequiredClicks(FILL_CIRCLE_LINES); clickPoints.clear(); printf("Tool: Fill Circle Lines\n"); break;
        case ID_FILL_CIRCLE_CIRCLES: mode=FILL_CIRCLE_CIRCLES; requiredClicks=RequiredClicks(FILL_CIRCLE_CIRCLES); clickPoints.clear(); printf("Tool: Fill Circle Circles\n"); break;
        case ID_FILL_SQUARE_HERMITE: mode=FILL_SQUARE_HERMITE; requiredClicks=RequiredClicks(FILL_SQUARE_HERMITE); clickPoints.clear(); printf("Tool: Fill Square Hermite\n"); break;
        case ID_FILL_RECT_BEZIER: mode=FILL_RECT_BEZIER; requiredClicks=RequiredClicks(FILL_RECT_BEZIER); clickPoints.clear(); printf("Tool: Fill Rectangle Bezier\n"); break;
        case ID_FILL_CONVEX: mode=FILL_CONVEX; requiredClicks=RequiredClicks(FILL_CONVEX); clickPoints.clear(); printf("Tool: Fill Convex Polygon\n"); break;
        case ID_FILL_NONCONVEX: mode=FILL_NONCONVEX; requiredClicks=RequiredClicks(FILL_NONCONVEX); clickPoints.clear(); printf("Tool: Fill Non-Convex Polygon\n"); break;
        case ID_FILL_RECURSIVE_FLOOD:mode=FLOOD_RECURSIVE; requiredClicks=RequiredClicks(FLOOD_RECURSIVE); clickPoints.clear(); printf("Tool: Recursive Flood Fill\n"); break;
        case ID_FILL_NONREC_FLOOD: mode=FLOOD_NONRECURSIVE; requiredClicks=RequiredClicks(FLOOD_NONRECURSIVE); clickPoints.clear(); printf("Tool: Non-Recursive Flood Fill\n"); break;
        // Clipping
        case ID_CLIP_RECT_POINT: mode=CLIP_RECT_POINT; requiredClicks=RequiredClicks(CLIP_RECT_POINT); clickPoints.clear(); printf("Tool: Clip Rectangle Point\n"); break;
        case ID_CLIP_RECT_LINE: mode=CLIP_RECT_LINE; requiredClicks=RequiredClicks(CLIP_RECT_LINE); clickPoints.clear(); printf("Tool: Clip Rectangle Line\n"); break;
        case ID_CLIP_RECT_POLYGON: mode=CLIP_RECT_POLYGON; requiredClicks=RequiredClicks(CLIP_RECT_POLYGON); clickPoints.clear(); printf("Tool: Clip Rectangle Polygon\n"); break;
        case ID_CLIP_SQ_POINT: mode=CLIP_SQ_POINT; requiredClicks=RequiredClicks(CLIP_SQ_POINT); clickPoints.clear(); printf("Tool: Clip Square Point\n"); break;
        case ID_CLIP_SQ_LINE: mode=CLIP_SQ_LINE; requiredClicks=RequiredClicks(CLIP_SQ_LINE); clickPoints.clear(); printf("Tool: Clip Square Line\n"); break;
        case ID_CLIP_CIRCLE_POINT: mode=CLIP_CIRCLE_POINT; requiredClicks=RequiredClicks(CLIP_CIRCLE_POINT); clickPoints.clear(); printf("Tool: Clip Circle Point\n"); break;
        case ID_CLIP_CIRCLE_LINE: mode=CLIP_CIRCLE_LINE; requiredClicks=RequiredClicks(CLIP_CIRCLE_LINE); clickPoints.clear(); printf("Tool: Clip Circle Line\n"); break;
        // Bonus smiley
        case ID_SMILEY_HAPPY: mode=SMILEY_HAPPY; requiredClicks=RequiredClicks(SMILEY_HAPPY); clickPoints.clear(); printf("Tool: Happy Smiley\n"); break;
        case ID_SMILEY_SAD: mode=SMILEY_SAD; requiredClicks=RequiredClicks(SMILEY_SAD); clickPoints.clear(); printf("Tool: Sad Smiley\n"); break;

        default: break;
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        // Fill background
        RECT rc; GetClientRect(hwnd, &rc);
        HBRUSH hbr = CreateSolidBrush(whiteBGcolor ? RGB(255,255,255) : RGB(211,211,211));
        FillRect(hdc, &rc, hbr);
        DeleteObject(hbr);
        // Redraw all shapes
        for (auto& s : shapes) DrawShape(hdc, s);
        EndPaint(hwnd, &ps);
        break;
    }

		
	case WM_CLOSE: DestroyWindow(hwnd); break;
	case WM_DESTROY: PostQuitMessage(0); break;
	default: return DefWindowProc(hwnd, mcode, wp, lp);
	}
	return 0;
}


int APIENTRY WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int nsh)
{
    // Create console for debug output
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONIN$",  "r", stdin);

	WNDCLASSA wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hInstance = h;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "Graphics Project";
	wc.lpszMenuName = NULL;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClassA(&wc);

    HMENU hMenuBar = CreateMainMenu();
	HWND hwnd = CreateWindowA("Graphics Project", "Graphics Project", 
		WS_OVERLAPPEDWINDOW, 100, 100, 1000, 700, NULL, hMenuBar, h, 0);
	ShowWindow(hwnd, nsh);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
#include <Windows.h>
#include "Filling.h"
#include <commctrl.h>
#include <commdlg.h>
#include <vector>
#include <string>
#include <iostream>
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

// Globals
string mode;
vector<Point> pts;
bool whiteBGcolor = false;
HCURSOR hCustomCursor = NULL;
COLORREF CurrentColor = RGB(0,0,0);

void clear(HWND hwnd) {
    InvalidateRect(hwnd, NULL, TRUE);
    return;
}

void save(HWND hwnd) {
    return;
}

void load(HWND hwnd) {
    return;
}


LRESULT WINAPI WndProc(HWND hwnd, UINT mcode, WPARAM wp, LPARAM lp)
{
	HDC hdc;
	switch (mcode)
	{
	case WM_LBUTTONDOWN:
    {
        int mx = LOWORD(lp);
        int my = HIWORD(lp);
        COLORREF c = RGB(0, 0, 255);
		hdc = GetDC(hwnd);
        pts.push_back(Point(mx, my));

        if(mode == "FILL_CIRCLE_LINES"){
            if(pts.size() == 1){
                
            } else if(pts.size() == 2){
                Point p1 = pts[0];
                Point p2 = pts[1];
                int radius = sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2));
                DrawCircleBres(hdc, p1.x, p1.y, radius, c);
            } else if(pts.size() == 3){
                Point center = pts[0];
                Point quarter = pts[2];
                int radius = sqrt(pow((pts[0].x - pts[1].x), 2) + pow((pts[0].y - pts[1].y), 2));

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
                FillCircleWithLines(hdc, center.x, center.y, radius, q, c);
            }
        } else if(mode == "FILL_CIRCLE_CIRCLES"){
            if(pts.size() == 1){
                
            } else if(pts.size() == 2){
                Point p1 = pts[0];
                Point p2 = pts[1];
                int radius = sqrt(pow((p1.x - p2.x), 2) + pow((p1.y - p2.y), 2));
                DrawCircleBres(hdc, p1.x, p1.y, radius, c);
            } else if(pts.size() == 3){
                Point center = pts[0];
                Point quarter = pts[2];
                int radius = sqrt(pow((pts[0].x - pts[1].x), 2) + pow((pts[0].y - pts[1].y), 2));

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
                FillCircleWithCircles(hdc, center.x, center.y, radius, q, c);
            }
        } else if(mode == "FILL_SQUARE_HERMITE"){
            if(pts.size() == 2){
                Point topLeft = pts[0];
                Point botRight = pts[1];

                vector<Point> p =
                {
                        Point(topLeft.x,     topLeft.y),
                        Point(botRight.x,    topLeft.y),  
                        Point(botRight.x,    botRight.y),    
                        Point(topLeft.x,     botRight.y)
                };
                SquareHermiteFilling(hdc, p, c);
                pts.clear();
            }
           
        } else if(mode == "FILL_RECT_BEZIER"){
            if(pts.size() == 2){
                Point topLeft = pts[0];
                Point botRight = pts[1];

                vector<Point> p =
                {
                        Point(topLeft.x,     topLeft.y),
                        Point(botRight.x,    topLeft.y),  
                        Point(botRight.x,    botRight.y),    
                        Point(topLeft.x,     botRight.y)
                };
                FillRectangleBezier(hdc, p, c);
                pts.clear();
            }
        } else if(mode == "FILL_CONVEX"){
            if(pts.size() == 4){
            ConvexFill(hdc, pts, c);
            pts.clear();
            } else {
                DrawBoldPoint(hdc, pts[pts.size() - 1].x,pts[pts.size() - 1].y, RGB(0, 0, 0));
            }
        } else if(mode == "FILL_NONCONVEX"){
            if(pts.size() == 6){
                NonConvexFill(hdc, pts, c);
                pts.clear();
            } else {
               DrawBoldPoint(hdc, pts[pts.size() - 1].x,pts[pts.size() - 1].y, RGB(0, 0, 0)); 
            }
        } else if(mode == "FILL_RECURSIVE_FLOOD"){
            RecursiveFloodFillCall(hdc, pts[0].x, pts[0].y, c);
        } else if(mode == "FILL_NONREC_FLOOD"){
            NonRecursiveFloodFill(hdc, pts[0].x, pts[0].y, c);
        }
		ReleaseDC(hwnd, hdc);
		break;
    }

    case WM_RBUTTONDOWN:
    {
		hdc = GetDC(hwnd);
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
            InvalidateRect(hwnd, NULL, TRUE); break;
        }
        case ID_PREF_BGDefault:
        {
            whiteBGcolor = false;
            SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)LTGRAY_BRUSH);
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
                printf("Color changed to R=%d G=%d B=%d", GetRValue(CurrentColor),GetGValue(CurrentColor),GetBValue(CurrentColor));
            }
            break;
        }
        case ID_FILL_CIRCLE_LINES:
        {
            pts.clear();
            mode = "FILL_CIRCLE_LINES";
            break;
        }
        case ID_FILL_CIRCLE_CIRCLES:
        {
            pts.clear();
            mode = "FILL_CIRCLE_CIRCLES";
            break;
        }
        case ID_FILL_SQUARE_HERMITE:
        {
            pts.clear();
            mode = "FILL_SQUARE_HERMITE";
            break;
        }
        case ID_FILL_RECT_BEZIER:
        {
            pts.clear();
            mode = "FILL_RECT_BEZIER";
            break;
        }
        case ID_FILL_CONVEX:
        {
            pts.clear();
            mode = "FILL_CONVEX";
            break;
        }
        case ID_FILL_NONCONVEX:
        {
            pts.clear();
            mode = "FILL_NONCONVEX";
            break;
        }
        case ID_FILL_RECURSIVE_FLOOD:
        {
            pts.clear();
            mode = "FILL_RECURSIVE_FLOOD";
            break;
        }
        case ID_FILL_NONREC_FLOOD:
        {
            pts.clear();
            mode = "FILL_NONREC_FLOOD";
            break;
        }
        
        default: break;
        }
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        hdc = BeginPaint(hwnd, &ps);

        Ellipse(hdc, 100, 100, 500, 400);

        EndPaint(hwnd, &ps);

        break;
    }

		
	case WM_CLOSE: DestroyWindow(hwnd); break;
	case WM_DESTROY: PostQuitMessage(0); break;
	default: return DefWindowProc(hwnd, mcode, wp, lp);
	}
	return 0;
}

// ==================== MENU CREATION ====================
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


int APIENTRY WinMain(HINSTANCE h, HINSTANCE p, LPSTR c, int nsh)
{
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

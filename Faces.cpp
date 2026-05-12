#include <GLUT/glut.h>
#include <cmath>
#include <cstdio>
#include <vector>

const int WIN_W = 900;
const int WIN_H = 600;

int activeScene = 3;   

void putPixel(int x, int y)
{
    glVertex2i(x, y);
}

void plotEllipsePts(int cx, int cy, int x, int y)
{
    putPixel(cx + x, cy + y);
    putPixel(cx - x, cy + y);
    putPixel(cx + x, cy - y);
    putPixel(cx - x, cy - y);
}

void midpointEllipse(int cx, int cy, int a, int b)
{
    long long a2 = (long long)a * a;
    long long b2 = (long long)b * b;
    int x = 0, y = b;

    
    long long d1 = b2 - a2 * b + a2 / 4;
    long long dx = 2 * b2 * x;
    long long dy = 2 * a2 * y;

    glBegin(GL_POINTS);
    while (dx < dy)
    {
        plotEllipsePts(cx, cy, x, y);
        if (d1 < 0) { x++; dx += 2*b2; d1 += dx + b2; }
        else        { x++; y--; dx += 2*b2; dy -= 2*a2; d1 += dx - dy + b2; }
    }

    
    long long d2 = (long long)roundf(
        (float)b2 * (x + 0.5f) * (x + 0.5f)
      + (float)a2 * (y - 1)    * (y - 1)
      - (float)(a2 * b2));

    while (y >= 0)
    {
        plotEllipsePts(cx, cy, x, y);
        if (d2 > 0) { y--; dy -= 2*a2; d2 += a2 - dy; }
        else        { x++; y--; dx += 2*b2; dy -= 2*a2; d2 += dx - dy + a2; }
    }
    glEnd();
}

void midpointCircle(int cx, int cy, int r)
{
    midpointEllipse(cx, cy, r, r);
}

void filledCircle(int cx, int cy, int r)
{
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f((float)cx, (float)cy);
        for (int i = 0; i <= 36; i++)
        {
            float a = 2.0f * M_PI * i / 36.0f;
            glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
        }
    glEnd();
}

void hermiteBasis(float t, float& h1, float& h2, float& h3, float& h4)
{
    float t2 = t*t, t3 = t2*t;
    h1 =  2*t3 - 3*t2 + 1;
    h2 = -2*t3 + 3*t2;
    h3 =   t3 - 2*t2 + t;
    h4 =   t3 -  t2;
}

void cardinalSpline(const float px[], const float py[], int n,
                    float s, int subdiv)
{
    if (n < 2) return;
    glBegin(GL_LINE_STRIP);
    for (int seg = 0; seg < n - 1; seg++)
    {
        float p0x, p0y, p3x, p3y;
        float p1x = px[seg],   p1y = py[seg];
        float p2x = px[seg+1], p2y = py[seg+1];

        
        if (seg == 0)   { p0x = p1x-(p2x-p1x); p0y = p1y-(p2y-p1y); }
        else            { p0x = px[seg-1];       p0y = py[seg-1]; }
        if (seg == n-2) { p3x = p2x+(p2x-p1x); p3y = p2y+(p2y-p1y); }
        else            { p3x = px[seg+2];       p3y = py[seg+2]; }

        float sc = (1.0f - s) * 0.5f;
        float t1x = sc*(p2x-p0x), t1y = sc*(p2y-p0y);
        float t2x = sc*(p3x-p1x), t2y = sc*(p3y-p1y);

        for (int k = 0; k <= subdiv; k++)
        {
            float t = (float)k / subdiv;
            float h1, h2, h3, h4;
            hermiteBasis(t, h1, h2, h3, h4);
            glVertex2f(h1*p1x + h2*p2x + h3*t1x + h4*t2x,
                       h1*p1y + h2*p2y + h3*t1y + h4*t2y);
        }
    }
    glEnd();
}

void drawHead(int cx, int cy, int a, int b,
              float fr, float fg, float fb)
{
    
    glColor3f(fr, fg, fb);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f((float)cx, (float)cy);
        for (int i = 0; i <= 72; i++)
        {
            float ang = 2.0f * M_PI * i / 72.0f;
            glVertex2f(cx + a * cosf(ang), cy + b * sinf(ang));
        }
    glEnd();

    
    glColor3f(fr * 0.5f, fg * 0.5f, fb * 0.5f);
    glPointSize(2.0f);
    midpointEllipse(cx, cy, a, b);
}

void drawEye(int ex, int ey, int eyeR, int pupilR)
{
    
    glColor3f(1.0f, 1.0f, 1.0f);
    filledCircle(ex, ey, eyeR);

    
    glColor3f(0.15f, 0.15f, 0.15f);
    glPointSize(2.0f);
    midpointCircle(ex, ey, eyeR);

    
    glColor3f(0.1f, 0.1f, 0.1f);
    filledCircle(ex, ey, pupilR);
}

void drawEyebrow(int x0, int y0, int x1, int y1)
{
    glLineWidth(3.0f);
    glBegin(GL_LINES);
        glVertex2i(x0, y0);
        glVertex2i(x1, y1);
    glEnd();
}

void drawMouth(int cx, int cy, int hw, int depth, bool happy)
{
    
    float px[3] = { (float)(cx - hw), (float)cx,              (float)(cx + hw) };
    float py[3] = { (float)cy,        (float)(cy - depth),    (float)cy        };
    
    
    if (!happy)
        py[1] = (float)(cy + depth);

    glColor3f(0.15f, 0.05f, 0.05f);
    glLineWidth(3.0f);
    cardinalSpline(px, py, 3, 0.0f, 40);
}

void drawHappyFace(int cx, int cy)
{
    int headA = 140, headB = 160;

    
    drawHead(cx, cy, headA, headB, 1.0f, 0.85f, 0.2f);

    
    int eyeY = cy + 45;
    drawEye(cx - 50, eyeY, 22, 10);
    drawEye(cx + 50, eyeY, 22, 10);

    
    glColor3f(0.5f, 0.05f, 0.1f);
    drawMouth(cx, cy - 30, 70, 50, true);
}

void drawSadFace(int cx, int cy)
{
    int headA = 140, headB = 160;

    
    drawHead(cx, cy, headA, headB, 0.65f, 0.75f, 0.90f);

    
    int eyeY = cy + 35;
    drawEye(cx - 50, eyeY, 22, 10);
    drawEye(cx + 50, eyeY, 22, 10);

    
    
    glColor3f(0.2f, 0.2f, 0.35f);
    drawEyebrow(cx - 75, eyeY + 45, cx - 28, eyeY + 30);   
    drawEyebrow(cx + 28, eyeY + 30, cx + 75, eyeY + 45);   

    
    glColor3f(0.2f, 0.05f, 0.1f);
    drawMouth(cx, cy - 55, 65, 40, false);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    switch (activeScene)
    {
        case 1:
            drawHappyFace(WIN_W / 2, WIN_H / 2);
            break;

        case 2:
            drawSadFace(WIN_W / 2, WIN_H / 2);
            break;

        case 3:
            drawHappyFace(WIN_W / 4,     WIN_H / 2);
            drawSadFace(3 * WIN_W / 4,   WIN_H / 2);
            
            glColor3f(1.0f, 1.0f, 1.0f);
            glRasterPos2i(WIN_W/4 - 28, 30);
            {const char* t = "Happy :)"; for(const char* c=t;*c;c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);}
            glRasterPos2i(3*WIN_W/4 - 24, 30);
            {const char* t = "Sad :("; for(const char* c=t;*c;c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);}
            break;
    }

    
    glColor3f(0.6f, 0.6f, 0.6f);
    glRasterPos2i(10, WIN_H - 20);
    const char* hint = "Press 1=Happy  2=Sad  3=Both  ESC=quit";
    for (const char* c = hint; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

    
    glColor3f(0.5f, 0.5f, 0.5f);
    glRasterPos2i(10, 20);
    const char* legend = "Head: Midpoint Ellipse  |  Eyes: Midpoint Circle  |  Mouth: Cardinal Spline  |  Brows: Line";
    for (const char* c = legend; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int , int )
{
    if (key == 27) exit(0);
    if (key >= '1' && key <= '3') activeScene = key - '0';
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Smiley Faces — Student 3");

    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
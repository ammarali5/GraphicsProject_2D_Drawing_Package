#include <GLUT/glut.h>
#include <cmath>
#include <cstdio>

const int WIN_W = 800;
const int WIN_H = 600;

int activeAlgo = 1;

const int CX = WIN_W / 2;
const int CY = WIN_H / 2;
const int A  = 200;   
const int B  = 120;   

void putPixel(int x, int y)
{
    glBegin(GL_POINTS);
        glVertex2i(x, y);
    glEnd();
}

void drawCentreDot(int cx, int cy)
{
    glColor3f(1.0f, 0.3f, 0.3f);
    for (int dx = -3; dx <= 3; dx++)
        for (int dy = -3; dy <= 3; dy++)
            if (dx*dx + dy*dy <= 9)
                putPixel(cx + dx, cy + dy);
}

void drawDirectEllipse(int cx, int cy, int a, int b)
{
    float a2 = (float)(a * a);
    float b2 = (float)(b * b);

    
    glColor3f(0.2f, 0.6f, 1.0f);
    for (int x = -a; x <= a; x++)
    {
        float inner = 1.0f - (float)(x * x) / a2;
        if (inner < 0.0f) inner = 0.0f;        
        int y = (int)roundf(b * sqrtf(inner));
        putPixel(cx + x,  cy + y);             
        putPixel(cx + x,  cy - y);             
    }

    
    glColor3f(0.3f, 0.8f, 1.0f);
    for (int y = -b; y <= b; y++)
    {
        float inner = 1.0f - (float)(y * y) / b2;
        if (inner < 0.0f) inner = 0.0f;
        int x = (int)roundf(a * sqrtf(inner));
        putPixel(cx + x,  cy + y);             
        putPixel(cx - x,  cy + y);             
    }
}

void drawPolarEllipse(int cx, int cy, int a, int b)
{
    glColor3f(0.2f, 0.9f, 0.5f);

    
    float step = 1.0f / (float)(a > b ? a : b);

    for (float theta = 0.0f; theta < 2.0f * M_PI; theta += step)
    {
        int x = (int)roundf(cx + a * cosf(theta));
        int y = (int)roundf(cy + b * sinf(theta));
        putPixel(x, y);
    }
}

void plotEllipsePoints(int cx, int cy, int x, int y)
{
    
    putPixel(cx + x,  cy + y);
    putPixel(cx - x,  cy + y);
    putPixel(cx + x,  cy - y);
    putPixel(cx - x,  cy - y);
}

void drawMidpointEllipse(int cx, int cy, int a, int b)
{
    glColor3f(1.0f, 0.75f, 0.1f);

    long long a2 = (long long)a * a;
    long long b2 = (long long)b * b;

    int x = 0;
    int y = b;

    
    
    
    
    long long d1 = b2 - a2 * b + a2 / 4;

    
    long long dx = 2 * b2 * x;   
    long long dy = 2 * a2 * y;   

    while (dx < dy)                  
    {
        plotEllipsePoints(cx, cy, x, y);

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

    
    
    
    long long d2 = (long long)(b2 * (x + 0) * (x + 0))
                 + (long long)(a2 * (y - 1) * (y - 1))
                 - (long long)(a2 * b2);
    
    d2 = (long long)roundf((float)(b2) * (x + 0.5f) * (x + 0.5f)
                         + (float)(a2) * (y - 1) * (y - 1)
                         - (float)(a2 * b2));

    while (y >= 0)                   
    {
        plotEllipsePoints(cx, cy, x, y);

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

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    
    glColor3f(0.25f, 0.25f, 0.25f);
    glBegin(GL_LINES);
        glVertex2i(0, CY);     glVertex2i(WIN_W, CY);   
        glVertex2i(CX, 0);     glVertex2i(CX, WIN_H);   
    glEnd();

    glPointSize(2.0f);

    switch (activeAlgo)
    {
        case 1:
            drawDirectEllipse(CX, CY, A, B);
            break;
        case 2:
            drawPolarEllipse(CX, CY, A, B);
            break;
        case 3:
            drawMidpointEllipse(CX, CY, A, B);
            break;
    }

    drawCentreDot(CX, CY);

    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(10, WIN_H - 20);
    const char* names[] = { "", "1 - Direct Ellipse", "2 - Polar Ellipse", "3 - Midpoint Ellipse" };
    for (const char* c = names[activeAlgo]; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    glRasterPos2i(10, 20);
    const char* hint = "Press 1 / 2 / 3 to switch algorithm   ESC to quit";
    for (const char* c = hint; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int , int )
{
    if (key == 27) exit(0);                
    if (key >= '1' && key <= '3')
        activeAlgo = key - '0';
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Ellipse Algorithms — Student 3");

    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}

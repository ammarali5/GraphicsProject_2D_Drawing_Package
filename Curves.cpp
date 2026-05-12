#include <GLUT/glut.h>
#include <cmath>
#include <cstdio>
#include <vector>

const int WIN_W = 800;
const int WIN_H = 600;

std::vector<float> ctrlX = { 80, 200, 320, 440, 560, 680, 750 };
std::vector<float> ctrlY = { 300, 150, 430, 120, 400, 200, 300 };

float tension = 0.5f;          
int   steps   = 50;            

int   dragIdx = -1;

void putPixel(float x, float y)
{
    glVertex2f(x, y);
}

void hermiteBasis(float t, float& h1, float& h2, float& h3, float& h4)
{
    float t2 = t * t;
    float t3 = t2 * t;
    h1 =  2*t3 - 3*t2 + 1;
    h2 = -2*t3 + 3*t2;
    h3 =   t3 - 2*t2 + t;
    h4 =   t3 -  t2;
}

void drawCardinalSpline(const std::vector<float>& px,
                        const std::vector<float>& py,
                        float s, int subdiv)
{
    int n = (int)px.size();
    if (n < 2) return;

    glBegin(GL_LINE_STRIP);

    for (int seg = 0; seg < n - 1; seg++)
    {
        
        
        
        
        

        float p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y;

        p1x = px[seg];       p1y = py[seg];
        p2x = px[seg + 1];   p2y = py[seg + 1];

        
        if (seg == 0)
        {
            p0x = p1x - (p2x - p1x);
            p0y = p1y - (p2y - p1y);
        }
        else
        {
            p0x = px[seg - 1];
            p0y = py[seg - 1];
        }

        
        if (seg == n - 2)
        {
            p3x = p2x + (p2x - p1x);
            p3y = p2y + (p2y - p1y);
        }
        else
        {
            p3x = px[seg + 2];
            p3y = py[seg + 2];
        }

        
        
        
        float scale = (1.0f - s) * 0.5f;
        float t1x = scale * (p2x - p0x);
        float t1y = scale * (p2y - p0y);
        float t2x = scale * (p3x - p1x);
        float t2y = scale * (p3y - p1y);

        
        for (int k = 0; k <= subdiv; k++)
        {
            float t = (float)k / (float)subdiv;
            float h1, h2, h3, h4;
            hermiteBasis(t, h1, h2, h3, h4);

            float x = h1*p1x + h2*p2x + h3*t1x + h4*t2x;
            float y = h1*p1y + h2*p2y + h3*t1y + h4*t2y;
            putPixel(x, y);
        }
    }

    glEnd();
}

void drawControlPoint(float cx, float cy, bool highlighted)
{
    int segs = 16;
    float r = highlighted ? 9.0f : 7.0f;
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= segs; i++)
        {
            float a = 2.0f * M_PI * i / segs;
            glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
        }
    glEnd();
    
    glBegin(GL_LINE_LOOP);
        for (int i = 0; i <= segs; i++)
        {
            float a = 2.0f * M_PI * i / segs;
            glVertex2f(cx + r * cosf(a), cy + r * sinf(a));
        }
    glEnd();
}

void drawTangents()
{
    int n = (int)ctrlX.size();
    glColor3f(1.0f, 0.6f, 0.0f);
    glLineStipple(2, 0xAAAA);
    glEnable(GL_LINE_STIPPLE);
    glLineWidth(1.0f);

    for (int i = 1; i < n - 1; i++)
    {
        float scale = (1.0f - tension) * 0.5f;
        float tx = scale * (ctrlX[i+1] - ctrlX[i-1]);
        float ty = scale * (ctrlY[i+1] - ctrlY[i-1]);
        
        glBegin(GL_LINES);
            glVertex2f(ctrlX[i] - tx * 0.5f, ctrlY[i] - ty * 0.5f);
            glVertex2f(ctrlX[i] + tx * 0.5f, ctrlY[i] + ty * 0.5f);
        glEnd();
    }
    glDisable(GL_LINE_STIPPLE);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    
    drawTangents();

    
    glColor3f(0.3f, 0.5f, 1.0f);
    glLineWidth(2.5f);
    drawCardinalSpline(ctrlX, ctrlY, tension, steps);

    
    glColor3f(0.4f, 0.4f, 0.4f);
    glLineWidth(1.0f);
    glLineStipple(1, 0x3333);
    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_STRIP);
        for (int i = 0; i < (int)ctrlX.size(); i++)
            glVertex2f(ctrlX[i], ctrlY[i]);
    glEnd();
    glDisable(GL_LINE_STIPPLE);

    
    for (int i = 0; i < (int)ctrlX.size(); i++)
    {
        bool hi = (i == dragIdx);
        glColor3f(hi ? 1.0f : 0.1f, hi ? 0.4f : 0.85f, hi ? 0.2f : 0.35f);
        drawControlPoint(ctrlX[i], ctrlY[i], hi);
        glColor3f(0.0f, 0.2f, 0.1f);
        
        glRasterPos2f(ctrlX[i] + 10, ctrlY[i] + 10);
        char buf[8]; snprintf(buf, sizeof(buf), "P%d", i+1);
        for (char* c = buf; *c; c++)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2i(10, WIN_H - 22);
    char buf[80];
    snprintf(buf, sizeof(buf), "Cardinal Spline   tension=%.2f   pts=%d",
             tension, (int)ctrlX.size());
    for (char* c = buf; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    glColor3f(0.7f, 0.7f, 0.7f);
    glRasterPos2i(10, 20);
    const char* hint = "+/- tension   drag points   click to add   R=reset   ESC=quit";
    for (const char* c = hint; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);

    glutSwapBuffers();
}

int findNearestPoint(int mx, int my, float radius = 12.0f)
{
    
    float gy = WIN_H - my;
    for (int i = 0; i < (int)ctrlX.size(); i++)
    {
        float dx = ctrlX[i] - mx;
        float dy = ctrlY[i] - gy;
        if (sqrtf(dx*dx + dy*dy) < radius) return i;
    }
    return -1;
}

void mouseButton(int button, int state, int x, int y)
{
    float gy = WIN_H - y;
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            dragIdx = findNearestPoint(x, y);
            if (dragIdx == -1)
            {
                
                ctrlX.push_back((float)x);
                ctrlY.push_back(gy);
                dragIdx = (int)ctrlX.size() - 1;
            }
        }
        else
        {
            dragIdx = -1;
        }
    }
    glutPostRedisplay();
}

void mouseMotion(int x, int y)
{
    if (dragIdx >= 0)
    {
        ctrlX[dragIdx] = (float)x;
        ctrlY[dragIdx] = (float)(WIN_H - y);
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int , int )
{
    switch (key)
    {
        case 27:  exit(0); break;
        case '+': case '=': tension = fminf(tension + 0.05f, 1.0f); break;
        case '-': case '_': tension = fmaxf(tension - 0.05f, 0.0f); break;
        case 'r': case 'R':
            ctrlX = { 80, 200, 320, 440, 560, 680, 750 };
            ctrlY = { 300, 150, 430, 120, 400, 200, 300 };
            tension = 0.5f;
            break;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Cardinal Spline — Student 3");

    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutMainLoop();
    return 0;
}
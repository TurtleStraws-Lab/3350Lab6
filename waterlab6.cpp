//
//modified by:
//date:
//
//original author: Gordon Griesel
//date:            2025
//purpose:         OpenGL sample program
//
//This program needs some refactoring.
//We will do this in class together.
//
//to do list
//1/31/2025
//
//
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cmath>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#include "fonts.h"

//macro 
#define rnd() (float)rand() / (float)RAND_MAX
#define MAX_PARTICLES 2000
//some structures

float randomFloat() {
    return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

int e = 0;
class Box {
    public:
        float tfs = 256;
        int width;
        int height;
        float pos[2];
        float pos2[2];
        float pos3[2];
        float pos4[2];
        float pos5[2];
        float prev[2];
        float prev2[2];
        float vel[2];
        float force[2];
        float color[3];
        char text[100];
        Box() 
        {
            width = 50;
            height = 25;
            vel[0] = vel[1] = 0.0f;
            
            color[0] = randomFloat();
            color[1] = randomFloat();
            color[2] = randomFloat();
         
            }
        Box(int w, int h){
            Box();
            width = w;
            height = h;

        }
} box, particle(4,4);

Box particles[MAX_PARTICLES];
int n = 0;
void make_particle(int x, int y) {
    if (n >= MAX_PARTICLES)
        return;
    particles[n].width = 4;
    particles[n].height = 4;
    particles[n].pos[0] = x;
    particles[n].pos[1] = y;
    particles[n].vel[1] = rnd() * 0.6f - 0.2;
    particles[n].vel[0] = rnd() * 0.6f - 0.2;
    ++n;

}

void del_particle(int a) {
    if (n == 0)
        return;
    //particles[a] = particles[n-1];
    //n = n - 1;
    //--n;
    //--n;
    particles[a] = particles[--n];

}
class Global {
    public:
        int xres, yres;
    //--n;
        //float w;
        //float dir, dir2;
        //float pos[2];
        int a, b, c;
        int count;
        Global() {
            xres = 640;
            yres = 480;
            // w = 25.0f;
            //dir = 20.0f;
            //dir2 = 10.0f;
            //pos[0] = 0.0f+w;
            //pos[1] = yres/2;
            a = 0; 
            b = 0;
            c = 0;
            count = 0;
        }
    //--n;
} g;

class X11_wrapper {
    private:
        Display *dpy;
        Window win;
        GLXContext glc;
    public:
        ~X11_wrapper();
        X11_wrapper();
        void set_title();
        bool getXPending();
        XEvent getXNextEvent();
        void swapBuffers();
        void reshape_window(int width, int height);
        void check_resize(XEvent *e);
        void check_mouse(XEvent *e);
        int check_keys(XEvent *e);
} x11;

//Function prototypes
void init_opengl(void);
void physics(void);
void render(void);


int main()
{
    srand(time(NULL));
    init_opengl();
    void init_box();
    init_box();
    int done = 0;

    //main game loop
    while (!done) {
        //look for external events such as keyboard, mouse.
        while (x11.getXPending()) {

            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            x11.check_mouse(&e);
            done = x11.check_keys(&e);
        }

        physics();
        render();
        x11.swapBuffers();
        usleep(200);
    }
    cleanup_fonts();
    return 0;
}

void init_box()
{
    // Y is height X is Width
    box.pos[0] = (g.xres / 2);
    box.pos[1] = (g.yres / 2) - 30;
    box.pos2[0] = (g.xres / 2) - 95;
    box.pos2[1] = (g.yres / 2) + 45;

    box.pos3[0] = (g.xres / 2) - 155;
    box.pos3[1] = (g.yres / 2) + 105;

    box.pos4[0] = (g.xres / 2) + 95;
    box.pos4[1] = (g.yres / 2) - 105;

    box.pos5[0] = (g.xres / 2) + 155;
    box.pos5[1] = (g.yres / 2) - 165;
}

//Global::Global()
//{
//}

X11_wrapper::~X11_wrapper()
{
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None }; //24 OG
    int w = g.xres, h = g.yres;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        cout << "\n\tcannot connect to X server\n" << endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if (vi == NULL) {
        cout << "\n\tno appropriate visual found\n" << endl;
        exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask =
        ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
            InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "3350 Lab-6, Esc to exit");
}

bool X11_wrapper::getXPending()
{
    //See if there are pending events.
    return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
    //Get a pending event.
    XEvent e;
    XNextEvent(dpy, &e);
    return e;
}

void X11_wrapper::swapBuffers()
{
    glXSwapBuffers(dpy, win);
}

void X11_wrapper::reshape_window(int width, int height)
{
    //Window has been resized.
    g.xres = width;
    g.yres = height;
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glOrtho(0, g.xres, 0, g.yres, -1, 1);

}

void X11_wrapper::check_resize(XEvent *e)
{

    //The ConfigureNotify is sent by the
    //server if the window is resized.
    if (e->type != ConfigureNotify){
        return;
    }
    XConfigureEvent xce = e->xconfigure;
    if (xce.width != g.xres || xce.height != g.yres) {
        //Window size did change.
        reshape_window(xce.width, xce.height);
    }
}
//-----------------------------------------------------------------------------

void X11_wrapper::check_mouse(XEvent *e)
{
    static int savex = 0;
    static int savey = 0;

    //Weed out non-mouse events
    if (e->type != ButtonRelease &&
            e->type != ButtonPress &&
            e->type != MotionNotify) {
        //This is not a mouse event that we care about.
        return;
    }
    //
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button was pressed.
            int y = g.yres - e->xbutton.y;
            //particle.pos[0] = e->xbutton.x;
            //particle.pos[1] = y;i
            for (int i = 0; i < 5; i++)
                make_particle(e->xbutton.x, y);
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed.
            return;
        }
    }
    if (e->type == MotionNotify) {
        //The mouse moved!
        if (savex != e->xbutton.x || savey != e->xbutton.y) {
            savex = e->xbutton.x;
            savey = e->xbutton.y;
            //Code placed here will execute whenever the mouse moves.

            for (int i = 0; i < 2; i++){
                make_particle(savex,g.yres-savey);
            }
            }

    }
}

int X11_wrapper::check_keys(XEvent *e)
{
    if (e->type != KeyPress && e->type != KeyRelease)
        return 0;
    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyPress) {
        switch (key) {
            case XK_a:
                //g.dir = (g.dir/16) + g.dir;
                //the 'a' key was pressed
                break;
            case XK_b:
                //  g.dir = (-g.dir/16) + g.dir;
                break;
            case XK_Escape:
                //Escape key was pressed
                return 1;
        }
    }
    return 0;
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);


    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
}

const float GRAVITY = -0.01; 

void physics()
{
    for (int i = 0; i<n; i++) {
        Box *p = &particles[i];
        p->prev[0] = p->pos[0];
        p->prev[1] = p->pos[1];
        // move the particle...
        p->force[1] =  GRAVITY;
        p->vel[1] += p->force[1];
        p->vel[0] += p->force[0];
        p->pos[1] += p->vel[1];
        p->pos[0] += p->vel[0]; //+ 0.2f
        p->force[1] = 0.0f;
        p->force[0] = 0.0f;
        // Collision detection
        if (p->pos[1] <= (box.pos[1] + box.height ) &&
                p->pos[0] >= (box.pos[0] - box.width ) &&
                p->pos[0] <= (box.pos[0] + box.width ) &&
                p->pos[1] >= (box.pos[1] - box.height )) {
            p->prev[0] = p->pos[0];
            p->prev[1] = p->pos[1];
            p->vel[1] = -p->vel[1] * 0.8;
            p->vel[0] = -p->vel[0] + 0.2f;

        }

        if (p->pos[1] <= (box.pos2[1] + box.height ) &&
                p->pos[0] >= (box.pos2[0] - box.width ) &&
                p->pos[0] <= (box.pos2[0] + box.width ) &&
                p->pos[1] >= (box.pos2[1] - box.height )) {
            p->prev[0] = p->pos[0];
            p->prev[1] = p->pos[1];
            p->vel[1] = -p->vel[1] * 0.8;
            p->vel[0] = -p->vel[0] + 0.2f;
          }
        if (p->pos[1] <= (box.pos3[1] + box.height ) &&
                p->pos[0] >= (box.pos3[0] - box.width ) &&
                p->pos[0] <= (box.pos3[0] + box.width ) &&
                p->pos[1] >= (box.pos3[1] - box.height )) {
            p->prev[0] = p->pos[0];
            p->prev[1] = p->pos[1];
            p->vel[1] = -p->vel[1] * 0.8;
            p->vel[0] = -p->vel[0] + 0.2f;
        }
        if (p->pos[1] <= (box.pos4[1] + box.height ) &&
                p->pos[0] >= (box.pos4[0] - box.width ) &&
                p->pos[0] <= (box.pos4[0] + box.width ) &&
                p->pos[1] >= (box.pos4[1] - box.height )) {
        p->prev[0] = p->pos[0];
        p->prev[1] = p->pos[1];
        p->vel[1] = -p->vel[1] * 0.8;
            p->vel[0] = -p->vel[0] + 0.2f;
        }
            
        if (p->pos[1] <= (box.pos5[1] + box.height ) &&
                p->pos[0] >= (box.pos5[0] - box.width ) &&
                p->pos[0] <= (box.pos5[0] + box.width ) &&
                p->pos[1] >= (box.pos5[1] - box.height )) {
            p->prev[0] = p->pos[0];
            p->prev[1] = p->pos[1];
            p->vel[1] = -p->vel[1] * 0.8; // slows down bounce
            p->vel[0] = -p->vel[0] + 0.2f;

        }
        if (p->pos[1] < -4.0f)
            del_particle(i);
    }
}



void render()
{
    glClear(GL_COLOR_BUFFER_BIT); // clear the window
                                  //draw the box

    glPushMatrix();
    glColor3fv(box.color);
    e++;
    glTranslatef(box.pos[0], box.pos[1], 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-box.width, -box.height);// first two negative first of second negative
    glVertex2f(-box.width,  box.height);
    glVertex2f( box.width,  box.height);
    glVertex2f( box.width, -box.height);  //if negative removed makes triangle
    glEnd();
    glPopMatrix();
    // Second Recntagle
    glPushMatrix();
    glColor3fv(box.color);
    glTranslatef(box.pos2[0], box.pos2[1], 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-box.width, -box.height);// first two negative first of second negative
    glVertex2f(-box.width,  box.height);
    glVertex2f( box.width,  box.height);
    glVertex2f( box.width, -box.height);  //if negative removed makes triangle
    glEnd();
    glPopMatrix();
    // Third Rectangle
    glPushMatrix();
    glColor3fv(box.color);
    e++;
    glTranslatef(box.pos3[0], box.pos3[1], 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-box.width, -box.height);// first two negative first of second negative
    glVertex2f(-box.width,  box.height);
    glVertex2f( box.width,  box.height);
    glVertex2f( box.width, -box.height);  //if negative removed makes triangle
    glEnd();
    glPopMatrix();
    // Fourth Rectangle
    glPushMatrix();
    glColor3fv(box.color);
    e++;
    glTranslatef(box.pos4[0], box.pos4[1], 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-box.width, -box.height);// first two negative first of second negative
    glVertex2f(-box.width,  box.height);
    glVertex2f( box.width,  box.height);
    glVertex2f( box.width, -box.height);  //if negative removed makes triangle
    glEnd();
    glPopMatrix();
    // Fifth Rectangle
    glPushMatrix();
    glColor3fv(box.color);
    e++;
    glTranslatef(box.pos5[0], box.pos5[1], 0.0f);
    glBegin(GL_QUADS);
    glVertex2f(-box.width, -box.height);// first two negative first of second negative
    glVertex2f(-box.width,  box.height);
    glVertex2f( box.width,  box.height);
    glVertex2f( box.width, -box.height);  //if negative removed makes triangle
    glEnd();
    glPopMatrix();
    // draw particles
    for (int i = 0; i<n; i++){ 
        glPushMatrix();
    glColor3ub(50,120,220);
    glTranslatef(particles[i].pos[0], particles[i].pos[1], 0.0f);
    Box *p = &particle;
    glBegin(GL_QUADS);
    glVertex2f(-p->width, -p->height);// first two negative first of second negative
    glVertex2f(-p->width,  p->height);
    glVertex2f( p->width,  p->height);
    glVertex2f( p->width, -p->height);  //if negative removed makes triangle
    glEnd();
    glPopMatrix();
    }

      Rect r,x,a,b,c,d;
      r.bot = g.yres - 20;
      r.left = 10;
      r.center = 0;
      // all heights were dropped by 30
      x.bot = g.yres - 140;
      x.left = 125;
      x.center = 0;

      a.bot = g.yres - 200;
      a.left = 205;
      a.center = 0;
      
      b.bot = g.yres - 275;
      b.left = 285;
      b.center = 0;
      
      c.bot = g.yres - 350;
      c.left = 373;
      c.center = 0;

      d.bot = g.yres - 410;
      d.left = 445;
      d.center = 0;
      ggprint8b(&r, 16, 0x00ff0000, "3350 - Waterfall Model");
      ggprint8b(&r, 16, 0x00ff0000, "Move Mouse to spawn water");
      ggprint8b(&r, 16, 0x00ff0000, "Click mouse to spawn water");
      ggprint8b(&x, 16, 0x00000000, "Requirements");
      ggprint8b(&a, 16, 0x00000000, "Design");
      ggprint8b(&b, 16, 0x00000000, "Development");
      ggprint8b(&c, 16, 0x00000000, "Implementation");
      ggprint8b(&d, 16, 0x00000000, "Matainence");


}





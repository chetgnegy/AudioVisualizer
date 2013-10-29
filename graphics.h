/*
 * graphics.cpp
 *
 *  Created on: Oct 18, 2013
 *      Author: chetgnegy
 */
#ifdef __MACOSX_CORE__
#include <GLUT/glut.h>

#else
//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/glut.h>
#endif

#include <iostream>
#include <math.h>
#include "bubble.h"
#include "graphicsutil.h"
#include "AudioFeatures.h"

using namespace std;

typedef int BOOL;
#define TRUE 1
#define FALSE 0

// initial window size (in pixels)
GLsizei g_width = 800;
GLsizei g_height = 600;

//-----------------------------------------------------------------------------
// Defines a point in a 3D space (coords x, y and z)
//-----------------------------------------------------------------------------
struct pt3d {
  pt3d(GLfloat _x, GLfloat _y, GLfloat _z)
      : x(_x),
        y(_y),
        z(_z) {
  }
  ;

  float x;
  float y;
  float z;
};

// =======================
// = Function prototypes =
// =======================
void idleFunc();
void displayFunc();
void reshapeFunc(int width, int height);
void keyboardFunc(unsigned char, int, int);
void mouseFunc(int button, int state, int x, int y);
void specialFunc(int key, int x, int y);
void initialize();
void changeLookAt(pt3d look_from, pt3d look_to, pt3d head_up);

// Camera global variables
pt3d g_look_from(0, 0, 1);
pt3d g_look_to(0, 0, 0);
pt3d g_head_up(0, 1, 0);

// All of the features
AudioFeatures *feat;
// All of the bubbles
std::list<pair<Bubble *, int> *> bubbles;
// The sound buffer for the visuals
double *sound;
// The length of the sound buffer
int sound_length;

void setSoundBuffer(double *buf, int len) {
  sound = buf;
  sound_length = len;
}

int setGraphicsFeatures(AudioFeatures *f) {
  feat = f;
}

// Entry point
int startGraphics(int argc, char *argv[]) {
  // initialize GLUT
  glutInit(&argc, argv);
  // initialize the window size
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA);

  glutInitWindowSize(g_width, g_height);
  // set the window postion
  //glutInitWindowPosition(350, 350);
  // create the window
  glutCreateWindow("Smellovision");
  //glutEnterGameMode();

  // set the idle function - called when idle
  glutIdleFunc(idleFunc);
  // set the display function - called when redrawing
  glutDisplayFunc(displayFunc);
  // set the reshape function - called when client area changes
  glutReshapeFunc(reshapeFunc);
  // set the keyboard function - called on keyboard events
  glutKeyboardFunc(keyboardFunc);
  // set the mouse function - called on mouse stuff
  glutMouseFunc(mouseFunc);
  // set the special function - called on special keys events (fn, arrows, pgDown, etc)
  glutSpecialFunc(specialFunc);
  return 0;
}

// ============
// = GL Stuff =
// ============

//-----------------------------------------------------------------------------
// Name: initialize( )
// Desc: sets initial OpenGL states
//       also initializes any application data
//-----------------------------------------------------------------------------
void glInitialize() {
  // seed random number generator
  srand(time(NULL));
  // set the front faces of polygons
  // set fill mode
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Enable transparency
  glEnable (GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 0.0 };  //color of shiny part
  GLfloat mat_shininess[] = { 10.0 };  //shininess of the objects
  GLfloat light_position[] = { 2.0, 4.0, 3.0, 0.0 };  //moves the lighting
  glClearColor(0.0, 0.0, 0.0, 1.0);  //sets the background color
  glShadeModel (GL_SMOOTH);

  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  glEnable (GL_COLOR_MATERIAL);
  glEnable (GL_LIGHT0);
  glEnable (GL_DEPTH_TEST);
}

//-----------------------------------------------------------------------------
// Name: reshapeFunc( )
// Desc: called when window size changes
//-----------------------------------------------------------------------------
void reshapeFunc(int x, int y) {
  // save the new window size
  if (y == 0 || x == 0)
    return;
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(39.0, (GLdouble) x / (GLdouble) y, 0.6, 21.0);
  glMatrixMode (GL_MODELVIEW);
  glViewport(0, 0, x, y);
}

//-----------------------------------------------------------------------------
// Name: idleFunc( )
// Desc: callback from GLUT
//-----------------------------------------------------------------------------
void idleFunc() {
  // render the scene
  glutPostRedisplay();
}

void addBubble(int note) {
  Bubble *b = new Bubble(0, 0, 0, .5 * (rand() / (1.0 * RAND_MAX) + .2));
  std::pair<Bubble*, int> *k = new std::pair<Bubble*, int>(
      new Bubble(0, 0, 0, .5 * (rand() / (1.0 * RAND_MAX) + .2)), note);
  bubbles.push_back(k);
}

void popAll() {
  std::list<std::pair<Bubble *, int> *>::iterator b2;
  b2 = bubbles.begin();
  //Pops bubbles that are close together
  while (b2 != bubbles.end()) {
    if (rand() / (1.0 * RAND_MAX) > .6) {
      (*b2)->first->pop();
    }
    ++b2;
  }
}

void keysShift(int note, double key_width, double *&arr) {
  double offsetX = 0;
  double offsetZ = 0;
  if (note < 5) {  //C to E
    offsetX = note / 2.0;
    if (note % 2 == 1)  //C# and D#
      offsetZ = -1;
  } else {
    offsetX = (note - 5) / 2.0 + 3;
    if (note % 2 == 0)
      offsetZ = -1;

  }
  arr[0] = offsetX * key_width;
  arr[1] = 0.0;
  arr[2] = offsetZ;
}

//-----------------------------------------------------------------------------
// Name: displayFunc( )
// Desc: callback function invoked to draw the client area
//-----------------------------------------------------------------------------
void displayFunc() {
  // clear the color and depth buffers
  glMatrixMode (GL_MODELVIEW);
  // clear the drawing buffer.

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable (GL_LIGHTING);

  // clear the identity matrix.
  glLoadIdentity();
  glTranslatef(0.0, 0.0, -9.0);

  float colorBronzeDiff[4] = { 0.8, 0.6, 0.0, 1.0 };
  float colorBronzeSpec[4] = { 1.0, 1.0, 0.4, 1.0 };
  float colorWhite[4] = { 0.99, 0.90, 0.99, 1.0 };
  float colorBlack[4] = { 0.09, 0.09, 0.09, 1.0 };
  float colorNone[4] = { 0.0, 0.0, 0.0, 0.0 };
  glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
  glMaterialfv(GL_FRONT, GL_SPECULAR, colorNone);

  glLightModeli(GL_LIGHT_MODEL_AMBIENT, GL_TRUE);

  double key_width = 1.02;

  glPushMatrix();
  //move down to keyboard
  glTranslatef(-3.75, -3.0, 0.0);

  glColor4fv(colorWhite);
  glBindTexture(GL_TEXTURE_2D, 1);
  glPushMatrix();
  //Tilt Keys
  glRotatef(10, 1.0, 0.0, 0.0);
  glPushMatrix();
  //white keys
  glTranslatef(-1.5, 0.0, 0.0);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, colorNone);
  for (int i = 0; i < 11; ++i) {
    glPushMatrix();
    //Draws a single white key
    for (int k = 0; k < 3; ++k) {
      DrawCubeWithTextureCoords(1.0);
      glTranslatef(0.0, 0.0, -1.0);
    }
    glPopMatrix();
    glTranslatef(key_width, 0.0, 0.0);
  }
  glPopMatrix();
  glPushMatrix();
  //black keys
  glTranslatef(-2.0, 0.4, -0.9);
  glColor4fv(colorBlack);
  for (int i = -3; i < 8; ++i) {
    if (i != -1 && i != 2 && i != 6) {
      glPushMatrix();
      //Draws a single black key
      for (int k = 0; k < 3; ++k) {
        DrawCubeWithTextureCoords(.7);
        glTranslatef(0.0, 0.0, -.68);
      }
      glPopMatrix();
    }
    glTranslatef(key_width, 0.0, 0.0);
  }
  glPopMatrix();
  glPopMatrix();

  glTranslatef(0.5, 0.0, 0.0);

  //Draw Bubbles
  glPushAttrib (GL_ALL_ATTRIB_BITS);
  glLightModeli(GL_LIGHT_MODEL_AMBIENT, GL_TRUE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, colorWhite);
  glMaterialfv(GL_FRONT, GL_SPECULAR, colorWhite);

  glColor4f(1.0, 1.0, 1.0, .35);
  if (bubbles.size() > 0) {
    std::list<std::pair<Bubble *, int> *>::iterator b1;
    std::list<std::pair<Bubble *, int> *>::iterator b2;
    b1 = bubbles.begin();

    while (b1 != bubbles.end()) {
      glPushMatrix();
      double *a = new double[3];
      keysShift((*b1)->second, key_width, a);
      glTranslatef(a[0], a[1], a[2]);  //move to key position
      (*b1)->first->getPosition(a);
      glTranslatef(a[0], a[1], a[2]);  //move to bubble position
      delete[] a;
      if (!(*b1)->first->is_popped())
        glScalef((*b1)->first->sx_, (*b1)->first->sy_, (*b1)->first->sz_);
      glutSolidSphere((*b1)->first->radius_, 20, 20);
      (*b1)->first->ascend();

      b2 = bubbles.begin();

      //Pops bubbles that are close together
      while (b2 != bubbles.end()) {
        if (b2 != b1) {
          if ((*b1)->first->distance_to(((*b2)->first)) < 1) {
            if (rand() / (1.0 * RAND_MAX) < .000005) {
              (*b2)->first->pop();
              (*b1)->first->pop();
            }
          }
        }
        ++b2;
      }
      glPopMatrix();
      ++b1;

    }

    //Deletes popped bubbles
    b1 = bubbles.begin();
    while (bubbles.size() > 0 && b1 != bubbles.end()) {
      if ((*b1)->first->is_popped()) {
        glPushMatrix();
        //Show bubble popping
        double *a = new double[3];
        keysShift((*b1)->second, key_width, a);
        glTranslatef(a[0], a[1], a[2]);  //move to key position
        (*b1)->first->getPosition(a);
        glTranslatef(a[0], a[1], a[2]);  //move to bubble position
        delete[] a;

        glColor4f(1.0, 1.0, 1.0, .20);
        glutSolidSphere(0.75 * (*b1)->first->radius_, 20, 20);
        glColor4f(1.0, 1.0, 1.0, .15);
        glutSolidSphere(1.5 * (*b1)->first->radius_, 20, 20);
        glColor4f(1.0, 1.0, 1.0, .8);
        glutSolidSphere(1.24 * (*b1)->first->radius_, 20, 20);
        glPopMatrix();

        delete (*b1)->first;
        b1 = bubbles.erase(b1);
      } else {
        ++b1;
      }

      //std::cout << "moveto " << (*b1)->first<< std::endl;

    }

  }

  glPopMatrix();

  glPopAttrib();

  glDisable(GL_LIGHTING);

  glPointSize(1.5);

  // find the x increment
  int wrap = 10;
  // DISPLAYS TIME DOMAIN
  if (sound_length > 0) {
    double val;
    glPushMatrix();
    glTranslatef(0.0, 2.0, 0.0);
    glRotatef(40, 1.0, 0.0, 0.0);  //tilt forward

    for (int i = 0; i < sound_length - wrap; ++i) {
      val = sound[i];

      //Crossfade
      if (i < wrap) {
        val = i / (1.0 * wrap) * sound[i]
            + (1 - i / (1.0 * wrap)) * sound[sound_length - wrap + i];
      }
      //Sexy color scheme
      double R, G, B;
      spectrum(8.0 * pow(fabs(val - sound[i - 1]), .8), R, G, B);
      glColor3f(R, G, B);

      glRotatef(360.0 / (sound_length - wrap), 0.0, 1.0, 0.0);  //rotates radius vector
      glPushMatrix();
      glTranslatef(2.0, 0.0, 0.0);  //the radius vector

      glBegin (GL_POINTS);
      glVertex3f(0, 1 * val, 0);
      glEnd();

      glPopMatrix();
    }

    glPopMatrix();
  }

  GLfloat xinc = .02;
  double distance = -10;
  glPushMatrix();
  glTranslatef(0, -3.2, -8);
  glScalef(3, 3, 3);
  static double power[100] = { 0 };
  static double specpower[100] = { 0 };
  static int thisFrame = 0;
  static int inc = 0;

  glLineWidth(5);

  for (int i = 0; i < 100; ++i) {
    glBegin (GL_LINES);
    glColor3f(.1, 0, .2);
    glVertex3f(4 * xinc * (i - 50 - 1 * (inc % 100) / 100.),
               power[(thisFrame + i) % 100] / 2000.0, 0);
    glVertex3f(4 * xinc * (i - 50 - 1 * (inc % 100) / 100.), 0, 0);
    glEnd();
    glBegin(GL_LINES);
    glColor3f(.25, .1, 0);
    glVertex3f(4 * xinc * (i + .5 - 50 - 1 * (inc % 100) / 100.),
               specpower[(thisFrame + i) % 100] / 2000.0, 0);
    glVertex3f(4 * xinc * (i + .5 - 50 - 1 * (inc % 100) / 100.), 0, 0);
    glEnd();
  }
  if (inc % 100 == 0) {
    power[thisFrame] = feat->NFoFT_power();
    specpower[thisFrame] = feat->spectral_power();
    thisFrame = (1 + thisFrame) % 100;
  }
  inc = (1 + inc) % 100;
  glPopMatrix();

  if (feat->valid_FoFT_) {
    double K = 1.2;

    //DISPLAYS FoFT
    int FoFT_buf_size = feat->get_buffer_size() / 4;
    for (int k = 0; k < feat->depth_; ++k) {
      for (int i = -FoFT_buf_size + 1; i < FoFT_buf_size; ++i) {
        glBegin (GL_POINTS);
        //Sexy color scheme
        double R, G, B;
        double factor = k / (1.0 * feat->depth_);
        spectrum(k / (1.0 * feat->depth_ - 1), R, G, B);
        glColor3f(factor * R, factor * G, factor * B);

        glVertex3f(.4 * xinc * i,
                   -1.5 + K * feat->FoFT_history_[(k) * FoFT_buf_size + abs(i)],
                   distance + k);

        glEnd();
      }
    }

    glPointSize(3.0);
    glColor3f(1, 1, 1);
    //DISPLAYS FoFT
    int peaks = AudioFeatures::kNumberPeaks;
    for (int k = 0; k < feat->depth_; ++k) {
      for (int i = 0; i < peaks; ++i) {
        glBegin (GL_POINTS);
        //Sexy color scheme
        double factor = k / (1.0 * feat->depth_);
        int P = feat->past_peaks_[i + k * peaks];
        double A = feat->past_amplitudes_[i + k * peaks];
        if (P >= 5) {
          glVertex3f(.4 * xinc * P, -1.5 + K * A * A, distance + k);
          glVertex3f(.4 * xinc * -P, -1.5 + K * A * A, distance + k);
        }
        glEnd();
      }
    }

  }

  /*
   //DISPLAYS FFT
   if (feat->valid_FFT_) {
   glColor3f(0, 1, 1);
   for (int i = 0; i < feat->get_buffer_size() / 2; i++) {
   glBegin (GL_POINTS);
   glVertex3f(-4 + xinc * i, -3 + feat->get_NFFT_frame(i).norm(),
   distance);
   glEnd();
   }
   }
   */
  // flush!
  glFlush();
  // swap the double buffer
  glutSwapBuffers();
}

//-----------------------------------------------------------------------------
// name: changeLookAt()
// desc: changes the "camera"
//-----------------------------------------------------------------------------
void changeLookAt(pt3d look_from, pt3d look_to, pt3d head_up) {
  gluLookAt(look_from.x, look_from.y, look_from.z, look_to.x, look_to.y,
            look_to.z, head_up.x, head_up.y, head_up.z);
}

//-----------------------------------------------------------------------------
// Name: keyboardFunc( )
// Desc: key event
//-----------------------------------------------------------------------------
void keyboardFunc(unsigned char key, int x, int y) {
  switch (key) {
    case 'Q':
    case 'q':
      exit(1);
      break;
    case 'L':
    case 'l':
      g_look_from = pt3d(-1, 0, 0);
      cerr << "Looking from the left" << endl;
      break;
    case 'R':
    case 'r':
      g_look_from = pt3d(1, 0, 0);
      cerr << "Looking from the right" << endl;
      break;
    case 'F':
    case 'f':
      g_look_from = pt3d(0, 0, 1);
      cerr << "Looking from the front" << endl;
      break;
  }

  glutPostRedisplay();
}

//-----------------------------------------------------------------------------
// Name: mouseFunc( )
// Desc: handles mouse stuff
//-----------------------------------------------------------------------------
void mouseFunc(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON) {
    // when left mouse button is down, move left
    if (state == GLUT_DOWN) {
    } else {
    }
  } else if (button == GLUT_RIGHT_BUTTON) {
    // when right mouse button down, move right
    if (state == GLUT_DOWN) {
    } else {
    }
  } else {
  }

  glutPostRedisplay();
}

//-----------------------------------------------------------------------------
// Name: specialFunc( )
// Desc: handles special function keys
//-----------------------------------------------------------------------------
void specialFunc(int key, int x, int y) {
  if (key == GLUT_KEY_LEFT) {
    std::cerr << "Left arrow" << std::endl;
  }
  if (key == GLUT_KEY_RIGHT) {
    std::cerr << "Right arrow" << std::endl;
  }
  if (key == GLUT_KEY_DOWN) {
    std::cerr << "Down arrow" << std::endl;
  }
  if (key == GLUT_KEY_UP) {
    std::cerr << "Up arrow" << std::endl;
  }
  if (key == GLUT_KEY_PAGE_UP) {
    std::cerr << "PageUp arrow" << std::endl;
  }
  if (key == GLUT_KEY_PAGE_DOWN) {
    std::cerr << "PageDown arrow" << std::endl;
  }

  glutPostRedisplay();
}


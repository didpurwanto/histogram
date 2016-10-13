// main.cpp
// ========
// display histogram and equalize the histogram to enhance image contrast
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2002-03-11
// UPDATED: 2006-09-27
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>                      // for strcpy()
#include <GL/glut.h>
#include "Bmp.h"                        // BMP loader (namespace Image)
#include "histogram.h"                  // for histogram equalization
using namespace std;


// CALLBACK functions for GLUT ////////////////////////////////////////////////
void displayCB();
void reshapeCB(int w, int h);
void keyboardCB(unsigned char key, int x, int y);
void displaySubWin1CB();
void displaySubWin2CB();
void displaySubWin3CB();
void displaySubWin4CB();
void displaySubWin5CB();
void displaySubWin6CB();
void reshapeSubWin1CB(int w, int h);
void reshapeSubWin2CB(int w, int h);
void reshapeSubWin3CB(int w, int h);
void reshapeSubWin4CB(int w, int h);
void reshapeSubWin5CB(int w, int h);
void reshapeSubWin6CB(int w, int h);


// function declarations //////////////////////////////////////////////////////
void initGL();
int  initGLUT(int argc, char **argv);
bool initSharedMem();
void clearSharedMem();
void drawString(const char *str, int x, int y, float color[4], void *font);
unsigned int getMaxHistogram(const unsigned int *histogram, int size); // get max value in histogram


// CONSTANTS //////////////////////////////////////////////////////////////////
const char *FILE_NAME = "lena.bmp";
const int MAX_NAME = 512;               // max length of string
const int HISTOGRAM_SIZE = 256;         // histogram bin size

// global variables ///////////////////////////////////////////////////////////
char    fileName[MAX_NAME];             // image file name
int     imageX, imageY;                 // image resolution
int     mainWin;                        // GLUT main window ID
int     subWin1, subWin2, subWin3, subWin4, subWin5, subWin6; // GLUT sub window IDs
void    *font = GLUT_BITMAP_8_BY_13;    // GLUT font type
GLenum  format;                         // pixel format (GL_LUMINANCE, GL_RGB, GL_BGR_EXT, ...)
GLenum  type;                           // data type (GL_UNSIGNED_BYTE, GL_INT, ...)
const unsigned char *inBuf;             // image buffer
unsigned char *outBuf;                  // image buffer for histogram equalization
unsigned int *histogram;                // histogram bin
unsigned int *equalHistogram;           // histogram bin after equalization
unsigned int *sumHistogram;             // cumulative histogram
unsigned int *sumEqualHistogram;        // cumulative equalized histogram
float   histoScale;                     // to make histogram graph fit into window
float   sumHistoScale;                  // to make histogram graph fit into window
Image::Bmp bmp;                         // BMP loader object




///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // use default image file if not specified
    if(argc == 2)
    {
        strcpy(fileName, argv[1]);
    }
    else{
        cout << "Usage: " << argv[0] << " <BMP image-file>\n";
        strcpy(fileName, FILE_NAME);
        cout << "\nUse default image \"" << fileName << "\"" << endl;
    }

    // load source BMP image
    if(bmp.read(fileName))
    {
        inBuf = bmp.getData();
        imageX = bmp.getWidth();
        imageY = bmp.getHeight();
    }
    else // exit if failed to open image
    {
        cout << "[ERROR] ";
        cout << bmp.getError() << endl;
        return 0;
    }

    // allocate memory for arrays
    initSharedMem();

    // compute the histogram of the image
    dip::getHistogram(inBuf, imageX, imageY, histogram, HISTOGRAM_SIZE);
    dip::getSumHistogram(inBuf, imageX, imageY, sumHistogram, HISTOGRAM_SIZE);

    // The histogram graph may not fit into the rendering window.
    // compute scaling factor, so the graph can fit into the window
    // For example, if the height of window is 255 (0~255), then
    // the max value in histogram should be 255
    histoScale = (float)(imageY-1) / getMaxHistogram(histogram, HISTOGRAM_SIZE);
    sumHistoScale = (float)(imageY-1) / (imageX * imageY);  // max value of histogram is the number of pixels

    // enhance image with histogram equalization
    dip::equalizeHistogram(inBuf, imageX, imageY, outBuf);
    dip::getHistogram(outBuf, imageX, imageY, equalHistogram, HISTOGRAM_SIZE);
    dip::getSumHistogram(outBuf, imageX, imageY, sumEqualHistogram, HISTOGRAM_SIZE);

    // init GLUT and GL
    mainWin= initGLUT(argc, argv);
    initGL();

    // the last GLUT call (LOOP) ///////////////////////////////////////////////
    // window will be shown and display callback is triggered by events
    // NOTE: this call never return to main().
    glutMainLoop(); /* Start GLUT event-processing loop */

    return 0;
} // END OF MAIN //////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////////////////////
// initialize GLUT for windowing
///////////////////////////////////////////////////////////////////////////////
int initGLUT(int argc, char **argv)
{
    // GLUT stuff for windowing
    // initialization openGL window.
    // it is called before any other GLUT routine
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);   // display mode

    glutInitWindowSize(3*imageX, 2*imageY);         // window size

    glutInitWindowPosition(0, 0);                   // window location

    // finally, create a window with openGL context
    // Window will not displayed until glutMainLoop() is called
    // it returns a unique ID
    int handle = glutCreateWindow(argv[0]);     // param is the title of window

    // register GLUT callback functions
    glutDisplayFunc(displayCB);
    glutReshapeFunc(reshapeCB);
    glutKeyboardFunc(keyboardCB);
    //glutTimerFunc(100, timerCB, 100);             // redraw only every given millisec
    //glutMouseFunc(mouseCB);
    //glutMotionFunc(mouseMotionCB);

    // 6 sub-windows //////////////////////////////////////////////////////////
    // each sub-windows has its own openGL context, callbacks
    // share same keyboard callback function
    subWin1 = glutCreateSubWindow(handle, 0, 0, imageX, imageY);
    glutDisplayFunc(displaySubWin1CB);
    glutKeyboardFunc(keyboardCB);

    subWin2 = glutCreateSubWindow(handle, 0, imageY, imageX, imageY);
    glutDisplayFunc(displaySubWin2CB);
    glutKeyboardFunc(keyboardCB);

    subWin3 = glutCreateSubWindow(handle, imageX, 0, imageX, imageY);
    glutDisplayFunc(displaySubWin3CB);
    glutKeyboardFunc(keyboardCB);

    subWin4 = glutCreateSubWindow(handle, imageX, imageY, imageX, imageY);
    glutDisplayFunc(displaySubWin4CB);
    glutKeyboardFunc(keyboardCB);

    subWin5 = glutCreateSubWindow(handle, imageX*2, 0, imageX, imageY);
    glutDisplayFunc(displaySubWin5CB);
    glutKeyboardFunc(keyboardCB);

    subWin6 = glutCreateSubWindow(handle, imageX*2, imageY, imageX, imageY);
    glutDisplayFunc(displaySubWin6CB);
    glutKeyboardFunc(keyboardCB);

    return handle;
}



///////////////////////////////////////////////////////////////////////////////
// initialize OpenGL
// disable unused features
///////////////////////////////////////////////////////////////////////////////
void initGL()
{
    glClearColor(0, 0, 0, 0);
    glShadeModel(GL_FLAT);                      // shading mathod: GL_SMOOTH or GL_FLAT
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);      //

    // TUNNING IMAGING PERFORMANCE
    // turn off  all pixel path and per-fragment operation
    // which slow down OpenGL imaging operation (glDrawPixels glReadPixels...).
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_FOG);
    glDisable(GL_LIGHTING);
    glDisable(GL_LOGIC_OP);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_TEXTURE_1D);
    glDisable(GL_TEXTURE_2D);

    glPixelTransferi(GL_MAP_COLOR, GL_FALSE);
    glPixelTransferi(GL_RED_SCALE, 1);
    glPixelTransferi(GL_RED_BIAS, 0);
    glPixelTransferi(GL_GREEN_SCALE, 1);
    glPixelTransferi(GL_GREEN_BIAS, 0);
    glPixelTransferi(GL_BLUE_SCALE, 1);
    glPixelTransferi(GL_BLUE_BIAS, 0);
    glPixelTransferi(GL_ALPHA_SCALE, 1);
    glPixelTransferi(GL_ALPHA_BIAS, 0);
}



///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem()
{
    // allocate memory for image data
    outBuf = new unsigned char[imageX * imageY];
    if(!outBuf) return false;    // exit if failed allocation

    histogram = new unsigned int[HISTOGRAM_SIZE];
    if(!histogram) return false;    // exit if failed allocation

    equalHistogram = new unsigned int[HISTOGRAM_SIZE];
    if(!equalHistogram) return false;    // exit if failed allocation

    sumHistogram = new unsigned int[HISTOGRAM_SIZE];
    if(!sumHistogram) return false;    // exit if failed allocation

    sumEqualHistogram = new unsigned int[HISTOGRAM_SIZE];
    if(!sumEqualHistogram) return false;    // exit if failed allocation

    return true;
}


///////////////////////////////////////////////////////////////////////////////
// deallocated shrared memory and clean up
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
    // decallocate arrays
    delete [] outBuf;
    delete [] histogram;
    delete [] equalHistogram;
    delete [] sumHistogram;
    delete [] sumEqualHistogram;

    inBuf = outBuf = 0;
    histogram = equalHistogram = sumHistogram = sumEqualHistogram = 0;
}



///////////////////////////////////////////////////////////////////////////////
// write 2d text using GLUT
// The projection matrix must be set to orthogonal before call this function.
///////////////////////////////////////////////////////////////////////////////
void drawString(const char *str, int x, int y, float color[4], void *font)
{
    //glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    //glDisable(GL_LIGHTING);     // need to disable lighting for proper text color

    glColor4fv(color);          // set text color
    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    //glEnable(GL_LIGHTING);
    //glPopAttrib();
}



///////////////////////////////////////////////////////////////////////////////
// find the max frequency of histogram
///////////////////////////////////////////////////////////////////////////////
unsigned int getMaxHistogram(const unsigned int *histogram, int size)
{
    if(!histogram)
        return 0;

    unsigned int maxFreq = 0;

    for(int i = 0; i < size; ++i)
    {
        if(histogram[i] > maxFreq)
            maxFreq = histogram[i];
    }

    return maxFreq;
}



//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB(void)
{
    glutSetWindow(mainWin);
    glClear(GL_COLOR_BUFFER_BIT);

    glutSwapBuffers();
}


///////////////////////////////////////////////////////////////////////////////
// draw original image at top-left corner
///////////////////////////////////////////////////////////////////////////////
void displaySubWin1CB()
{
    glutSetWindow(subWin1);
    glClear(GL_COLOR_BUFFER_BIT);       // clear canvas

    // specify current paster position(coordinate)
    glRasterPos2i(0, imageY);           // upper-left corner in openGL coordinates
    glPixelZoom(1.0f, -1.0f);

    // draw 8-bit grayscale image
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDrawPixels(imageX, imageY, GL_LUMINANCE, GL_UNSIGNED_BYTE, inBuf);
    glPixelZoom(1.0f, 1.0f);

    GLfloat color[4] = {0, 0, 1, 1};
    drawString("Original Image", 1, imageY-14, color, font);

    glutSwapBuffers();
}



///////////////////////////////////////////////////////////////////////////////
// draw image after histogram equalization at bottom-left window
///////////////////////////////////////////////////////////////////////////////
void displaySubWin2CB()
{
    glutSetWindow(subWin2);
    glClear(GL_COLOR_BUFFER_BIT);       // clear canvas

    // specify current paster position(coordinate)
    glRasterPos2i(0, imageY);           // upper-left corner in openGL coordinates
    glPixelZoom(1.0f, -1.0f);

    // draw 8-bit grayscale image
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glDrawPixels(imageX, imageY, GL_LUMINANCE, GL_UNSIGNED_BYTE, outBuf);
    glPixelZoom(1.0f, 1.0f);

    GLfloat color[4] = {1, 0, 0, 1};
    drawString("Enhanced Image", 1, imageY-14, color, font);

    glutSwapBuffers();
}



///////////////////////////////////////////////////////////////////////////////
// draw original histogram at top-middle window
///////////////////////////////////////////////////////////////////////////////
void displaySubWin3CB()
{
    glutSetWindow(subWin3);
    glClear(GL_COLOR_BUFFER_BIT);       // clear canvas

    glColor3f(0, 0, 1);

    glBegin(GL_LINES);
        for(int i = 0; i < HISTOGRAM_SIZE; i++)
        {
            glVertex2f(i, 0);
            glVertex2f(i, histogram[i] * histoScale);
        }
    glEnd();

    GLfloat color[4] = {1, 1, 1, 1};
    drawString("Original Histogram", 1, imageY-14, color, font);

    glutSwapBuffers();
}



///////////////////////////////////////////////////////////////////////////////
// draw equalized histogram at bottom-middle window
///////////////////////////////////////////////////////////////////////////////
void displaySubWin4CB()
{
    glutSetWindow(subWin4);
    glClear(GL_COLOR_BUFFER_BIT);           // clear canvas

    glColor3f(1, 0, 0);
    glBegin(GL_LINES);
        for(int i = 0; i < HISTOGRAM_SIZE; ++i)
        {
            glVertex2f(i, 0);
            glVertex2f(i, equalHistogram[i] * histoScale);
        }
    glEnd();

    GLfloat color[4] = {1, 1, 1, 1};
    drawString("Equalized Histogram", 1, imageY-14, color, font);

    glutSwapBuffers();
}



///////////////////////////////////////////////////////////////////////////////
// draw original cumulative histogram at top-right window
///////////////////////////////////////////////////////////////////////////////
void displaySubWin5CB()
{
    glutSetWindow(subWin5);
    glClear(GL_COLOR_BUFFER_BIT);           // clear canvas

    glColor3f(0.5f, 0.5f, 1);
    glBegin(GL_LINES);
        for(int i = 0; i < HISTOGRAM_SIZE; ++i)
        {
            glVertex2f(i, 0);
            glVertex2f(i, sumHistogram[i] * sumHistoScale);
        }
    glEnd();

    GLfloat color[4] = {1, 1, 1, 1};
    drawString("Cumulative Histogram", 1, imageY-14, color, font);

    glutSwapBuffers();
}



///////////////////////////////////////////////////////////////////////////////
// draw equalized cumulative histogram at bottom-right window
///////////////////////////////////////////////////////////////////////////////
void displaySubWin6CB()
{
    glutSetWindow(subWin6);
    glClear(GL_COLOR_BUFFER_BIT);           // clear canvas

    glColor3f(1, 0.5f, 0.5f);
    glBegin(GL_LINES);
        for(int i = 0; i < HISTOGRAM_SIZE; ++i)
        {
            glVertex2f(i, 0);
            glVertex2f(i, sumEqualHistogram[i] * sumHistoScale);
        }
    glEnd();

    GLfloat color[4] = {1, 1, 1, 1};
    drawString("Cumulative Equalized Histogram", 1, imageY-14, color, font);

    glutSwapBuffers();
}



void reshapeCB(int w, int h)
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);

    // top-left sub-window
    glutSetWindow(subWin1);
    glutPositionWindow(0,0);
    glutReshapeWindow(imageX, imageY);
    glViewport(0, 0, imageX, imageY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, imageX, 0, imageY, -1, 1);

    // bottom-left sub-window
    glutSetWindow(subWin2);
    glutPositionWindow(0, imageY);
    glutReshapeWindow(imageX, imageY);
    glViewport(0, 0, imageX, imageY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, imageX, 0, imageY, -1, 1);

    // top-middle sub-window
    glutSetWindow(subWin3);
    glutPositionWindow(imageX, 0);
    glutReshapeWindow(imageX, imageY);
    glViewport(0, 0, imageX, imageY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, imageX, 0, imageY, -1, 1);

    // bottom-middle sub-window
    glutSetWindow(subWin4);
    glutPositionWindow(imageX,imageY);
    glutReshapeWindow(imageX, imageY);
    glViewport(0, 0, imageX, imageY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, imageX, 0, imageY, -1, 1);

    // top-right sub-window
    glutSetWindow(subWin5);
    glutPositionWindow(imageX*2,0);
    glutReshapeWindow(imageX, imageY);
    glViewport(0, 0, imageX, imageY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, imageX, 0, imageY, -1, 1);

    // bottom-right sub-window
    glutSetWindow(subWin6);
    glutPositionWindow(imageX*2,imageY);
    glutReshapeWindow(imageX, imageY);
    glViewport(0, 0, imageX, imageY);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, imageX, 0, imageY, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}



void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27: // ESCAPE
        clearSharedMem();
        exit(0);
        break;

    case 'r':
    case 'R':
        break;

    case '+':
    case '=':
        break;

    case '-':
    case '_':
        break;
    }

    glutPostRedisplay();
}

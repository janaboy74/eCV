/* Copyright by János Klingl in 2023 */

#ifndef GLCORE_H
#define GLCORE_H

#ifndef __int64
#define __int64 long long
#endif
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <GL/gl.h>
#ifdef _WIN32
#include <windows.h>
#include <GL/wglew.h>
#include <GL/wglext.h>
#endif
#ifdef __linux
#include <X11/X.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <xcb/xcb.h>
#include <GL/glx.h>
#define GLX_GLXEXT_PROTOTYPES 1
#include <GL/glu.h>
#include <unistd.h>
#endif
#include <vector>
#include <iostream>
#include <string.h>
#include "Graphics.h"

/* Cross platform window declaration and functions */

class CrossWindow {
public:
    CrossWindow();
    ~CrossWindow();
    int createWindow();
    GLuint CompileShaders( const char *vertexShaderSource, const char *fragmentShaderSource );
    void ProcessMessages();
    void SwapBuffer();
    void Cleanup();
    void Run();
    void StartMainLoop();
    virtual void Init() = 0;
    virtual void HandleEvents() = 0;
    virtual void Finish() = 0;
    virtual void keyPress( char key ) = 0;
    virtual void keyRelease( char key ) = 0;
    virtual void mouseMove( float x, float y ) = 0;
    virtual void mousePress() = 0;
    virtual void mouseRelease() = 0;
    virtual void paint() = 0;
protected:
#ifdef __linux
    Display *display;
    xcb_screen_t *screen;
    GLXDrawable drawable;
    GLXWindow glxwindow;
    GLXContext context;
    xcb_connection_t *connection;
    xcb_window_t window;
    XWindowAttributes xwatr;
#endif
#ifdef _WIN32
    HWND hwnd;
    MSG msg;
    HDC hdc;
    HGLRC hglrc;
#endif
public:
    bool loop;
    int winx;
    int winy;
    int winw;
    int winh;
    int x;
    int y;
    int cx;
    int cy;
    int schw;
    int schh;
    void checkGLError();
};

/* Shader program helpers */

class Program
{
public:
    Program();
    Program( const Program &other );
    Program &operator = ( const Program &other );
    GLuint CompileShaders( const char *vertexShaderSource, const char *fragmentShaderSource );
    void useProgram();
    GLuint getProgram() const;
    void resetProgream();
    void enablePosition( bool enable = true );
    void enableNormal( bool enable = true );
    void setUniform( const char *value, const float f1 );
    void setUniform( const char *value, const float f1, const float f2 );
    void setUniform( const char *value, const float f1, const float f2, const float f3 );
    void setUniform( const char *value, const float f1, const float f2, const float f3, const float f4 );
    void setUniform( const char *value, const Vector2D &vector2d );
    void setUniform( const char *value, const Vector3D &vector3d );
    void setUniform( const char *value, const Matrix &matrix );
    void drawMesh( const Mesh &mesh ) const;
protected:
    GLuint mProgram;
    GLuint mUPos;
    GLuint mPositionAttribute;
    GLuint mNormalAttribute;
};

#endif // GLCORE_H

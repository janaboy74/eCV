#include "GLCore.h"
#include <thread>
#include <stdlib.h>

using namespace std;

#ifdef __linux
static int visual_attribs[] =
{
    GLX_X_RENDERABLE, True,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    GLX_DOUBLEBUFFER, True,
    GLX_SAMPLE_BUFFERS  , 1,
#if RPI4
    GLX_SAMPLES         , 2,
#else
    GLX_SAMPLES         , 8,
#endif
    None
};
#endif

#ifdef _WIN32
extern const char *ApplicationClassName;
extern const char *ApplicationName;
LRESULT CALLBACK WindowProcedure( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    switch( message ) {
    case WM_ERASEBKGND:
        return 1;
    default:
        return DefWindowProc( hwnd, message, wParam, lParam );
    }

    return 0;
}
#define CLEAR( value ) memset( &value, 0, sizeof( value ))
#endif

CrossWindow::CrossWindow()
{
}

CrossWindow::~CrossWindow()
{
}

int CrossWindow::createWindow()
{
#ifdef __linux
    setenv( "MESA_GL_VERSION_OVERRIDE" , "3.3", true );

    screen = 0;
    drawable = 0;

    int default_screen;

    display = XOpenDisplay( 0 );
    if( !display ) {
        cerr << "Can't open display\n";
        return -1;
    }

    default_screen = DefaultScreen( display );

    connection = XGetXCBConnection( display );
    if( !connection ) {
        XCloseDisplay( display );
        cerr << "Can't get xcb connection from display\n";
        return -1;
    }

    XSetEventQueueOwner( display, XCBOwnsEventQueue );

    xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator( xcb_get_setup( connection ));
    for( int screen_num = default_screen; screen_iter.rem && screen_num > 0; --screen_num, xcb_screen_next( &screen_iter ));
    screen = screen_iter.data;

    int visualID = 0;

    GLXFBConfig *fb_configs = 0;
    int num_fb_configs = 0;
    fb_configs = glXChooseFBConfig( display, default_screen, visual_attribs, &num_fb_configs );
    if( !fb_configs || num_fb_configs == 0 ) {
        cerr << "glXGetFBConfigs failed\n";
        return -1;
    }

    GLXFBConfig fb_config = fb_configs[ 0 ];
    glXGetFBConfigAttrib( display, fb_config, GLX_VISUAL_ID , &visualID );

    context = glXCreateNewContext( display, fb_config, GLX_RGBA_TYPE, 0, True );
    if( !context ) {
        cerr << "glXCreateNewContext failed\n";
        return -1;
    }

    xcb_colormap_t colormap = xcb_generate_id( connection );
    window = xcb_generate_id( connection );

    xcb_create_colormap( connection, XCB_COLORMAP_ALLOC_NONE, colormap, screen->root, visualID );

    uint32_t eventmask = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE | XCB_EVENT_MASK_BUTTON_PRESS |
            XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION;

// Fullscreen switch : off -> debug
#if 1
    uint32_t valuelist[] = { True, eventmask, colormap, 0 };
    uint32_t valuemask = XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;
#else
    uint32_t valuelist[] = { eventmask, colormap, 0 };
    uint32_t valuemask = XCB_CW_EVENT_MASK | XCB_CW_COLORMAP;
#endif
    XGetWindowAttributes( display, screen->root, &xwatr );

    xcb_create_window( connection, XCB_COPY_FROM_PARENT, window, screen->root,
                0, 0, screen->width_in_pixels, screen->height_in_pixels, 0,
                XCB_WINDOW_CLASS_INPUT_OUTPUT, visualID, valuemask, valuelist );

    xcb_map_window( connection, window );
    xcb_set_input_focus( connection, XCB_INPUT_FOCUS_POINTER_ROOT, window, XCB_CURRENT_TIME );

    glxwindow = glXCreateWindow( display, fb_config, window, 0 );

    if( !window ) {
        xcb_destroy_window( connection, window );
        glXDestroyContext( display, context );

        cerr << "glXDestroyContext failed\n";
        return -1;
    }

    drawable = glxwindow;

    if( !glXMakeContextCurrent( display, drawable, drawable, context )) {
        xcb_destroy_window( connection, window );
        glXDestroyContext( display, context );

        cerr << "glXMakeContextCurrent failed\n";
        return -1;
    }
    Cursor invisibleCursor;
    Pixmap bitmapNoData;
    XColor black;
    static char noData[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    black.red = black.green = black.blue = 0;

    bitmapNoData = XCreateBitmapFromData( display, window, noData, 8, 8 );
    invisibleCursor = XCreatePixmapCursor( display, bitmapNoData, bitmapNoData, &black, &black, 0, 0 );
    XDefineCursor( display,window, invisibleCursor );
    XFreeCursor( display, invisibleCursor );
    XFreePixmap( display, bitmapNoData );
    Window child;
    XGetWindowAttributes( display, window, &xwatr );
    XTranslateCoordinates( display, window, screen->root, xwatr.x, xwatr.y, &winx, &winy, &child );
    winw = xwatr.width;
    winh = xwatr.height;
#endif
#ifdef _WIN32
    SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );
    if( AttachConsole( ATTACH_PARENT_PROCESS ) )
    {
        freopen( "CONIN$",  "rb", stdin );   // reopen stdin handle as console window input
        freopen( "CONOUT$", "wb", stdout );  // reopen stdout handle as console window output
        freopen( "CONOUT$", "wb", stderr );  // reopen stderr handle as console window output
    }
    WNDCLASSEX wincl;
    PIXELFORMATDESCRIPTOR pfd;
    CLEAR( wincl );
    CLEAR( pfd );

    wincl.hInstance = GetModuleHandle( NULL );
    wincl.lpszClassName = ApplicationClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof( WNDCLASSEX );

    wincl.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wincl.hIconSm = LoadIcon( NULL, IDI_APPLICATION );
    wincl.hCursor = LoadCursor( NULL, IDC_CROSS );
    wincl.hbrBackground = ( HBRUSH ) COLOR_BACKGROUND;
    ShowCursor( FALSE );

    if ( !RegisterClassEx( &wincl ))
        return 0;
    winw = GetSystemMetrics( SM_CXSCREEN );
    winh = GetSystemMetrics( SM_CYSCREEN );

    hwnd = CreateWindowEx( 0, ApplicationClassName, ApplicationName, WS_POPUP, 0, 0, 10, 10, 0, 0, wincl.hInstance, 0 );
    hdc = GetDC( hwnd );

    memset( &pfd, 0, sizeof( PIXELFORMATDESCRIPTOR ) );
    pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR );
    pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.iLayerType = PFD_MAIN_PLANE;

    int nPixelFormat = ChoosePixelFormat( hdc, &pfd );

    SetPixelFormat( hdc, nPixelFormat, &pfd );

    hglrc = wglCreateContext( hdc );

    wglMakeCurrent( hdc, hglrc );
#endif
    schw = winw * 0.5f;
    schh = winh * 0.5f;
    GLenum err = glewInit();
    if( GLEW_OK != err ) {
        cout << glewGetErrorString( err );
    }
#ifdef __linux
    xcb_warp_pointer( connection, XCB_NONE, screen->root, 0, 0, 0, 0, winx + schw, winy + schh);
//    glXSwapIntervalEXT( display, drawable, 0 );
#endif
#ifdef _WIN32
    wglMakeCurrent( NULL, NULL );
    wglDeleteContext( hglrc );
    ReleaseDC( hwnd, hdc );
    DestroyWindow( hwnd );

    hwnd = CreateWindowEx( 0, ApplicationClassName, ApplicationName ,WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, winw, winh, 0, 0, wincl.hInstance, 0 );
    hdc = GetDC( hwnd );

    const int iPixelFormatAttribList[] = {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
        WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB, 32,
        WGL_DEPTH_BITS_ARB, 24,
        WGL_STENCIL_BITS_ARB, 8,
        WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
        WGL_SAMPLES_ARB, 8,
        0 // End of attributes list
    };
#if 0
    int attributes[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3 ,
        WGL_CONTEXT_MINOR_VERSION_ARB, 2 ,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        0
    };
#endif
    nPixelFormat = 0;
    UINT iNumFormats = 0;

    wglChoosePixelFormatARB( hdc, iPixelFormatAttribList, NULL, 1, &nPixelFormat, (UINT*)&iNumFormats );

    SetPixelFormat( hdc, nPixelFormat, &pfd );

    hglrc = wglCreateContext( hdc );
#if 0
    hglrc = wglCreateContextAttribsARB( hdc, 0, attributes );
#endif

    wglMakeCurrent( NULL, NULL );
    wglMakeCurrent( hdc, hglrc );
    ShowWindow( hwnd, SW_SHOW );
    SetCursorPos( schw, schh );
    wglSwapIntervalEXT( 0 );
#endif
    return 0;
}

void CrossWindow::ProcessMessages()
{
    int MAX_EVENT = 500;
#ifdef __linux
    sleep( 0 );
    xcb_flush( connection );
    int16_t mX = winx + schw;
    int16_t mY = winy + schh;

    for( int e = 0; e < MAX_EVENT; ++e ) {
        xcb_generic_event_t *event = xcb_poll_for_event( connection );
        if( event ) {
            unique_ptr<xcb_generic_event_t> freeEvent( event );
            uint8_t response_type = event->response_type & ~0x80;

            switch( response_type ) {
            case XCB_KEY_PRESS:
            case XCB_KEY_RELEASE:
                {
                    bool press = ( XCB_KEY_PRESS == response_type );
                    xcb_key_press_event_t* e = reinterpret_cast<xcb_key_press_event_t*>(event);
                    KeySym keySym = XkbKeycodeToKeysym( display, e->detail, 0, 1 );
                    if( 'A' <= keySym && keySym <= 'Z' ) {
                        press ? keyPress( *XKeysymToString( keySym )) : keyRelease( *XKeysymToString( keySym ));
                    } else {
                        switch( keySym ) {
                        case XK_Escape:
                            press ? keyPress( 27 ) : keyRelease( 27 );
                            break;
                        case XK_space:
                            press ? keyPress( 32 ) : keyRelease( 32 );
                            break;
                        }
                    }
                }
                break;
            case XCB_BUTTON_PRESS:
                mousePress();
                break;
            case XCB_BUTTON_RELEASE:
                mouseRelease();
                break;
            case XCB_MOTION_NOTIFY:
                {
                    xcb_motion_notify_event_t* e = reinterpret_cast<xcb_motion_notify_event_t*>(event);
                    mX = e->root_x;
                    mY = e->root_y;
                    break;
                }
            case XCB_EXPOSE:
                e = MAX_EVENT;
                break;
            }
        } else {
            e = MAX_EVENT;
        }
    }
    if( winx + schw != mX || winy - schh != mY ) {
        mouseMove( 2 * ( mX - winx - schw ), 2 * ( mY - winy - schh ));
        xcb_warp_pointer( connection, XCB_NONE, screen->root, 0, 0, 0, 0, winx + schw, winy + schh);
    }

    XGetWindowAttributes( display, window, &xwatr );
    winw = xwatr.width;
    winh = xwatr.height;
#endif
#ifdef _WIN32
    if( !IsWindowVisible( hwnd ))
        PostQuitMessage( 0 );
    Sleep( 0 );
    for( int e = 0; e < 500; ++e ) {
        if ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE )) {
            switch( msg.message ) {
            case WM_QUIT:
                loop = false;
                break;
            case WM_LBUTTONDOWN:
                SetCapture( hwnd );
                cx = LOWORD( msg.lParam );
                cy = HIWORD( msg.lParam );
                mousePress();
                break;
            case WM_LBUTTONUP:
                ReleaseCapture();
                mouseRelease();
                break;
            case WM_KEYDOWN:
                if( 'A' <= msg.wParam && msg.wParam <= 'Z' ) {
                    keyPress( msg.wParam );
                } else {
                    switch( msg.wParam ) {
                    case VK_ESCAPE:
                        keyPress( 27 );
                        break;
                    case VK_SPACE:
                        keyPress( 32 );
                        break;
                    }
                }
                break;
            case WM_MOUSEMOVE:
                x = LOWORD( msg.lParam );
                y = HIWORD( msg.lParam );
                mouseMove( 2 * ( x - schw ), 2 * ( y - schh ));
                SetCursorPos( schw, schh );
                break;
            case WM_SIZE:
                winw = LOWORD( msg.lParam );
                winh = HIWORD( msg.lParam );
                break;
            case WM_PAINT:
                e = MAX_EVENT;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } else {
            e = MAX_EVENT;
            break;
        }
    }
#endif
}

void CrossWindow::SwapBuffer()
{
#ifdef __linux
    glXSwapBuffers( display, drawable );
#endif
#ifdef _WIN32
    SwapBuffers( hdc );
#endif
}

void CrossWindow::Cleanup()
{
    /* Cleanup */
#ifdef __linux
    if( display ) {
        glXDestroyContext(display, context);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
        display = 0;
    }
#endif
#ifdef _WIN32
    if( hwnd ) {
        wglMakeCurrent( NULL, NULL );
        wglDeleteContext( hglrc );
        ReleaseDC( hwnd, hdc );
        hwnd = 0;
    }
#endif
}

void CrossWindow::Run()
{
    Init();
    StartMainLoop();
}

void CrossWindow::StartMainLoop()
{
    loop = true;
    while( loop ) {
        ProcessMessages();
        paint();
        SwapBuffer();
        HandleEvents();
    }
#ifdef _WIN32
    PostQuitMessage( 0 );
#endif
    Finish();
    Cleanup();
    cout.flush();
}

void CrossWindow::checkGLError()
{
    GLenum err = glGetError();

    if ( err != GL_NO_ERROR ) {
        if ( err == GL_INVALID_OPERATION )
            cout << "OpenGL error: GL_INVALID_OPERATION\n";
        else if ( err == GL_INVALID_VALUE )
            cout << "OpenGL error: GL_INVALID_VALUE\n";
        else
            cout << "OpenGL error: " << err << endl;
    }
}

Program::Program() {
    mProgram = -1;
    mUPos = -1;
    mPositionAttribute = -1;
    mNormalAttribute = -1;
}

Program::Program( const Program &other ) {
    *this = other;
}

Program &Program::operator =( const Program &other ) {
    mProgram = other.mProgram;
    return *this;
}

GLuint Program::CompileShaders( const char *vertexShaderSource, const char *fragmentShaderSource ) {
    GLuint vertShader = glCreateShader( GL_VERTEX_SHADER );
    GLuint fragShader = glCreateShader( GL_FRAGMENT_SHADER );

    vector<char> errorString;
    GLint result = GL_FALSE;
    int logLength;

    glShaderSource( vertShader, 1, &vertexShaderSource, NULL );
    glCompileShader( vertShader );

    glGetShaderiv( vertShader, GL_COMPILE_STATUS, &result );
    if( result == GL_FALSE ) {
        glGetShaderiv( vertShader, GL_INFO_LOG_LENGTH, &logLength );
        errorString.reserve(( logLength > 1 ) ? logLength : 1 );
        glGetShaderInfoLog( vertShader, logLength, NULL, &errorString[ 0 ]);
        cout << &errorString[ 0 ] << endl;
    }

    glShaderSource( fragShader, 1, &fragmentShaderSource, NULL );
    glCompileShader( fragShader);

    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result );
    if( result == GL_FALSE ) {
        glGetShaderiv( fragShader, GL_INFO_LOG_LENGTH, &logLength );
        errorString.reserve(( logLength > 1 ) ? logLength : 1 );
        glGetShaderInfoLog( fragShader, logLength, NULL, &errorString[ 0 ]);
        cout << &errorString[ 0 ] << endl;
    }

    mProgram = glCreateProgram();
    glAttachShader( mProgram, vertShader );
    glAttachShader( mProgram, fragShader );
    glLinkProgram( mProgram );

    glGetProgramiv( mProgram, GL_LINK_STATUS, &result );
    if( result == GL_FALSE ) {
        glGetProgramiv( mProgram, GL_INFO_LOG_LENGTH, &logLength );
        errorString.reserve(( logLength > 1 ) ? logLength : 1 );
        glGetProgramInfoLog( mProgram, logLength, NULL, &errorString[ 0 ]);
        cout << &errorString[ 0 ] << endl;
    }

    glDeleteShader( vertShader );
    glDeleteShader( fragShader );

    return mProgram;
}

void Program::useProgram() {
    glUseProgram( mProgram );
    mPositionAttribute = glGetAttribLocation( mProgram, "posAttr" );
    mNormalAttribute = glGetAttribLocation( mProgram, "normalAttr" );
}

GLuint Program::getProgram() const {
    return mProgram;
}

void Program::resetProgream()
{
    glUseProgram( 0 );
}

void Program::enablePosition( bool enable ) {
    if( enable )
        glEnableVertexAttribArray( mPositionAttribute );
    else
        glDisableVertexAttribArray( mPositionAttribute );
}

void Program::enableNormal( bool enable ) {
    if( enable )
        glEnableVertexAttribArray( mNormalAttribute );
    else
        glDisableVertexAttribArray( mNormalAttribute );
}

void Program::setUniform( const char *value, const float f1 ) {
    mUPos = glGetUniformLocation( mProgram, value );
    glUniform1f( mUPos, f1 );
}

void Program::setUniform( const char *value, const float f1, const float f2 ) {
    mUPos = glGetUniformLocation( mProgram, value );
    glUniform2f( mUPos, f1, f2 );
}

void Program::setUniform( const char *value, const float f1, const float f2, const float f3 ) {
    mUPos = glGetUniformLocation( mProgram, value );
    glUniform3f( mUPos, f1, f2, f3 );
}

void Program::setUniform( const char *value, const float f1, const float f2, const float f3, const float f4 ) {
    mUPos = glGetUniformLocation( mProgram, value );
    glUniform4f( mUPos, f1, f2, f3, f4 );
}

void Program::setUniform( const char *value, const Vector2D &vector2d ) {
    setUniform( value, vector2d.x, vector2d.y );
}

void Program::setUniform( const char *value, const Vector3D &vector3d ) {
    setUniform( value, vector3d.x, vector3d.y, vector3d.z );
}

void Program::setUniform( const char *value, const Matrix &matrix ) {
    mUPos = glGetUniformLocation( mProgram, value );
    glUniformMatrix4fv( mUPos, 1, GL_FALSE, matrix.getFloatPtr() );
}

void Program::drawMesh( const Mesh &mesh ) const
{
    glVertexAttribPointer( mPositionAttribute, 3, GL_FLOAT, GL_FALSE, 0, mesh.mVertices.data() );
    glVertexAttribPointer( mNormalAttribute, 3, GL_FLOAT, GL_FALSE, 0, mesh.mNormals.data() );
    glDrawElements( GL_TRIANGLES, mesh.mIndices.size(), GL_UNSIGNED_INT, mesh.mIndices.data() );
}

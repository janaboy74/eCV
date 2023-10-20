/* Copyright by János Klingl in 2023 */

#ifndef ECV_H
#define ECV_H

#include <Core.h>
#include <Settings.h>
#include <GLCore.h>
#include <Graphics.h>
#include <Faces.h>
#include <Triangles.h>
#include <VectorFont.h>
#include <memory>
#include <thread>

/* Storage for a single character */

struct singleChar
{
    /* Constructor */                               singleChar( std::shared_ptr<VectorFont::Char3D> letter = nullptr );
    /* Copy constructor */                          singleChar( const singleChar &other );
    singleChar *                                    operator = ( const singleChar &other );

    float                                           minx;
    std::shared_ptr<VectorFont::Char3D>             letter;
    Vector3D                                        move_from;
    float                                           rotate_y;
    float                                           rotate_z;
    float                                           time;
    float                                           speed;
    int                                             line;
};

/* Storage for a 3D text */

struct Letters3D
{
    /* Copy constructor */                          Letters3D( const VectorFont::Text3D &text );
    Letters3D *                                     operator = ( const VectorFont::Text3D &text );

    std::vector< singleChar >                       letters;
    float                                           start_time;

    void                                            randomize();
};

/* Helper class for animate 3D texts */

class AnimHandler
{
public:
    enum ANIMTYPE {
        AT_NONE,
        AT_LINEAR,
        AT_QUAD
    };
private:
    Timer< chrono::milliseconds, chrono::steady_clock> timer;
    ANIMTYPE                                        animType;
    Vector2D                                        shift;
    Vector2D                                        scaleFactor;
    Vector2D                                        axis;
    float                                           backwardTime;
    float                                           speed;
    bool                                            running;
public:
    /* Constructor */                               AnimHandler( ANIMTYPE animType = AT_NONE );
    void                                            setAnimType( ANIMTYPE animType );
    void                                            move( Vector2D shift );
    void                                            moveXtoZero( float shiftX );
    void                                            scale( Vector2D scale );
    bool                                            isBackward();
    void                                            backward( float time );
    void                                            calcAxis( Vector2D pos );
    void                                            start();
    void                                            stop();
    Vector2D &                                      getAxis();
    float                                           getSpeed();
    float                                           getTime();
    float                                           getValue();
    bool                                            isRunning();
};

/* Implemention of the main window and functions */

class ECV : public CrossWindow {
public:
    ECV();

    float                                           minx;

    /* Cross platform callback functions */

    virtual void                                    Init() override;
    virtual void                                    HandleEvents() override;
    virtual void                                    Finish() override;
    virtual void                                    keyPress( char key ) override;
    virtual void                                    keyRelease( char key ) override;
    virtual void                                    mouseMove( float x, float y ) override;
    virtual void                                    mousePress() override;
    virtual void                                    mouseRelease() override;
    virtual void                                    paint() override;

    VectorFont                                      defaultFont;
    Triangles                                       mCursor;
    float                                           xPos;
    float                                           yPos;
    bool                                            mMove;
    bool                                            mStop;
    bool                                            mWire;
    bool                                            mTransp;
    bool                                            mPause;
    GLenum                                          format;
    GLuint                                          texId;
    float                                           diffClock;
    float                                           angle;
    Program                                         program;
    Program                                         minimalProgram;
    Timer< chrono::milliseconds, chrono::steady_clock> clock;
    std::shared_ptr<Letters3D>                      mChars;
};

#endif // ECV_H

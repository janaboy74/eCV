#include "eCV.h"
#include <math.h>
#include "Roboto_Regular.h"
#include "Intro.h"

using namespace std;

const char *ApplicationClassName = "eCV";
const char *ApplicationName = "Electoric Introduction";

const char *vertexShaderSource =
    "#version 330\n"
    "attribute lowp vec4 posAttr;\n"
    "attribute lowp vec4 normalAttr;\n"
    "varying lowp vec4 normal;\n"
    "varying lowp vec4 pos;\n"
    "uniform lowp mat4 projection;\n"
    "uniform lowp mat4 view;\n"
    "uniform lowp mat4 model;\n"
    "void main() {\n"
    "   lowp mat4 object = model;"
    "   object[3][0] = 0;\n"
    "   object[3][1] = 0;\n"
    "   object[3][2] = 0;\n"
    "   object[3][3] = 0;\n"
    "   normal = object * normalAttr;\n"
    "   gl_Position = projection * view * model * posAttr;\n"
    "   pos = view * model * posAttr;\n"
    "}\n";

const char *fragmentShaderSource =
    "#version 330\n"
    "uniform lowp float ambient;\n"
    "uniform lowp vec4 color;\n"
    "uniform lowp vec4 specColor;\n"
    "varying lowp vec4 normal;\n"
    "varying lowp vec4 pos;\n"
    "uniform lowp vec4 lightPos;\n"
    "void main() {\n"
    "   vec4 lightDir = normalize( lightPos - pos );\n"
    "   vec4 norm = normalize( normal );\n"
    "   vec4 reflectDir = reflect( -lightDir, norm );\n"
    "   float diff = max( dot( -lightDir, norm ), ambient );\n"
    "   float spec = 3.0f * pow( max( 3.f * dot( normalize( pos ), reflectDir ) - 2.f, 0.f ), 8.f );\n"
    "   gl_FragColor = diff * color + spec * specColor;\n"
    "   gl_FragColor.a = color.a;\n"
    "}\n";

const char *minimalFragmentShaderSource =
        "#version 330\n"
        "uniform lowp vec4 color;\n"
        "void main() {\n"
        "   gl_FragColor = color;\n"
        "}\n";

singleChar::singleChar( std::shared_ptr<VectorFont::Char3D> letter  ) {
    this->letter = letter;
    rotate_y = 0;
    rotate_z = 0;
    time = 0;
}

singleChar::singleChar(const singleChar &other)
{
    operator =( other );
}

singleChar *singleChar::operator = ( const singleChar &other )
{
    letter = other.letter;
    move_from.x = other.move_from.x;
    move_from.y = other.move_from.y;
    move_from.z = other.move_from.z;
    rotate_y = other.rotate_y;
    rotate_z = other.rotate_z;
    time = other.time;
    speed = other.speed;
    return this;
}

Letters3D::Letters3D( const VectorFont::Text3D &text )
{
    operator =( text );
}

Letters3D *Letters3D::operator =( const VectorFont::Text3D &text )
{
   singleChar chr;
   chr.rotate_y = 0;
   for( auto letter : text ) {
       letters.push_back( singleChar( letter ));
   }
   return this;
}

void Letters3D::randomize()
{
    float speed = 0;
    float ypos = 0;
    for( auto &letter : letters ) {
        letter.rotate_y = 0;
        letter.rotate_z = 0;
        letter.move_from.x = letter.letter->pos.x + get_rnd( -8000, 8000 ) * 1e-3f;
        letter.move_from.y = letter.letter->pos.y + get_rnd( 1000, 4000 ) * 1e-3f;
        letter.move_from.z = letter.letter->pos.z + get_rnd( -6000, -10000 ) * 1e-3f;
        letter.rotate_y = get_rnd( 300, -300 );
        letter.rotate_z = get_rnd( -200, 200 );
        letter.time = -letter.letter->pos.y * 0.5 + get_rnd( 1000, 3000 ) * 1e-3f;
        if( !speed || ypos != letter.letter->pos.y ) {
            speed = get_rnd( 800, 3000 ) * 1e-3f;
            ypos = letter.letter->pos.y;
        }
        letter.speed = speed;
    }
}

AnimHandler::AnimHandler( ANIMTYPE animType ) {
    this->animType = animType;
    this->scaleFactor = Vector2D( 1, 1 );
    running = false;
}

void AnimHandler::setAnimType( ANIMTYPE animType ) {
    this->animType = animType;
}

void AnimHandler::move( Vector2D shift ) {
    this->shift = shift;
}

void AnimHandler::moveXtoZero( float shiftX ) {
    this->shift.x = shiftX;
    if( animType == AT_LINEAR )
        this->shift.y = -shiftX;
    else if( animType == AT_QUAD )
        this->shift.y = -pow( shiftX, 2 );
    else
        this->shift.y = 0;
}

void AnimHandler::scale( Vector2D scale ) {
    this->scaleFactor = scale;
}

bool AnimHandler::isBackward() {
    return backwardTime != 0;
}

void AnimHandler::backward( float time ) {
    backwardTime = time;
    timer.start();
    running = true;
}

void AnimHandler::calcAxis( Vector2D pos )
{
    speed = pos.length();
    axis.x = pos.y;
    axis.y = -pos.x;
    if( speed )
        axis *= 1 / speed;
    else
        axis = Vector2D();
}

void AnimHandler::start() {
    backwardTime = 0;
    timer.start();
    running = true;
}

void AnimHandler::stop() {
    running = false;
}

Vector2D &AnimHandler::getAxis()
{
    return axis;
}

float AnimHandler::getSpeed()
{
    return speed;
}

float AnimHandler::getTime() {
    if( !running )
        return 0;
    if( backwardTime ) {
        if( backwardTime - timer.duration() < 0 ) {
            stop();
            return 0;
        }
        return backwardTime - timer.duration();
    }
    return timer.duration();
}

float AnimHandler::getValue() {
    if( !running )
        return 0;
    float x = scaleFactor.x * ( getTime() + shift.x );
    float y = 0;
    if( animType == AT_LINEAR )
        y = x + shift.y;
    else if( animType == AT_QUAD )
        y = pow( x, 2 ) + shift.y;
    return y * scaleFactor.y;
}

bool AnimHandler::isRunning() {
    return running;
}

ECV::ECV()
{

}

void ECV::Init()
{
    createWindow();

    defaultFont.Init( Roboto_Regular::font, 0.1f, 0.5f, 0.12f, 1 );

    program.CompileShaders( vertexShaderSource, fragmentShaderSource );
    minimalProgram.CompileShaders( vertexShaderSource, minimalFragmentShaderSource );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    glShadeModel( GL_SMOOTH );
    glAlphaFunc( GL_GEQUAL, 0.001f );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    glCullFace( GL_BACK );
    glFrontFace( GL_CW );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glMatrixMode( GL_TEXTURE );
    glLoadIdentity();
    gluOrtho2D( -1, 1, -1, 1 );
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_TEXTURE0 );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, texId );
    glEnable( GL_ALPHA_TEST );
    glEnable( GL_BLEND );
    glEnable( GL_MULTISAMPLE );

    Face cursor;
    cursor.push_back( Vector2D(  0.00f,  0.00f ));
    cursor.push_back( Vector2D(  0.03f,  0.00f ));
    cursor.push_back( Vector2D(  0.02f, -0.01f ));
    cursor.push_back( Vector2D(  0.05f, -0.04f ));
    cursor.push_back( Vector2D(  0.04f, -0.05f ));
    cursor.push_back( Vector2D(  0.01f, -0.02f ));
    cursor.push_back( Vector2D(  0.00f, -0.03f ));
    mCursor = TriangleGeneators::bevelExtrude( cursor, 0.02, 0.005, 1, true, true );

    mChars = std::make_shared<Letters3D>( defaultFont.genTextChars( Intro ));
    mChars->randomize();

    mChars->start_time = clock.duration();
}

void ECV::HandleEvents()
{
}

void ECV::Finish()
{
}

void ECV::keyPress( char key )
{
    switch( key ) {
    case 27:
        loop = false;
        break;
    case 32:
        mMove = !mMove;
        break;
    case 'W':
        mWire = !mWire;
        mTransp = mWire;
        break;
    case 'T':
        if( mWire ) {
            mWire = false;
            mTransp = true;
        } else {
            mTransp = !mTransp;
        }
        break;
    case 'P':
        mPause = !mPause;
        break;
    }
}

void ECV::keyRelease( char key )
{
    ( void ) key;
}

void ECV::mouseMove( float x, float y )
{
    ( void ) x; ( void ) y;
    float ow = 1.0f * winw / winh;
    xPos += x / winh;
    yPos -= y / winh;
    if( xPos < -ow )
        xPos = -ow;
    if( xPos > ow )
        xPos = ow;
    if( yPos < -1 )
        yPos = -1;
    if( yPos > 1 )
        yPos = 1;
}

void ECV::mousePress()
{
}

void ECV::mouseRelease()
{

}

void ECV::paint()
{
    glViewport( 0, 0, winw, winh );

    glClearColor( 0.05, 0.07, 0.2, 1 );
    glClearDepth( 1000.f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glDepthMask( GL_FALSE );
    glBegin( GL_QUADS );
    glColor3f( 0.00f, 0.10f, 0.20f ); glVertex2f( -1.0f, +0.0f );
    glColor3f( 0.00f, 0.08f, 0.15f ); glVertex2f( -1.0f, +1.0f );
    glColor3f( 0.00f, 0.08f, 0.15f ); glVertex2f( +1.0f, +1.0f );
    glColor3f( 0.00f, 0.10f, 0.20f ); glVertex2f( +1.0f, +0.0f );

    glColor3f( 0.20f, 0.20f, 0.30f ); glVertex2f( -1.0f, -1.0f );
    glColor3f( 0.00f, 0.10f, 0.20f ); glVertex2f( -1.0f, +0.0f );
    glColor3f( 0.00f, 0.10f, 0.20f ); glVertex2f( +1.0f, +0.0f );
    glColor3f( 0.20f, 0.20f, 0.30f ); glVertex2f( +1.0f, -1.0f );
    glEnd();

    auto curProgram = program;
    if( mWire ) {
        curProgram = minimalProgram;
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        glEnable( GL_BLEND );
        glDisable( GL_DEPTH_TEST );
        curProgram.useProgram();
        if( mTransp ) {
            glDisable( GL_CULL_FACE );
        } else {
            glEnable( GL_CULL_FACE );
        }
    } else {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        curProgram.useProgram();
        if( mTransp ) {
            glEnable( GL_BLEND );
            glDisable( GL_DEPTH_TEST );
            glDisable( GL_CULL_FACE );
        } else {
            glDisable( GL_BLEND );
            glEnable( GL_DEPTH_TEST );
            glEnable( GL_CULL_FACE );
        }
    }
    curProgram.enablePosition();
    curProgram.enableNormal( !mWire );

    glDepthMask( GL_TRUE );

    Matrix projection;
    Matrix view;
    Matrix model;

    curProgram.setUniform( "lightPos", 0, -0.3, -4, 0 );
    curProgram.setUniform( "ambient", 0.5f );
    curProgram.setUniform( "color", 0.60f, 0.65f, 0.15f, 1.0f );
    curProgram.setUniform( "specColor", 0.88f, 0.99f, 0.38f, 1.f );

    float ow = 1.f * winw / winh;

    projection.ortho( -1, 1, -ow, ow, 0.1f, 100.0f );
    curProgram.setUniform( "projection", projection );

    if( mWire ) {
        if( mTransp ) {
            curProgram.setUniform( "color", 0.6f, 0.9f, 1.0f, 0.05f );
        } else {
            curProgram.setUniform( "color", 0.6f, 0.9f, 1.0f, 0.1f );
        }
    } else {
        curProgram.setUniform( "lightPos", 0, -40, -40, 0 );
        curProgram.setUniform( "ambient", 0.2f );
        curProgram.setUniform( "color", 0.392f, 0.431f, 0.550f, 0.292f );
        curProgram.setUniform( "specColor", 0.784f, 0.863f, 0.980f, 1.f );
    }

    projection.toIdent();
    projection.perspective( 60.0f, 1.0f * winw / winh, 0.1f, 100.0f );

    curProgram.setUniform( "projection", projection );

    view.toIdent();
    view.translate( 0, -10, -20 );

    float duration = clock.duration();
    if( mPause )
        diffClock = duration - angle;
    angle = duration - diffClock;
    float camspeed = 1.0f;
    float camAngle = sin( 0.25f * angle * camspeed );
    view.rotate( 80 * camAngle, 0, 1, 0 );
    view.translate( 15 * sin( 0.3f * angle * camspeed ), 0, 0 );
    view.translate( 0, 0, -10 );
    curProgram.setUniform( "view", view );

    curProgram.setUniform( "lightPos", 0, -30, -30, 0 );
    projection.perspective( 60.0f, 1.0f * winw / winh, 0.1f, 100.0f );
    curProgram.setUniform( "projection", projection );
    model.toIdent();
    model.rotate( 5 * sin( 2.0 * angle ), 1, 0, 0 );
    curProgram.setUniform( "model", model );
    glDisable( GL_BLEND );
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    curProgram.setUniform( "color", 0.392f, 0.431f, 0.550f, 1.f );

    curProgram.setUniform( "lightPos", 3, -8, -65, 0 );
    projection.perspective( 40.0f, 1.0f * winw / winh, 0.1f, 100.0f );
    curProgram.setUniform( "projection", projection );
    view.toIdent();
    view.rotate( -45, 1, 0, 0 );
    curProgram.setUniform( "view", view );
    float difftime = angle - mChars->start_time;
    float mintime = -5;
    for( auto letter : mChars->letters ) {
        model.toIdent();
        float curtime = letter.time - difftime;
        if( mintime < curtime )
            mintime = curtime;
        if( curtime < 0 )
            curtime = 0;
        Vector3D pos( letter.letter->pos.x + ( letter.move_from.x - letter.letter->pos.x ) * curtime,
                      letter.letter->pos.y + ( letter.move_from.y - letter.letter->pos.y ) * curtime,
                      letter.letter->pos.z + ( letter.move_from.z - letter.letter->pos.z ) * curtime );
        model.translate( 0, 10.2 + 2 * angle, -26 );
        model.translate( pos + Vector3D( 0, 0, 0.3 * sin( letter.speed * angle + letter.letter->pos.x * 0.6 )));
        model.rotate( letter.rotate_y * curtime, 0, 1, 0 );
        model.rotate( letter.rotate_z * curtime, 0, 0, 1 );
        curProgram.setUniform( "model", model );
        curProgram.drawMesh( letter.letter->geometry );
    }
    curProgram.enablePosition( false );
    curProgram.enableNormal( false );
    std::this_thread::sleep_for( std::chrono::milliseconds( 2 ));
    curProgram.resetProgream();
}

ECV mainWindow;

void exitApp() {
    mainWindow.Finish();
}

int main( int argc, char* argv[] )
{
    atexit( exitApp );
    (void) argc, (void) argv;

    mainWindow.Run();

    return 0;
}


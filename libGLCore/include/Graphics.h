/* Copyright by János Klingl in 2023 */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <math.h>
#include <vector>
#ifndef __int64
#define __int64 long long
#endif
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#include <GL/glew.h>
#include <GL/gl.h>

/* If PI is not defined by the system we declare it */

#ifndef M_PI
#define M_PI 3.141592653589793238462643
#endif

#ifndef uint
typedef unsigned int uint;
#endif

using namespace std;

/* Structure to store 2D vector and provides helper functions for it */

struct Vector2D
{
    float x;
    float y;
    Vector2D();
    Vector2D( float x, float y );
    Vector2D( const Vector2D &other );
    Vector2D &operator = ( const Vector2D &other );
    void operator *= ( float scale );
    void operator -= ( const Vector2D &translate );
    void operator += ( const Vector2D &translate );
    float length() const;
    void normalize();
    Vector2D normalized() const;
    static float dot( const Vector2D &v1, const Vector2D &v2 );
};

/* Additional operators for 2D vectors */

Vector2D operator - ( const Vector2D v );
Vector2D operator - ( const Vector2D v1, const Vector2D v2 );
Vector2D operator + ( const Vector2D v1, const Vector2D v2 );
Vector2D operator * ( const Vector2D v1, const float scale );
bool operator == ( const Vector2D v1, const Vector2D v2 );

struct Quaternion;

/* Structure declaration and additional functions for 3D vectors */

struct Vector3D
{
    float x;
    float y;
    float z;
    Vector3D();
    Vector3D( float x, float y, float z );
    Vector3D( const Vector3D &other );
    Vector3D &operator = ( const Vector3D &other );
    Vector3D( const Vector2D &other );
    void operator *= ( float scale );
    void operator *= ( const Vector3D scale );
    void operator -= ( const Vector3D translate );
    void operator += ( const Vector3D translate );
    float length() const;
    bool isEmpty() const;
    Vector3D normalized() const;
    Vector2D toVector2D() const;
    void rotate( const Quaternion &quaternion );
    static float dot( const Vector3D &v1, const Vector3D &v2 );
    static Vector3D cross( const Vector3D &v1, const Vector3D &v2 );
};

/* Additional operators for 3D vectors */

Vector3D operator - ( const Vector3D &v );
Vector3D operator - ( const Vector3D &v1, const Vector3D &v2 );
Vector3D operator + ( const Vector3D &v1, const Vector3D &v2 );
Vector3D operator * ( const Vector3D &point, float scale );

/* Quaternion structure declaration and helper functions */

struct Quaternion
{
    float w;
    float x;
    float y;
    float z;
    Quaternion();
    Quaternion( float w, float x, float y, float z );
    Quaternion( const Quaternion &other );
    Quaternion &operator = ( const Quaternion &other );
    static Quaternion AxisRotation( float angle, Vector3D axis );
};

/* 4x4 Matrix structure declaration and helper functions */

struct Matrix
{
    float m[4][4];
    Matrix();
    Matrix( const Matrix &other );
    Matrix &operator = ( const Matrix &other );
    void operator *= ( const Matrix &other );
    operator float *() const;
    float *getFloatPtr() const;
    void toIdent();
    void rotate( float angle, float x, float y, float z );
    void rotate( float angle, const Vector3D &axis );
    void translate( float x, float y, float z );
    void translate( const Vector3D &vec );
    void scale( float scale );
    void scale( float x, float y, float z );
    void scale( const Vector3D &scale );
    void ortho( const float bottom, const float top, const float left, const float right, const float nearplane, const float farplane );
    void perspective( const float fov, const float aspect, const float nearplane, const float farplane );
};

inline float angleToDist( float angle ) {
    //float tangent = tan( sqrt( 2.4 ) * sqrt( sin( angle * angle / 2.1 )));
    return 1 + 0.671497735 * angle * angle ;
}

/* 4x4 Matrix operations */

Matrix operator * ( const Matrix mat1, const Matrix mat2 );
Vector3D operator * ( const Matrix matrix, const Vector3D point );

/* Plane declaration as vector of 2D Vectors and additional functions */

struct Plane : public vector<Vector2D>
{
                                            Plane();
                                            Plane( vector<Vector2D> &points );
    const Plane                             reversed() const;
    void                                    shift( Vector2D shift );
};

/* 3D Mesh structure declaration with storage and manipulation */

struct Mesh
{
    vector<Vector3D>                        mVertices;
    vector<Vector3D>                        mNormals;
    vector<uint>                            mIndices;
    int                                     addVertex( const Vector3D pos, const Vector3D norm );
    Mesh                                   *flip();
    Mesh                                   *shift( const Vector3D shift );
    Mesh                                   *rotate( const float angle, const Vector3D axis );
    Mesh                                   *scale( float scale );
    Mesh                                   *transform( const Matrix matrix );
    void                                    operator += ( const Mesh &other );
    Mesh                                    optimized();
    void                                    clear();
};

#endif // GRAPHICS_H

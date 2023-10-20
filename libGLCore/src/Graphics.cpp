#include "Graphics.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <memory.h>
#include <iostream>
#include <math.h>
#include <map>

using namespace std;

const double deg2rad = M_PI / 180.0;

Vector2D::Vector2D() : x( 0.f ), y( 0.f ) {
}

Vector2D::Vector2D( float x, float y ) : x( x ), y( y ) {
}

Vector2D::Vector2D( const Vector2D &other ) {
    *this = other;
}

Vector2D &Vector2D::operator = ( const Vector2D &other ) {
    x = other.x;
    y = other.y;
    return *this;
}

void Vector2D::operator *= ( float scale ) {
    x *= scale;
    y *= scale;
}

void Vector2D::operator -= ( const Vector2D &translate ) {
    x -= translate.x;
    y -= translate.y;
}

void Vector2D::operator += ( const Vector2D &translate ) {
    x += translate.x;
    y += translate.y;
}

float Vector2D::length() const {
    return sqrtf( x * x + y * y );
}

void Vector2D::normalize() {
    const float length_ = length();
    if( length_ )
        *this *= 1.f / length_;
}

Vector2D Vector2D::normalized() const {
    const float length_ = length();
    if( length_ )
        return Vector2D(*this) * ( 1.f / length_ );
    return Vector2D();
}

float Vector2D::dot( const Vector2D &v1, const Vector2D &v2 ) {
    return v1.x * v2.x + v1.y * v2.y;
}

Vector2D operator - ( const Vector2D v ) {
    return Vector2D( -v.x, -v.y );
}

Vector2D operator - ( const Vector2D v1, const Vector2D v2 ) {
    Vector2D result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    return result;
}

Vector2D operator + ( const Vector2D v1, const Vector2D v2 ) {
    Vector2D result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    return result;
}

Vector2D operator * ( const Vector2D v1, const float scale ) {
    Vector2D result;
    result.x = v1.x * scale ;
    result.y = v1.y * scale;
    return result;
}

bool operator == ( const Vector2D v1, const Vector2D v2 ) {
    return v1.x == v2.x && v1.y == v2.y;
}

Vector3D::Vector3D() : x( 0.f ), y( 0.f ), z( 0.f ) {
}

Vector3D::Vector3D( float x, float y, float z ) : x( x ), y( y ), z( z ) {
}

Vector3D::Vector3D(const Vector3D &other ) {
    *this = other;
}

Vector3D &Vector3D::operator = ( const Vector3D &other ) {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

Vector3D::Vector3D( const Vector2D &other )
{
    x = other.x;
    y = other.y;
    z = 0;
}

void Vector3D::operator *= ( float scale ) {
    x *= scale;
    y *= scale;
    z *= scale;
}

void Vector3D::operator *=( const Vector3D scale )
{
    x *= scale.x;
    y *= scale.y;
    z *= scale.z;
}

void Vector3D::operator -= ( const Vector3D translate ) {
    x -= translate.x;
    y -= translate.y;
    z -= translate.z;
}

void Vector3D::operator += ( const Vector3D translate ) {
    x += translate.x;
    y += translate.y;
    z += translate.z;
}

float Vector3D::length() const {
    return sqrtf( x * x + y * y + z * z );
}

bool Vector3D::isEmpty() const
{
    return ( 0 == x ) && ( 0 == y ) && ( 0 == z );
}

Vector3D Vector3D::normalized() const {
    const float length_ = length();
        return Vector3D(*this) * ( 1.f / length_ );
    return Vector3D();
}

Vector2D Vector3D::toVector2D() const
{
    return Vector2D( x, y );
}

void Vector3D::rotate( const Quaternion &quaternion )
{
    Vector3D u( quaternion.x, quaternion.y, quaternion.z );
    float s = quaternion.w;

    Vector3D result = u * 2.0f * dot( u, *this) +
          *this * ( s * s - dot( u, u )) +
          cross(u, *this) * 2.0f * s;
    *this = result;
}

float Vector3D::dot( const Vector3D &v1, const Vector3D &v2 ) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3D Vector3D::cross( const Vector3D &v1, const Vector3D &v2 ) {
    Vector3D result;
    result.x = v1.y * v2.z - v1.z * v2.y;
    result.y = v1.z * v2.x - v1.x * v2.z;
    result.z = v1.x * v2.y - v1.y * v2.z;
    return result;
}

Vector3D operator - ( const Vector3D &v ) {
    return Vector3D( -v.x, -v.y, -v.z );
}

Vector3D operator - ( const Vector3D &v1, const Vector3D &v2 ) {
    return Vector3D( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z );
}

Vector3D operator + ( const Vector3D &v1, const Vector3D &v2 ) {
    return Vector3D( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z );
}

Vector3D operator * ( const Vector3D &point, float scale ) {
    Vector3D out( point );
    out *= scale;
    return out;
}

Quaternion::Quaternion() : w( 1.f ), x( 0.f ), y( 0.f ), z( 0.f ) {
}

Quaternion::Quaternion( float w, float x, float y, float z ) : w( w ), x( x ), y( y ), z( z ) {
}

Quaternion::Quaternion( const Quaternion &other ) {
    *this = other;
}

Quaternion &Quaternion::operator = ( const Quaternion &other ) {
    w = other.w;
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

Quaternion Quaternion::AxisRotation(float angle, Vector3D axis)
{
    const float halfangle = 0.5f * deg2rad * angle;
    const float sinhalfangle = sin( halfangle );
    return Quaternion( cos( halfangle ), axis.x * sinhalfangle, axis.y * sinhalfangle, axis.z * sinhalfangle );
}

Matrix::Matrix() {
    toIdent();
}

Matrix::Matrix( const Matrix &other ) {
    *this = other;
}

Matrix &Matrix::operator = ( const Matrix &other ) {
    memcpy( m, other.m, sizeof( m ));
    return *this;
}

void Matrix::operator *= ( const Matrix &other ) {
    Matrix src = *this;
    for( int i = 0; i < 4; ++i ) {
        for( int j = 0; j < 4; ++j ) {
            float rv = 0;
            for( int r = 0; r < 4; ++r ) {
                rv += src.m[i][r] * other.m[r][j];
            }
            m[i][j] = rv;
        }
    }
}

float *Matrix::getFloatPtr() const {
    return ( float* ) m;
}

Matrix::operator float *() const {
    return getFloatPtr();
}

void Matrix::toIdent() {
    for( int i = 0; i < 4; ++i ) {
        for( int j = 0; j < 4; ++j ) {
            if( i == j ) {
                m[i][j] = 1;
            } else {
                m[i][j] = 0;
            }
        }
    }
}

void Matrix::rotate( float angle, float x, float y, float z ) {
    Matrix rot;
    const float halfangle = 0.5f * deg2rad * angle;
    const float sinhalfangle = sin( halfangle );
    float qx = x * sinhalfangle;
    float qy = y * sinhalfangle;
    float qz = z * sinhalfangle;
    float qw = cos( halfangle );
    rot.m[0][0] = 1 - 2 * qy * qy - 2 * qz * qz;
    rot.m[1][0] = 2 * qx * qy - 2 * qz * qw;
    rot.m[2][0] = 2 * qx * qz + 2 * qy * qw;
    rot.m[0][1] = 2 * qx * qy + 2 * qz * qw;
    rot.m[1][1] = 1 - 2 * qx * qx - 2 * qz * qz;
    rot.m[2][1] = 2 * qy * qz - 2 * qx * qw;
    rot.m[0][2] = 2 * qx * qz - 2 * qy * qw;
    rot.m[1][2] = 2 * qy * qz + 2 * qx * qw;
    rot.m[2][2] = 1 - 2 * qx * qx - 2 * qy * qy;
    *this = rot * *this;
}

void Matrix::rotate( float angle, const Vector3D &axis ) {
    rotate( angle, axis.x, axis.y, axis.z );
}

void Matrix::translate( float x, float y, float z ) {
    Matrix translate;
    translate.m[3][0] = x;
    translate.m[3][1] = y;
    translate.m[3][2] = z;
    *this = translate * *this;
}

void Matrix::translate( const Vector3D &vec ) {
    translate( vec.x, vec.y, vec.z );
}

void Matrix::scale( float scale ) {
    this->scale( scale, scale, scale );
}

void Matrix::scale(float x, float y, float z)
{
    Matrix scl;
    scl.m[0][0] = x;
    scl.m[1][1] = y;
    scl.m[2][2] = z;
    *this = scl * *this;
}

void Matrix::scale( const Vector3D &scale ) {
    this->scale( scale.x, scale.y, scale.z );
}

void Matrix::ortho( const float bottom, const float top, const float left, const float right, const float nearplane, const float farplane )
{
    toIdent();
    m[ 0 ][ 0 ] =  2 / ( right - left );
    m[ 1 ][ 1 ] =  2 / ( top - bottom );
    m[ 2 ][ 2 ] = -2 / ( farplane - nearplane );

    m[ 3 ][ 0 ] = -( right + left ) / ( right - left );
    m[ 3 ][ 1 ] = -( top + bottom ) / ( top- bottom );
    m[ 3 ][ 2 ] = -( farplane + nearplane ) / ( farplane - nearplane );
}

void Matrix::perspective( const float fov, const float aspect, const float nearplane, const float farplane ) {
    double yScale = 1.0 / tan( deg2rad * fov * 0.5f );
    double xScale = yScale / aspect;
    double nearmfar = nearplane - farplane;
    toIdent();
    m[ 0 ][ 0 ] = xScale;
    m[ 1 ][ 1 ] = yScale;
    m[ 2 ][ 2 ] = ( farplane + nearplane ) / nearmfar;
    m[ 2 ][ 3 ] = -1;
    m[ 3 ][ 2 ] = 2.f * farplane * nearplane / nearmfar;
    m[ 3 ][ 3 ] = 0;
}

Matrix operator * ( const Matrix mat1, const Matrix mat2 ) {
    Matrix result = mat1;
    result *= mat2;
    return result;
}

Vector3D operator * ( const Matrix matrix, const Vector3D point ) {
    Vector3D result;
    result.x = matrix.m[0][0] * point.x + matrix.m[1][0] * point.y + matrix.m[2][0] * point.z + matrix.m[3][0];
    result.y = matrix.m[0][1] * point.x + matrix.m[1][1] * point.y + matrix.m[2][1] * point.z + matrix.m[3][1];
    result.z = matrix.m[0][2] * point.x + matrix.m[1][2] * point.y + matrix.m[2][2] * point.z + matrix.m[3][2];
    return result;
}

Plane::Plane() { }

Plane::Plane(vector<Vector2D> &points) : vector<Vector2D>( points ) {}

const Plane Plane::reversed() const {
    Plane back;
    for( auto &point : *this ) {
        back.insert( back.begin(), point );
    }
    return back;
}

void Plane::shift( Vector2D shift ) {
    for( auto &point : *this ) {
        point += shift;
    }
}

int Mesh::addVertex( const Vector3D pos, const Vector3D norm )
{
    size_t vSize = mVertices.size();
    if( mVertices.size() != mNormals.size() )
        return vSize;
    auto pv = mVertices.begin();
    auto pn = mNormals.begin();
    for( size_t i = 0; i < vSize; ++i ) {
        if( fabs( pv->x - pos.x ) < 1e-4f &&
            fabs( pv->y - pos.y ) < 1e-4f &&
            fabs( pv->z - pos.z ) < 1e-4f &&
            fabs( pn->x - norm.x ) < 1e-6f &&
            fabs( pn->y - norm.y ) < 1e-6f &&
            fabs( pn->z - norm.z ) < 1e-6f)
            return i;
    }
    mVertices.push_back( pos );
    mNormals.push_back( norm );
    return vSize;
}

Mesh *Mesh::flip() {
    vector<uint> indices = mIndices;
    mIndices.clear();
    for( auto &index : indices )
        mIndices.insert( mIndices.begin(), index );
    for( auto &normal : mNormals )
        normal *= -1;
    return this;
}

Mesh *Mesh::shift( const Vector3D shift ) {
    for( auto &point : mVertices ) {
        point += shift;
    }
    return this;
}

Mesh *Mesh::rotate( const float angle, const Vector3D axis ) {
    Matrix matrix;
    matrix.rotate( angle, axis );
    transform( matrix );
    return this;
}

Mesh *Mesh::scale(float scale)
{
    for( auto &point : mVertices ) {
        point *= scale;
    }
    return this;
}

Mesh *Mesh::transform( const Matrix matrix ) {
    for( auto &point : mVertices ) {
        point = matrix * point;
    }
    for( auto &normal : mNormals ) {
        normal = matrix * normal;
    }
    return this;
}

void Mesh::operator += ( const Mesh &other ) {
    int startIDX = mVertices.size();
    mVertices.insert( mVertices.end(), other.mVertices.begin(), other.mVertices.end()) ;
    mNormals.insert( mNormals.end(), other.mNormals.begin(), other.mNormals.end()) ;
    for( auto index : other.mIndices ) {
        mIndices.push_back( startIDX + index );
    }
}

Mesh Mesh::optimized()
{
    std::multimap<float, uint>  distmap;
    std::vector<uint>           points;
    std::map<uint,uint>         pointmap;
    std::map<uint,uint>         remapmap;
    int id = 0;
    for( const auto &point : mVertices ) {
        distmap.insert( std::pair<float, uint>( point.x + point.y + point.z, id++ ));
    }
    const auto end = distmap.end();
    Mesh newMesh;
    int newid = 0;
    for( auto i1 = distmap.begin(); i1 != end; ++i1 ) {
        const int id1 = i1->second;
        const auto &point = mVertices[ id1 ];
        const auto &normal = mNormals[ id1 ];
        bool newPoint = false;
        for( auto i2 = distmap.find( i1->first );;++i2 ) {
            const int id2 = i2->second;
            if( i2 == i1 || i2 == end ) {
                newPoint = true;
                break;
            }
            const auto &point2 = mVertices[ i2->second ];
            const auto &normal2 = mNormals[ i2->second ];
            if( 0 == ( i1->first - i2->first )) {
                if(( point - point2 ).isEmpty() ) {
                    if(( normal - normal2 ).isEmpty() ) {
                        newPoint = false;
                        points.push_back( id2 );
                        pointmap.insert( std::pair<uint,uint>( id1, id2 ));
                        break;
                    }
                }
                newPoint = true;
            } else if( newPoint ) {
                break;
            }
        }
        if( newPoint ) {
            newMesh.mVertices.push_back( point );
            newMesh.mNormals.push_back( normal );
            points.push_back( id1 );
            remapmap.insert( std::pair<uint,uint>( id1, newid++ ));
        }
    }
    for( auto id : mIndices ) {
        int xid = id;
        if( pointmap.find( id ) != pointmap.end() ) {
            xid = pointmap[ id ];
        }
        newMesh.mIndices.push_back( remapmap[ xid ]);
    }
    return newMesh;
}

void Mesh::clear() {
    mVertices.clear();
    mNormals.clear();
    mIndices.clear();
}

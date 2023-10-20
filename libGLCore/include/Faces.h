/* Copyright by János Klingl in 2023 */

#ifndef FACES_H
#define FACES_H

#include <Graphics.h>

using namespace std;

/* This function is used to check the intersection between two lines */

struct Line
{
    static bool intersect( const Vector2D &pa1, const Vector2D &pa2, const Vector2D &pb1, const Vector2D &pb2, float &pos );
};

typedef Plane Face;

/* Declare Faces and vector of Face */

struct Faces : public vector<Face>
{
                                            Faces();
                                            Faces( const Face &polygon );
                                            Faces( vector<Face> &faces);
    void                                    shift( Vector2D shift );
};

/* Bounding box structure for extending Face structure */

struct BBoxFace : public Face
{
    float                                   minx;
    float                                   maxx;
    float                                   miny;
    float                                   maxy;
                                            BBoxFace();
                                            BBoxFace( const BBoxFace &src );
                                            BBoxFace( const Face &src );
                                            BBoxFace &operator = ( const BBoxFace &src );
    void                                    addPoint( const Vector2D pnt );
    int                                     checkRelation( const BBoxFace &other );
    void                                    grow( const BBoxFace &other );
    void                                    setBox( const BBoxFace &other );
};

/* Tangent declaration */

struct Tangent
{
    float                                   pangle;
    float                                   nangle;
    float                                   distance;
};

/* Tangents declaration */

struct Tangents : public vector<Tangent>
{
};

struct TriangleGeneators;

/* 2D plane generators */

struct FaceGeneators
{
    friend struct TriangleGeneators;
    static Face                             grow( const Face polygon, float width );
    static Face                             roundedRect( float height, float width, float radius, int step );
    static Face                             drill( const Face polygon, const Faces holes );
    static Tangents                         generateTangents( const Face polygon );
    static bool                             checkOrientation( const Face &polygon );
};

extern float degToRad;
extern float radToDeg;
extern float twoPI;

#endif // FACES_H

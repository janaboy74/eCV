/* Copyright by János Klingl in 2023 */

#ifndef TRIANGLES_H
#define TRIANGLES_H

#include "Faces.h"
#include <vector>

typedef Mesh Triangles;

/* This structure provides generators for 3D objects */

struct TriangleGeneators
{
    static float                            auto_smooth_angle;
    static Triangles                        bevelEdge( const Face polygon, float height, float depth, float radius, int slices, bool smooth );
    static Triangles                        bevelExtrude( const Face polygon, float height, float radius, int slices, bool smooth, bool cap = true );
    static Triangles                        bevelExtrude( const Face polygon, Faces holes, float height, float radius, int slices, bool smooth, bool cap = true );
    static Triangles                        revolution( const Faces polygons, float radius, float angleStep, bool smooth, bool close );
    static void                             bevel( Triangles &triangles, const Face polygon, float depth, float radius, float slices, bool flip, bool in );
    static void                             cylinder( Triangles &triangles, const Face polygon, float depth, bool smooth, bool cw );
    static void                             fillEdge( Triangles &triangles, const Face polygon, float width, float depth, bool cw );
    static void                             fillFace( Triangles &triangles, const Face polygon, float depth, bool bottom );
};

#endif // TRIANGLES_H

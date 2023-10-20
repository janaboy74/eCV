#include "Triangles.h"

float TriangleGeneators::auto_smooth_angle = 0.35;

Triangles TriangleGeneators::bevelEdge( Face polygon, float height, float depth, float radius, int slices, bool smooth )
{
    Triangles triangles;
    Face outside = FaceGeneators::grow( polygon, depth );
    bevel( triangles, polygon, height * 0.5, -radius, slices, false, false );
    bevel( triangles, polygon, -height * 0.5, -radius, slices, true, false );
    cylinder( triangles, polygon, height * 0.5, smooth, false );
    fillEdge( triangles, polygon, depth, -height * 0.25, false );
    fillEdge( triangles, polygon, depth, +height * 0.25, true );
    bevel( triangles, outside, height * 0.5, radius, slices, true, true );
    bevel( triangles, outside, -height * 0.5, radius, slices, false, true );
    cylinder( triangles, FaceGeneators::grow( outside, radius ), height * 0.5 - radius, smooth, true );
    return triangles;
}

Triangles TriangleGeneators::bevelExtrude( const Face polygon, float height, float radius, int slices, bool smooth, bool cap )
{
    Triangles triangles;
    if( cap ) {
        fillFace( triangles, polygon, height * 0.5, false );
        fillFace( triangles, polygon, height * 0.5, true );
    }
    bevel( triangles, polygon, height, radius, slices, true, true );
    bevel( triangles, polygon, -height, radius, slices, false, true );
    cylinder( triangles, FaceGeneators::grow( polygon, radius ), height - 2 * radius, smooth, true );
    return triangles;
}

Triangles TriangleGeneators::bevelExtrude( const Face polygon, const Faces holes, float height, float radius, int slices, bool smooth, bool cap )
{
    const Face face = FaceGeneators::drill( polygon, holes );
    Triangles triangles = bevelExtrude( polygon, height, radius, slices, smooth, false );
    if( cap ) {
        fillFace( triangles, face, height * 0.5, false );
        fillFace( triangles, face, height * 0.5, true );
    }
    for( auto &hole : holes ) {
        cylinder( triangles, FaceGeneators::grow( hole, -radius ), height - 2 * radius, smooth, false );
        bevel( triangles, hole, -height, -radius, slices, true, false );
        bevel( triangles, hole, height, -radius, slices, false, false );
    }
    return triangles;
}

Triangles TriangleGeneators::revolution( const Faces polygons, float radius, float angleStep, bool smooth, bool close )
{
    Triangles triangles;
    for( auto polygon : polygons ) {
        bool flip = FaceGeneators::checkOrientation( polygon );
        if( polygon.size() < 3 )
            continue;
        Face normals;
        if( smooth ) {
            Vector2D v1( *polygon.begin() - *polygon.rbegin() );
            auto p2 = polygon.begin() + 1;
            float prevangle = atan2( -v1.y, v1.x );
            for( auto p1 : polygon ) {
                Vector2D v1( *p2 - p1 );
                ++p2;
                if( p2 == polygon.end() )
                    p2 = polygon.begin();
                float angle = atan2( -v1.y, v1.x );
                if( angle < 0 )
                    angle += 2 * M_PI;
                float normalAngle = ( angle - prevangle  ) * 0.5 + prevangle + ( flip ? M_PI : 0 );
                normals.push_back( Vector2D( sin( normalAngle ), cos( normalAngle )));
                prevangle = angle;
            }
            const int pointCount = polygon.size();
            const int max = pointCount - ( close ? 0 : 1 );
            float fullangle = 360 / angleStep;
            for( int i = 0; i <= fullangle; ++i ) {
                int startIDX = triangles.mVertices.size();
                float angleRad = i * angleStep * degToRad;
                for( auto point : polygon ) {
                    float dist = radius + point.x;
                    triangles.mVertices.push_back( Vector3D( dist * sin( angleRad ), dist * cos( angleRad ), point.y ));
                }
                for( auto normalVector : normals ) {
                    float dist = normalVector.x;
                    triangles.mNormals.push_back( Vector3D( dist * sin( angleRad ), dist * cos( angleRad ), normalVector.y));
                }
                if( i ) {
                    for( int j = 0; j < max; ++j ) {
                        int jp0 = j % pointCount;
                        int jp1 = ( j + 1 ) % pointCount;
                        if( flip ) {
                            triangles.mIndices.push_back( startIDX - pointCount + jp0 );
                            triangles.mIndices.push_back( startIDX + jp0 );
                            triangles.mIndices.push_back( startIDX + jp1 );
                            triangles.mIndices.push_back( startIDX + jp1 );
                            triangles.mIndices.push_back( startIDX - pointCount + jp1 );
                            triangles.mIndices.push_back( startIDX - pointCount + jp0 );
                        } else {
                            triangles.mIndices.push_back( startIDX - pointCount + jp0 );
                            triangles.mIndices.push_back( startIDX - pointCount + jp1 );
                            triangles.mIndices.push_back( startIDX + jp1 );
                            triangles.mIndices.push_back( startIDX + jp1 );
                            triangles.mIndices.push_back( startIDX + jp0 );
                            triangles.mIndices.push_back( startIDX - pointCount + jp0 );
                        }
                    }
                }
            }
        } else {
            auto p2 = polygon.begin() + 1;
            for( auto p1 : polygon ) {
                Vector2D v1( *p2 - p1 );
                ++p2;
                if( p2 == polygon.end() )
                    p2 = polygon.begin();
                float curangle = atan2( -v1.y, v1.x ) + ( flip ? M_PI : 0 );
                normals.push_back( Vector2D( sin( curangle ), cos( curangle )));
            }
            float fullangle = 360 / angleStep;
            for( int i = 0; i <= fullangle; ++i ) {
                int startIDX = triangles.mVertices.size();
                float angleRad = i * angleStep * degToRad;
                for( auto point : polygon ) {
                    float dist = radius + point.x;
                    triangles.mVertices.push_back( Vector3D( dist * sin( angleRad ), dist * cos( angleRad ), point.y ));
                    triangles.mVertices.push_back( *triangles.mVertices.rbegin() );
                }
                triangles.mVertices.push_back( triangles.mVertices.at( startIDX ));
                triangles.mVertices.erase( triangles.mVertices.begin() + startIDX );
                for( auto normalVector : normals ) {
                    float dist = normalVector.x;
                    triangles.mNormals.push_back( Vector3D( dist * sin( angleRad ), dist * cos( angleRad ), normalVector.y ));
                    triangles.mNormals.push_back( *triangles.mNormals.rbegin() );
                }
                int pointCount = triangles.mVertices.size() - startIDX;
                const int max = pointCount - ( close ? 0 : 2 );
                if( i ) {
                    for( int j = 0; j < max; j += 2 ) {
                        int jp0 = j % pointCount;
                        int jp1 = ( j + 1 ) % pointCount;
                        if( flip ) {
                            triangles.mIndices.push_back( startIDX - pointCount + jp0 );
                            triangles.mIndices.push_back( startIDX + jp0 );
                            triangles.mIndices.push_back( startIDX + jp1 );
                            triangles.mIndices.push_back( startIDX + jp1 );
                            triangles.mIndices.push_back( startIDX - pointCount + jp1 );
                            triangles.mIndices.push_back( startIDX - pointCount + jp0 );
                        } else {
                            triangles.mIndices.push_back( startIDX - pointCount + jp0 );
                            triangles.mIndices.push_back( startIDX - pointCount + jp1 );
                            triangles.mIndices.push_back( startIDX + jp1 );
                            triangles.mIndices.push_back( startIDX + jp1 );
                            triangles.mIndices.push_back( startIDX + jp0 );
                            triangles.mIndices.push_back( startIDX - pointCount + jp0 );
                        }
                    }
                }
            }
        }
    }
    return triangles;
}

void TriangleGeneators::bevel( Triangles &triangles, Face polygon, float depth, float radius, float slices, bool flip, bool in )
{
    Tangents tangents = FaceGeneators::generateTangents( polygon );

    const int pointCount = polygon.size();
    Face polygon1 = polygon;
    float up1 = 0;
    for( int s = 1 ; s <= slices; ++s ) {
        float up2 = 1 - cos( 0.5 * M_PI * s / slices );
        float s1;
        float u1;
        float s2;
        float u2;
        if( slices > 1 ) {
            s1 = sin( 0.5 * M_PI * ( s - 1 ) / slices );
            u1 = cos( 0.5 * M_PI * ( s - 1 ) / slices );
            s2 = sin( 0.5 * M_PI * ( s ) / slices );
            u2 = cos( 0.5 * M_PI * ( s ) / slices );
        } else {
            s1 = sin( 0.5 * M_PI * ( s - 0.5 ) / slices );
            u1 = cos( 0.5 * M_PI * ( s - 0.5 ) / slices );
            s2 = s1;
            u2 = u1;
        }
        Face polygon2 = FaceGeneators::grow( polygon, radius * sin( 0.5 * M_PI * s / slices ));
        for( size_t i = 0 ; i < polygon2.size(); ++i ) {
            const Tangent tangent1 = tangents.at( i );
            const Tangent tangent2 = tangents.at(( i + 1 ) % pointCount );
            const Vector2D p11 =  polygon1.at( i );
            const Vector2D p12 =  polygon1.at(( i + 1 ) % pointCount );
            const Vector2D p21 =  polygon2.at( i % pointCount );
            const Vector2D p22 =  polygon2.at(( i + 1 ) % pointCount );
            float angle1 = fabs( tangent1.pangle - tangent1.nangle );
            if( angle1 < M_PI * auto_smooth_angle )
                angle1 = ( tangent1.pangle + tangent1.nangle ) * 0.5;
            else
                angle1 = tangent1.nangle;
            float angle2 = fabs( tangent2.pangle - tangent2.nangle );
            if( angle2 < M_PI * auto_smooth_angle )
                angle2 = ( tangent2.pangle + tangent2.nangle ) * 0.5;
            else
                angle2 = tangent2.pangle;
            const Vector3D n1( sin( angle1 ),cos( angle1 ), 0 );
            const Vector3D n2( sin( angle2 ),cos( angle2 ), 0 );
            Vector3D normal11 = ( n1 * s1 + Vector3D( 0, 0, u1 ) * ( flip ? 1 : -1 )).normalized();
            Vector3D normal21 = ( n1 * s2 + Vector3D( 0, 0, u2 ) * ( flip ? 1 : -1 )).normalized();
            Vector3D normal12 = ( n2 * s1 + Vector3D( 0, 0, u1 ) * ( flip ? 1 : -1 )).normalized();
            Vector3D normal22 = ( n2 * s2 + Vector3D( 0, 0, u2 ) * ( flip ? 1 : -1 )).normalized();

            const int id0 = triangles.addVertex( Vector3D( p11.x, p11.y, depth * 0.5 + radius * up1 * ( flip ? -1 : 1 )), normal11 * ( in ? 1 : -1 ));
            const int id1 = triangles.addVertex( Vector3D( p12.x, p12.y, depth * 0.5 + radius * up1 * ( flip ? -1 : 1 )), normal12 * ( in ? 1 : -1 ));
            const int id2 = triangles.addVertex( Vector3D( p22.x, p22.y, depth * 0.5 + radius * up2 * ( flip ? -1 : 1 )), normal22 * ( in ? 1 : -1 ));
            const int id3 = triangles.addVertex( Vector3D( p21.x, p21.y, depth * 0.5 + radius * up2 * ( flip ? -1 : 1 )), normal21 * ( in ? 1 : -1 ));

            if( flip ) {
                triangles.mIndices.push_back( id2 );
                triangles.mIndices.push_back( id1 );
                triangles.mIndices.push_back( id0 );
                triangles.mIndices.push_back( id0 );
                triangles.mIndices.push_back( id3 );
                triangles.mIndices.push_back( id2 );
            } else {
                triangles.mIndices.push_back( id2 );
                triangles.mIndices.push_back( id3 );
                triangles.mIndices.push_back( id0 );
                triangles.mIndices.push_back( id0 );
                triangles.mIndices.push_back( id1 );
                triangles.mIndices.push_back( id2 );
            }
        }
        polygon1 = polygon2;
        up1 = up2;
    }
}

void TriangleGeneators::cylinder( Triangles &triangles, Face polygon, float depth, bool smooth, bool cw )
{
    (void) smooth;
    Tangents tangents = FaceGeneators::generateTangents( polygon );
    const int pointCount = polygon.size();
    for( size_t i = 0 ; i < polygon.size(); ++i ) {
        const Tangent tangent1 = tangents.at( i );
        const Tangent tangent2 = tangents.at(( i + 1 ) % pointCount );
        const Vector2D p1 =  polygon.at( i  );
        const Vector2D p2 =  polygon.at(( i + 1 ) % pointCount );
        float angle1 = fabs( tangent1.pangle - tangent1.nangle );
        if( angle1 < M_PI * auto_smooth_angle )
            angle1 = ( tangent1.pangle + tangent1.nangle ) * 0.5;
        else
            angle1 = tangent1.nangle;
        float angle2 = fabs( tangent2.pangle - tangent2.nangle );
        if( angle2 < M_PI * auto_smooth_angle )
            angle2 = ( tangent2.pangle + tangent2.nangle ) * 0.5;
        else
            angle2 = tangent2.pangle;
        const Vector3D n1( sin( angle1 ),cos( angle1 ), 0 );
        const Vector3D n2( sin( angle2 ),cos( angle2 ), 0 );
        const int id0 = triangles.addVertex( Vector3D( p1.x, p1.y,  depth * 0.5 ), cw ? n1 : -n1 );
        const int id1 = triangles.addVertex( Vector3D( p2.x, p2.y,  depth * 0.5 ), cw ? n2 : -n2 );
        const int id2 = triangles.addVertex( Vector3D( p2.x, p2.y, -depth * 0.5 ), cw ? n2 : -n2 );
        const int id3 = triangles.addVertex( Vector3D( p1.x, p1.y, -depth * 0.5 ), cw ? n1 : -n1 );
        if( cw ) {
            triangles.mIndices.push_back( id2 );
            triangles.mIndices.push_back( id1 );
            triangles.mIndices.push_back( id0 );
            triangles.mIndices.push_back( id0 );
            triangles.mIndices.push_back( id3 );
            triangles.mIndices.push_back( id2 );
        } else {
            triangles.mIndices.push_back( id2 );
            triangles.mIndices.push_back( id3 );
            triangles.mIndices.push_back( id0 );
            triangles.mIndices.push_back( id0 );
            triangles.mIndices.push_back( id1 );
            triangles.mIndices.push_back( id2 );
        }
    }
}

void TriangleGeneators::fillEdge( Triangles &triangles, Face polygon, float width, float depth, bool cw )
{
    const int pointCount = polygon.size();
    Face big = FaceGeneators::grow( polygon, width );
    for ( int i = 0; i < pointCount; ++i ) {
        const Vector2D p11 = polygon.at( i % pointCount );
        const Vector2D p12 = polygon.at(( i + 1) % pointCount );
        const Vector2D p21 = big.at(( i + 1) % pointCount );
        const Vector2D p22 = big.at( i % pointCount );
        const int id0 = triangles.addVertex( Vector3D( p11.x, p11.y, depth ), Vector3D( 0, 0, cw ? 1 : -1 ));
        const int id1 = triangles.addVertex( Vector3D( p12.x, p12.y, depth ), Vector3D( 0, 0, cw ? 1 : -1 ));
        const int id2 = triangles.addVertex( Vector3D( p21.x, p21.y, depth ), Vector3D( 0, 0, cw ? 1 : -1 ));
        const int id3 = triangles.addVertex( Vector3D( p22.x, p22.y, depth ), Vector3D( 0, 0, cw ? 1 : -1 ));
        if( cw ) {
            triangles.mIndices.push_back( id0 );
            triangles.mIndices.push_back( id3 );
            triangles.mIndices.push_back( id2 );
            triangles.mIndices.push_back( id2 );
            triangles.mIndices.push_back( id1 );
            triangles.mIndices.push_back( id0 );
        } else {
            triangles.mIndices.push_back( id0 );
            triangles.mIndices.push_back( id1 );
            triangles.mIndices.push_back( id2 );
            triangles.mIndices.push_back( id2 );
            triangles.mIndices.push_back( id3 );
            triangles.mIndices.push_back( id0 );
        }
    }

}

void TriangleGeneators::fillFace( Triangles &triangles, const Face polygon, float depth, bool bottom )
{
    Face points = polygon;
    const int pointCount = points.size();

    bool flip = FaceGeneators::checkOrientation( polygon );
    const int startIndex = triangles.mVertices.size();

    for( auto &point : polygon ) {
        triangles.mVertices.push_back( Vector3D( point.x, point.y, depth * ( bottom ? - 1 : 1 )));
        triangles.mNormals.push_back( Vector3D( 0, 0, bottom ? - 1 : 1 ));
    }

    vector<int> indexes;
    for( int i = 0; i < pointCount; ++i ) {
        if( flip )
            indexes.push_back( pointCount - i - 1 );
        else
            indexes.push_back( i );
    }

    while ( indexes.size() > 2 ) {
        int isize = indexes.size();
        const int psize = isize;
        for ( int i = 0; i < isize ; ++i ) {
            auto pb = points.begin();
            auto pe = points.end();
            auto p0 = pb + i;
            auto p1 = p0; ++p1;
            if( p1 == pe )
                p1 = pb;
            auto p2 = p1; ++p2;
            if( p2 == pe )
                p2 = pb;
            Vector2D forw( *p2 - *p0 );
            const float forwLength = forw.length();
            if( forwLength == 0 )
                continue;
            forw *= 1 / forwLength;
            Vector2D norm( forw.y, -forw.x );
            const float nrlen = Vector2D::dot( norm , *p0 - *p1 );
            if( nrlen <= 0 )
                continue;
            bool closed = true;
            for ( const auto &p : points ) {
                if( p == *p0 || p == *p1 || p == *p2 )
                    continue;
                const Vector2D diff = p - *p1;
                const float nlen = Vector2D::dot( norm , diff );
                if( nrlen < nlen || nlen < 1e-6f )
                    continue;
                const float scale = nrlen / nlen;
                const Vector2D ps = *p1 + diff * scale;
                const float d0 = Vector2D::dot( forw, *p0 - ps );
                const float d2 = Vector2D::dot( forw, *p2 - ps );
                if( d0 * d2 >= 0 )
                    continue;
                closed = false;
                break;
            }
            const int pi = indexes.at( i );
            const int pi1 = indexes.at(( i + 1 ) % isize );
            const int pi2 = indexes.at(( i + 2 ) % isize );
            if ( closed || 3 == isize ) {
                if ( bottom ) {
                    triangles.mIndices.push_back( startIndex + pi2 );
                    triangles.mIndices.push_back( startIndex + pi1 );
                    triangles.mIndices.push_back( startIndex + pi );
                } else {
                    triangles.mIndices.push_back( startIndex + pi );
                    triangles.mIndices.push_back( startIndex + pi1 );
                    triangles.mIndices.push_back( startIndex + pi2 );
                }
                const int idx1 = ( i + 1 ) % isize;
                points.erase( points.begin() + idx1 );
                indexes.erase( indexes.begin() + idx1 );
                isize = indexes.size();
            }
        }
        if( psize == isize )
            break;
    }
}

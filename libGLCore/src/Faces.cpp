#include "Faces.h"
#include "Core.h"
#include <map>
#include <set>
#include <vector>
#include <stack>

struct Point
{
    int         faceID;
    Vector2D    point;
    int         pointID;
    Point() { faceID = 0; pointID = 0; }
    Point( const Point &other ) {
        operator = ( other );
    }
    void operator = ( const Point &other ) {
        faceID = other.faceID;
        point = other.point;
        pointID = other.pointID;
    }
    bool operator < ( const Point& other ) const
    {
        if( other.faceID < faceID )
            return true;
        if( other.faceID > faceID )
            return false;
        return other.pointID < pointID;
    }
    bool operator == ( const Point& other ) const
    {
        return ( other.faceID == faceID && other.pointID == pointID );
    }
    bool operator != ( const Point& other ) const
    {
        return ( other.faceID != faceID || other.pointID != pointID );
    }
};

struct Line2D
{
    Line2D() {}
    Line2D( const Line2D& other ) {
        operator = ( other );
    }
    Line2D( const Point& p1, const Point& p2 ) {
        this->p1 = p1;
        this->p2 = p2;
    }
    void operator = ( const Line2D& other ) {
        p1 = other.p1;
        p2 = other.p2;
    }
    bool same( const Line2D& other ) const;
    bool intersect( const Line2D& other ) const;
    Point p1;
    Point p2;
};

bool Line2D::same(const Line2D &other) const
{
    return ( p2.faceID == other.p2.faceID && p2.pointID == other.p2.pointID ) ||
           ( p1.faceID == other.p1.faceID && p1.pointID == other.p1.pointID ) ||
           ( p2.faceID == other.p1.faceID && p2.pointID == other.p1.pointID ) ||
           ( p1.faceID == other.p2.faceID && p1.pointID == other.p2.pointID );
}

bool Line2D::intersect( const Line2D &other ) const
{
    float pos;
    Vector2D forw( p2.point - p1.point );
    float length = forw.length();
    float div = 1;
    if( length )
        div = 1 / length;
    forw *= div;
    const Vector2D nor( forw.y, -forw.x );
    const Vector2D d1( other.p1.point - p1.point );
    const Vector2D d2( other.p2.point - p1.point );
    float f1 = Vector2D::dot( forw, d1 );
    float f2 = Vector2D::dot( forw, d2 );
    if( f1 <= 0 && f2 <= 0 )
        return false;
    if( f1 >= length && f2 >= length )
        return false;
    float s1 = Vector2D::dot( nor, d1 );
    float s2 = Vector2D::dot( nor, d2 );
    if( s1 * s2 > 0 )
        return false;
    if( s1 == s2 )
        return false;
    pos = f1 - s1 * ( f2 - f1 ) / ( s2 - s1 );
    return ( pos > 0 && pos < length );
}

float degToRad( M_PI / 180 );
float radToDeg( 180 / M_PI );
float twoPI( 2 * M_PI );

bool Line::intersect( Vector2D const &pa1, const Vector2D &pa2, const Vector2D &pb1, const Vector2D &pb2, float &pos )
{
    Vector2D forw( pa2 - pa1 );
    float length = forw.length();
    float div = 1;
    if( length )
        div = 1 / length;
    forw *= div;
    const Vector2D nor( forw.y, -forw.x );
    const Vector2D d1( pb1 - pa1 );
    const Vector2D d2( pb2 - pa1 );
    float f1 = Vector2D::dot( forw, d1 );
    float f2 = Vector2D::dot( forw, d2 );
    if( f1 <= 0 && f2 <= 0 )
        return false;
    if( f1 >= length && f2 >= length )
        return false;
    float s1 = Vector2D::dot( nor, d1 );
    float s2 = Vector2D::dot( nor, d2 );
    if( s1 * s2 > 0 )
        return false;
    if( s1 == s2 )
        return false;
    pos = f1 - s1 * ( f2 - f1 ) / ( s2 - s1 );
    return ( pos > 0 && pos < length );
}

Faces::Faces()
{

}

Faces::Faces( const Face &polygon )
{
    push_back( polygon );
}

Faces::Faces(vector<Face> &faces) :  vector<Face>( faces )
{

}

void Faces::shift( Vector2D shift )
{
    for( auto &face : *this ) {
        face.shift( shift );
    }
}

BBoxFace::BBoxFace()
{
}

BBoxFace::BBoxFace( const BBoxFace &src ) : Face( src )
{
    operator = ( src );
}

BBoxFace::BBoxFace( const Face &src )
{
    bool first = true;
    for( auto pnt : src ) {
        if( first ) {
            minx = maxx = pnt.x;
            miny = maxy = pnt.y;
            first = false;
        } else {
            if( minx > pnt.x )
                minx = pnt.x;
            if( maxx < pnt.x )
                maxx = pnt.x;
            if( miny > pnt.y )
                miny = pnt.y;
            if( maxy < pnt.y )
                maxy = pnt.y;
        }
    }
}

BBoxFace &BBoxFace::operator = ( const BBoxFace &src )
{
    *(Face*)this = (Face)src;
    minx = src.minx;
    maxx = src.maxx;
    miny = src.miny;
    maxy = src.maxy;
    return *this;
}

void BBoxFace::addPoint( const Vector2D pnt )
{
    if( !size() ) {
        minx = maxx = pnt.x;
        miny = maxy = pnt.y;
    } else {
        if( minx > pnt.x )
            minx = pnt.x;
        if( maxx < pnt.x )
            maxx = pnt.x;
        if( miny > pnt.y )
            miny = pnt.y;
        if( maxy < pnt.y )
            maxy = pnt.y;
    }
    insert( begin(), pnt );
}

int BBoxFace::checkRelation( const BBoxFace &other )
{
    if( other.minx > maxx )
        return 0;
    if( other.maxx < minx )
        return 0;
    if( other.miny > maxy )
        return 0;
    if( other.maxy < miny )
        return 0;
    if( other.maxx <= maxx && other.minx >= minx &&
        other.maxy <= maxy && other.miny >= miny )
        return 1;
    if( other.maxx >= maxx && other.minx <= minx &&
        other.maxy >= maxy && other.miny <= miny )
        return -1;
    return 0;
}

void BBoxFace::grow( const BBoxFace &other )
{
    if( minx > other.minx )
        minx = other.minx;
    if( maxx < other.maxx )
        maxx = other.maxx;
    if( miny > other.miny )
        miny = other.miny;
    if( maxy < other.maxy )
        maxy = other.maxy;
}

void BBoxFace::setBox( const BBoxFace &other )
{
    minx = other.minx;
    maxx = other.maxx;
    miny = other.miny;
    maxy = other.maxy;
}

Face FaceGeneators::grow( const Face polygon, float width )
{
    Tangents tangents = generateTangents( polygon );
    const int pointCount = polygon.size();
    Face newPolygon;

    for ( int i = 0; i < pointCount; ++i ) {
        const Vector2D p0 = polygon.at( i );
        const Tangent tangent = tangents.at( i );
        const float midangle = ( tangent.pangle + tangent.nangle ) * 0.5;
        const Vector2D normal( sin( midangle ), cos( midangle ));
        newPolygon.push_back( p0 + normal * tangent.distance * width );
    }
    return newPolygon;
}

Face FaceGeneators::roundedRect( float width, float height, float radius, int step )
{
    Face polygon;
    for( int s = 0 ; s < 4; ++s ) {
        bool h = s & 1 ? 1 : 0;
        bool v = s & 2 ? 1 : 0;
        Vector2D p(( width * 0.5 - radius ) * ( h ^ v ? 1 : -1 ), ( height * 0.5 - radius ) * ( v ? -1 : 1 ));
        if( step ) {
            for( int i = 0; i <= step; ++i ) {
                float angle = ( 1.f * i / step + s ) * M_PI * 0.5;
                Vector2D d( -cos( angle ), sin( angle ));
                polygon.push_back( p + d * radius );
            }
        } else {
            float angle = s * M_PI * 0.5;
            Vector2D d( -cos( angle ), sin( angle ));
            polygon.push_back( p + d * radius );
        }
    }
    return polygon;
}

Face FaceGeneators::drill( const Face polygon, const Faces holes )
{
    if( !holes.size() )
        return polygon;
    Face result;
    std::map<int, Face>                     faces;
    std::vector<Line2D>                     allFaces;
    std::map<Point, Point>                  faceConnects;
    std::set<Point>                         usedPoints;
    std::vector<Point> points;
    std::map<int, int> pointCounts;
    int facepointid = 0;
    bool first = true;
    Point p;
    Point l;
    if( FaceGeneators::checkOrientation( polygon ) )
        faces.insert( std::pair<int, Face>( 0, polygon.reversed() ));
    else
        faces.insert( std::pair<int, Face>( 0, polygon ));
    int faceid = 1;
    for( auto face : holes ) {
        if( FaceGeneators::checkOrientation( face ) )
            faces.insert( std::pair<int, Face>( faceid++, face ));
        else
            faces.insert( std::pair<int, Face>( faceid++, face.reversed() ));
    }
    for( auto newFace : faces ) {
        std::pair<int, Face> face;
        face.first = newFace.first;
        face.second = newFace.second;
        facepointid = 0;
        pointCounts.insert( std::pair<int, size_t>( face.first, face.second.size() ));
        first = true;
        for( auto item : face.second ) {
            Point point;
            point.faceID = face.first;
            point.point = item;
            point.pointID = facepointid++;
            p = point;
            size_t id = 0;
            for( id = 0; id < points.size(); ++id ) {
                if( item.y < points.at( id ).point.y )
                    continue;
                if( item.y == points.at( id ).point.y && item.x > points.at( id ).point.x )
                    continue;
                break;
            }
            points.insert( points.begin() + id, point );
            if( first )
                first = false;
            else
                allFaces.push_back( Line2D( p, l ));
            l = p;
        }
        p.faceID = face.first;
        p.point = *face.second.begin();
        p.pointID = 0;
        allFaces.push_back( Line2D( l, p ));
    }
    bool next = false;
    for( auto face : faces ) {
        if( 0 == face.first )
            continue;
        next = false;
        for( auto otherface : faces ) {
            if( otherface.first == face.first )
                continue;
            Point p1;
            p1.faceID = otherface.first;
            int otherID = 0;
            for( auto otherpoint : otherface.second ) {
                p1.faceID = 0;
                p1.pointID = otherID++;
                p1.point = otherpoint;
                if( usedPoints.find( p1 ) != usedPoints.end() )
                    continue;
                Point p2;
                p2.faceID = face.first;
                int faceID = 0;
                for( auto point : face.second ) {
                    p2.pointID = faceID++;
                    p2.point = point;
                    if( usedPoints.find( p2 ) != usedPoints.end() )
                        continue;
                    Line2D newLine( p1, p2 );
                    bool cross = false;
                    for( auto &line : allFaces ) {
                        if( newLine.intersect( line )) {
                            cross = true;
                            break;
                        }
                    }
                    if( !cross ) {
                        usedPoints.insert( newLine.p1 );
                        usedPoints.insert( newLine.p2 );
                        faceConnects.insert({ newLine.p1, newLine.p2 });
                        allFaces.push_back( newLine );
                        next = true;
                        break;
                    }
                }
                p1 = p2;
                if( next )
                    break;
            }
            if( next )
                break;
        }
    }

    std::stack<Point> pointStack;
    std::pair<Point, Point> line;
    Point zeropoint;

    line = *faceConnects.begin();
    zeropoint = line.first;
    pointStack.push( line.first );
    faceConnects.erase( line.first );
    Point point = line.second;
    result.push_back( line.first.point );
    while( faceConnects.size() || pointStack.size() ) {
        Point start = point;
        while( faceConnects.find( point ) == faceConnects.end() ) {
            point.point = faces[ point.faceID ][ point.pointID ];
            result.push_back( point.point );
            ++point.pointID;
            if( pointCounts[ point.faceID ] <= point.pointID ) {
                point.pointID = 0;
            }
            if( point == start ) {
                break;
            }
        }
        if( faceConnects.find( point ) != faceConnects.end() ) {
            point.point = faces[ point.faceID ][ point.pointID ];
            result.push_back( point.point );
            pointStack.push( point );
            line = *faceConnects.find( point );
            point = line.second;
            faceConnects.erase( line.first );
        }
        if( point == start ) {
            point.point = faces[ point.faceID ][ point.pointID ];
            result.push_back( point.point );
            point = pointStack.top();
            pointStack.pop();
        }
    }
    for(;;) {
        point.point = faces[ point.faceID ][ point.pointID ];
        result.push_back( point.point );
        ++point.pointID;
        if( pointCounts[ point.faceID ] <= point.pointID ) {
            point.pointID = 0;
        }
        if( zeropoint.pointID == point.pointID )
            break;
    }
    return result;
}

Tangents FaceGeneators::generateTangents( const Face polygon )
{
    const int pointCount = polygon.size();

    Tangents tangents;
    Tangent tangent;
    float prevangle = 0;
    auto p1 = polygon.begin() + ( polygon.size() - 1 );
    auto p0 = p1;
    for ( int i = 0; i <= pointCount; ++i ) {
        if( ++p1 == polygon.end() )
            p1 = polygon.begin();
        Vector2D v1( *p1 - *p0 );
        float curangle = atan2( v1.x, v1.y );
        if( curangle < 0 ) {
            curangle += twoPI;
        }
        if( i != 0 ) {
            float diffangle = curangle - prevangle;
            if( diffangle > M_PI )
                diffangle -= twoPI;
            if( diffangle < -M_PI )
                diffangle += twoPI;
            tangent.pangle = prevangle - M_PI * 0.5;
            tangent.nangle = prevangle + diffangle - M_PI * 0.5;
            tangent.distance = angleToDist( fabs( diffangle * 0.5 ));
            tangents.push_back( tangent );
        }
        prevangle = curangle;
        p0 = p1;
    }
    return tangents;
}

bool FaceGeneators::checkOrientation( const Face &polygon )
{
    const int pointCount = polygon.size();

    float angle = 0;
    float prevangle = 0;
    auto p1 = polygon.begin();
    auto p0 = p1;
    for ( int i = 0; i <= pointCount; ++i ) {
        if( ++p1 == polygon.end() )
            p1 = polygon.begin();
        Vector2D v1( *p1 - *p0 );
        float curangle = atan2( v1.x, v1.y );
        if( curangle < 0 )
            curangle += twoPI;
        float diffangle = curangle - prevangle;
        if( diffangle > M_PI )
            diffangle -= twoPI;
        if( diffangle < -M_PI )
            diffangle += twoPI;
        if( i != 0 )
            angle += diffangle;
        prevangle = curangle;
        p0 = p1;
    }
    return angle < 0;
}

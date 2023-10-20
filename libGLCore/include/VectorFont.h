/* Copyright by János Klingl in 2023 */

#ifndef VECTORFONT_H
#define VECTORFONT_H

#include <map>
#include <vector>
#include <memory>
#include "Faces.h"
#include "Triangles.h"

class BBoxFace;

/* Store and help to calculate font kerning */

struct ZeroX
{
    std::vector<float> leftPos;
    std::vector<float> rightPos;
    std::vector<float> angleScales;
    std::vector<float> scales;
    bool first;
    float dy;
    ZeroX();
    ZeroX( const ZeroX &other );
    ZeroX * operator = ( const ZeroX &other );
    void setAngles( float angle, int dirsteps );
    void setRange( float minx, float maxx );
    float calcPos( float angleScale, const Vector2D &point );
    float getMin( const ZeroX &previous, float gap );
    void addFace( const Face &face );
};

/* Calculate kerning */

struct KerningSource
{
    ZeroX zerox;
    BBoxFace bBox;
    KerningSource();
    KerningSource( const KerningSource &other );
    KerningSource * operator = ( const KerningSource &other );
    void calc();
    void addFace( const Face &face );
    float getWidth();
    float getKerning( KerningSource *prevChar, float gap );
};

/* Storage and handlers of vector fonts */

struct VectorFont
{
    float                                           minx;
    float                                           gap;
    std::map< long, BBoxFace >                      letter_boxes;
    std::map< long, KerningSource>                  kernings;
    std::map< long, std::vector< std::pair< Face, std::vector< Face >>>> characters;
    std::map< long, Triangles >                     letters;

    struct Char3D
    {
        Triangles                                   geometry;
        Vector3D                                    pos;
    };

    VectorFont();
    void Init( std::map< long, std::vector<std::vector<std::pair<double,double>>>> font_src, float grow = 0.1f, float depth = 0.3f, float bevel = 0.06, int roundStep = 1 );
    unsigned long                                   fromUTF8( unsigned long narrow );
    unsigned long                                   toUTF8( unsigned long wide );
    int                                             UTF8len( unsigned long narrow );

    typedef struct std::vector< std::shared_ptr< Char3D >> Text3D;
    Text3D                                          genTextChars( string text );
    Triangles                                       genText( string text );
    Triangles                                       genTextPlane( string text );
};

#endif // VECTORFONT_H

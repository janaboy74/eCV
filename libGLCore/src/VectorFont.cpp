#include "VectorFont.h"
#include "GLCore.h"
#include <iostream>

extern float degToRad;

ZeroX::ZeroX()
{
    first = true;
}

ZeroX::ZeroX( const ZeroX &other )
{
    operator = ( other );
}

ZeroX *ZeroX::operator = ( const ZeroX &other )
{
    leftPos = other.leftPos;
    rightPos = other.rightPos;
    angleScales = other.angleScales;
    dy = other.dy;
    first = other.first;
    return this;
}

void ZeroX::setAngles( float angle, int dirsteps )
{
    float angleStep = 0;
    if( dirsteps )
        angleStep = angle / dirsteps;
    for( int s = -dirsteps; s <= dirsteps; ++s ) {
        float curAngle = s * angleStep;
        curAngle *= degToRad;
        angleScales.push_back( tan( curAngle ));
        scales.push_back( 1 / cos( curAngle ));
    }
}

void ZeroX::setRange( float minx, float maxx )
{
    int anglescls = angleScales.size();
    for( int s = 0; s < anglescls; ++s ) {
        auto &lpos = leftPos[ s ];
        auto &rpos = rightPos[ s ];
        lpos -= minx;
        rpos -= maxx;
    }
}

float ZeroX::calcPos( float angleScale, const Vector2D &point )
{
    return point.x + angleScale * ( point.y );
}

float ZeroX::getMin( const ZeroX &previous, float gap )
{
    int anglescls = angleScales.size();
    float min = 0;
    bool first = true;
    for( int s = 0; s < anglescls; ++s ) {
        const auto &scl = scales[ s ];
        const auto &lpos = leftPos[ s ];
        const auto &rpos = previous.rightPos[ s ];
        float d = ( rpos - lpos ) * 0.8f + /*1.0 * */scl - ( 1 - gap );
        if( first )
            min = d;
        else
            if( min > d )
                min = d;
        first = false;
    }
    return min;
}

void ZeroX::addFace( const Face &face )
{
    int anglescls = angleScales.size();
    if( !leftPos.size() || !rightPos.size() )
    for( int s = 0; s < anglescls; ++s ) {
        leftPos.push_back( 0 );
        rightPos.push_back( 0 );
    }
    for( const auto &point : face ) {
        for( int s = 0; s < anglescls; ++s ) {
            const auto &angle = angleScales[ s ];
            auto &lpos = leftPos[ s ];
            auto &rpos = rightPos[ s ];
            if( first ) {
                lpos = calcPos( angle, point );
                rpos = calcPos( angle, point );
            } else {
                float tmpx;
                tmpx =  calcPos( angle, point );
                if( lpos > tmpx )
                    lpos = tmpx;
                tmpx =  calcPos( angle, point );
                if( rpos < tmpx )
                    rpos = tmpx;
            }
       }
       first = false;
    }
}

KerningSource::KerningSource()
{
    zerox.setAngles( 20, 2 );
}

KerningSource::KerningSource( const KerningSource &other ) : KerningSource()
{
    operator = ( other );
}

KerningSource *KerningSource::operator = ( const KerningSource &other )
{
    zerox = other.zerox;
    bBox = other.bBox;
    return this;
}

void KerningSource::calc()
{
    zerox.setRange( bBox.minx, bBox.maxx );
}

void KerningSource::addFace( const Face &face )
{
    zerox.addFace( face );
}

float KerningSource::getWidth()
{
    return bBox.maxx - bBox.minx;
}

float KerningSource::getKerning( KerningSource *prevChar, float gap )
{
    if( prevChar )
        return zerox.getMin( prevChar->zerox, gap );
    return 0;
}

VectorFont::VectorFont() : gap( 0.32 )
{
}

void VectorFont::Init( std::map< long, std::vector<std::vector<std::pair<double,double>>>> font_src, float grow, float depth, float bevel, int roundStep ) {
    characters.clear();
    std::vector< std::pair< long, std::vector< BBoxFace >>> char_srcs;
    std::shared_ptr<BBoxFace> globalBox;
    for( auto letter : font_src ) {
        std::vector< BBoxFace > boxes;
        for( auto poly : letter.second ) {
            BBoxFace box;
            for( auto point : poly ) {
                box.addPoint( Vector2D( point.first, point.second ));
            }
            boxes.push_back( box );
        }
        BBoxFace letter_box;
        letter_box.setBox( boxes.front() );
        for( auto &box: boxes ) {
            letter_box.grow( box );
        }
        if( globalBox.get() )
            globalBox->grow( letter_box );
        else
            globalBox = std::make_shared<BBoxFace>( letter_box );
        char_srcs.push_back( std::pair< long, std::vector< BBoxFace >>( letter.first, boxes ));
        letter_boxes.insert( std::pair< long, BBoxFace >( letter.first, letter_box ));
    }
    for( auto &letter: char_srcs ) {
        std::vector< std::pair< Face, std::vector< Face >>> char_polys;
        while( letter.second.size() ) {
            auto &polygons = letter.second;
            auto &first_poly = polygons.front();
            int outside = 0;
            int inbound = 0;
            int outbound = 0;
            for( auto &face: letter.second ) {
                if( &first_poly == &face )
                    continue;
                int actrelation = first_poly.checkRelation( face );
                switch ( actrelation ) {
                case 0:
                    ++outside;
                    break;
                case -1:
                    ++outbound;
                    break;
                case 1:
                    ++inbound;
                    break;
                default:
                    break;
                }
            }
            if( !inbound && !outbound ) {
                char_polys.push_back( std::pair< Face, std::vector< Face >> ( first_poly, std::vector< Face >()));
                polygons.erase( polygons.begin() );
            }
            if( inbound && !outbound ) {
                std::vector< Face > second_polys;
                for( auto it = ++polygons.begin(); it != polygons.end(); ) {
                    auto face = ( *it );
                    int actrelation = first_poly.checkRelation( face );
                    if( actrelation == 1 ) {
                        second_polys.push_back( face );
                        polygons.erase( it );
                    } else {
                        ++it;
                    }
                }
                char_polys.push_back( std::pair< Face, std::vector< Face >> ( first_poly, second_polys ));
                polygons.erase( polygons.begin() );
            }
            if( !inbound && outbound ) {
                auto poly = first_poly;
                polygons.erase( polygons.begin() );
                polygons.push_back( poly );
            }
            if( inbound && outbound ) {
                std::cerr << "in and outbound in same contrext!\n";
                polygons.erase( polygons.begin() );
            }
        }
        characters.insert( std::pair< long, std::vector< std::pair< Face, std::vector< Face >>>>( letter.first, char_polys ));
        Triangles letter3D;
        for( auto poly : char_polys ) {
            letter3D += TriangleGeneators::bevelExtrude( poly.first, poly.second, depth, bevel, roundStep, true, true ).optimized();
        }
        letters.insert( std::pair<long, Triangles >( letter.first, letter3D ));
    }
    for( auto &letter: characters ) {
        KerningSource kerning;
        kerning.zerox.dy = globalBox->minx;
        for( auto &face: letter.second ) {
            if( grow )
                kerning.addFace( FaceGeneators::grow( face.first, grow ));
            else
                kerning.addFace( face.first );
        }
        auto &box = letter_boxes.at(( wchar_t ) letter.first );
        kerning.bBox = box;
        kerning.calc();
        kernings.insert( std::pair< long, KerningSource >( letter.first, kerning ));
    }
}

unsigned long VectorFont::fromUTF8( unsigned long narrow ) {
    if (( narrow & 0xc0e0 ) == 0x80c0 ) {
        return (( narrow & 0x3f00 ) >> 8 | ( narrow & 0x1f ) << 6 );
    }
    if (( narrow & 0xc0c0f0 ) == 0x8080e0 ) {
        return (( narrow & 0x3f0000 ) >> 16 | ( narrow & 0x3f00 ) >> 2 | ( narrow & 0x1f ) << 12 );
    }
    return narrow & 0x7f;
}

unsigned long VectorFont::toUTF8( unsigned long wide ) {
    if ( wide < 0x80 ) {
        return wide;
    } else if ( wide < 0x1000 ) {
        return ( wide & 0xfc0 ) >> 6 | ( wide & 0x3f ) << 6 | 0xc080;
    } else if ( wide < 0x20000 ) {
        return ( wide & 0x1f000 ) >> 12 | ( wide & 0xfc0 ) << 2 | ( wide & 0x3f ) << 16  | 0x8080e0;
    }
    return 0;
}

int VectorFont::UTF8len( unsigned long narrow ) {
    if (( narrow & 0x80 ) == 0x00 ) {
        return 1;
    } else if (( narrow & 0xc0e0 ) == 0x80c0 ) {
        return 2;
    } if (( narrow & 0xc0c0f0 ) == 0x8080e0 ) {
        return 3;
    }
    return 4;
}

VectorFont::Text3D VectorFont::genTextChars( string text ) {
    Faces holes;
    Text3D chars3D;
    float xpos = 0;
    float ypos = 0;
    Text3D line;
    KerningSource *prevKerning = 0;
    for( size_t chrpos = 0; chrpos < text.size(); ) {
        unsigned long narrow = 0;
        text.copy(( char* ) &narrow, 4, chrpos );
        wchar_t charID = narrow & 0xff;
        int utf8len = UTF8len( narrow );
        switch( utf8len ) {
        case 2:
            charID = fromUTF8( narrow );
            break;
        case 3:
            charID = fromUTF8( narrow );
            break;
        default:
            break;
        }
        chrpos += utf8len;
        holes.clear();
        if( charID == ' ' ) {
            xpos += 0.8;
            continue;
        }
        if( charID == '\n' ) {
            if( line.size() ) {
                for( auto letter : line ) {
                    letter->pos += Vector3D( -xpos * 0.5, ypos, 0 );
                    chars3D.push_back( letter );
                }
            }
            line.clear();
            xpos = 0;
            ypos -= 3.0;
            continue;
        }

        if( letters.find( charID ) == letters.end() ) // char not available
            continue;
        std::shared_ptr< Char3D> letter = std::make_shared< Char3D >();
        letter->geometry = letters.at( charID );
        auto &kerning = kernings.at( charID );
        xpos += kerning.getKerning( prevKerning, gap );
        letter->pos = Vector3D( xpos, 0, 0 );
        line.push_back( letter );

        xpos += kerning.getWidth();
        prevKerning = &kerning;
    }
    for( auto letter : line ) {
        letter->pos += Vector3D( -xpos * 0.5, ypos, 0 );
        chars3D.push_back( letter );
    }
    return chars3D;
}

Triangles VectorFont::genText( string text ) {
    Faces holes;
    Triangles text3D;
    Triangles intertext;
    float xpos = 0;
    float ypos = 0;
    KerningSource *prevKerning = 0;
    for( size_t chrpos = 0; chrpos < text.size(); ) {
        unsigned long narrow = 0;
        text.copy(( char* ) &narrow, 4, chrpos );
        wchar_t charID = narrow & 0xff;
        int utf8len = UTF8len( narrow );
        switch( utf8len ) {
        case 2:
            charID = fromUTF8( narrow );
            break;
        case 3:
            charID = fromUTF8( narrow );
            break;
        default:
            break;
        }
        chrpos += utf8len;
        holes.clear();
        if( charID == ' ' ) {
            xpos += 0.8;
            continue;
        }
        if( charID == '\n' ) {
            if( intertext.mVertices.size() ) {
                intertext.shift( Vector3D( -xpos * 0.5, ypos, 0 ));
                text3D += intertext;
            }
            intertext.clear();
            xpos = 0;
            ypos -= 3.0;
            continue;
        }

        if( letters.find( charID ) == letters.end() ) // char not available
            continue;
        auto &kerning = kernings.at( charID );
        xpos += kerning.getKerning( prevKerning, gap );
        Triangles letter = letters.at( charID );
        letter.shift( Vector3D( xpos, 0, 0 ));
        intertext += letter;
        xpos += kerning.getWidth();
        prevKerning = &kerning;
    }
    if( intertext.mVertices.size() ) {
        intertext.shift( Vector3D( -xpos * 0.5, ypos, 0 ));
        text3D += intertext;
    }
    return text3D;
}

Triangles VectorFont::genTextPlane( string text )
{
    Faces holes;
    Triangles textPlane;
    Triangles intertext;
    float xpos = 0;
    float ypos = 0;
    KerningSource *prevKerning = 0;
    for( size_t chrpos = 0; chrpos < text.size(); ) {
        unsigned long narrow = 0;
        text.copy(( char* ) &narrow, 4, chrpos );
        wchar_t charID = narrow & 0xff;
        int utf8len = UTF8len( narrow );
        switch( utf8len ) {
        case 2:
            charID = fromUTF8( narrow );
            break;
        case 3:
            charID = fromUTF8( narrow );
            break;
        default:
            break;
        }
        chrpos += utf8len;
        holes.clear();
        if( charID == ' ' ) {
            xpos += 0.8;
            continue;
        }
        if( charID == '\n' ) {
            if( intertext.mVertices.size() ) {
                intertext.shift( Vector3D( -xpos * 0.5, ypos, 0 ));
                textPlane += intertext;
            }
            intertext.clear();
            xpos = 0;
            ypos -= 3.0;
            continue;
        }

        if( letters.find( charID ) == letters.end() ) // char not available
            continue;
        auto &kerning = kernings.at( charID );
        xpos += kerning.getKerning( prevKerning, gap );
        Triangles letter = letters.at( charID );
        letter.shift( Vector3D( xpos, 0, 0 ));
        intertext += letter;
        xpos += kerning.getWidth();
        prevKerning = &kerning;
    }
    if( intertext.mVertices.size() ) {
        intertext.shift( Vector3D( -xpos * 0.5, ypos, 0 ));
        textPlane += intertext;
    }
    return textPlane;
}

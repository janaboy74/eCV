#include <iostream>
#include <ft2build.h>
#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <algorithm>
#include <string>
#include <locale>
#include <codecvt>
#ifdef _WIN32
#include <windows.h>
#endif
#include FT_FREETYPE_H
#include FT_OUTLINE_H

float degToRad( M_PI / 180 );
float radToDeg( 180 / M_PI );
float twoPI( 2 * M_PI );

#ifdef _WIN32
int vasprintf( char **strp, const char *fmt, va_list ap ) {
    // _vscprintf tells you how big the buffer needs to be
    int len = _vscprintf( fmt, ap );
    if( len == -1 ) {
        return -1;
    }
    size_t size = ( size_t ) len + 1;
    char *str = ( char * )malloc( size );
    if( !str ) {
        return -1;
    }
    // _vsprintf_s is the "secure" version of vsprintf
    int r = vsprintf_s( str, len + 1, fmt, ap );
    if ( r == -1 ) {
        free( str );
        return -1;
    }
    *strp = str;
    return r;
}

int asprintf( char **strp, const char *fmt, ... ) {
    va_list ap;
    va_start( ap, fmt );
    int r = vasprintf( strp, fmt, ap );
    va_end( ap );
    return r;
}
#endif

struct Vector2D
{
    float x;
    float y;
    Vector2D( float x = 0, float y = 0 ) : x( x ), y( y ) {}
    void operator *= ( const float scale ) {
        x *= scale ;
        y *= scale;
    }
};

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

struct Plane : public std::vector<Vector2D>
{
    Plane() {};
    Plane( vector<Vector2D> &points ) {};
    bool checkOrientation() {
        const int pointCount = size();

        float angle = 0;
        float prevangle = 0;
        auto p1 = begin();
        auto p0 = p1;
        for ( int i = 0; i <= pointCount; ++i ) {
            if( ++p1 == end() )
                p1 = begin();
            Vector2D v1( *p1 - *p0 );
            float curangle = atan2( v1.x, v1.y );
            if( curangle <= 0 )
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
    Plane reversed() const {
        Plane back;
        for( auto &point : *this ) {
            back.insert( back.begin(), point );
        }
        return back;
    }
};

struct CharData
{
    std::vector< Plane > planes;
    Plane plane;
    Vector2D previous;
    int steps;

    CharData() {
        steps = 4;
    }
    void clear() {
        planes.clear();
    }
    void addVertex( Vector2D pos )
    {
        pos *= 1e-3f;
//        if ( plane.size() && plane.back() == pos ) {
//            // OSG_NOTICE<<"addVertex("<<pos<<") duplicate, ignoring"<<std::endl;
//            return;
//        }
        plane.push_back( Vector2D( pos.x, pos.y ));
//        plane.insert( plane.begin(), Vector2D( pos.x, pos.y ));
    }
    void moveTo( const Vector2D pos )
    {
        finish();
        addVertex( pos );
        previous = pos;
    }

    void lineTo( const Vector2D pos )
    {
        addVertex( pos );
        previous = pos;
    }

    void conicTo( const Vector2D control,  const Vector2D pos)
    {
        double dt = 1.0 / steps;
        double u = dt;
        Vector2D vec;
        for( int i = 0; i < steps; ++i )
        {
            double w = 1;
            double u2 = u * u;
            double nu = 1 - u;
            double nu2 = nu * nu;
            double bs = 1.0 / ( nu2 + 2 * nu * u * w + u2 );
            vec = Vector2D( previous * nu2 + control * ( 2 * nu * u * w ) + pos * u2 ) * bs;
            addVertex( vec );
            u += dt;
        }
        previous = pos;
    }

    void cubicTo( const Vector2D control1,  const Vector2D control2,  const Vector2D pos)
    {
        double cx = 3 * ( control1.x - previous.x );
        double bx = 3 * ( control2.x - control1.x ) - cx;
        double ax = pos.x - previous.x - cx - bx;
        double cy = 3 * ( control1.y - previous.y );
        double by = 3 * ( control2.y - control1.y ) - cy;
        double ay = pos.y - previous.y - cy - by;

        double dt = 1.0 / steps;
        double u = dt;
        Vector2D vec;
        for( int i = 0; i < steps; ++i ) {
            vec = Vector2D( ax*u*u*u + bx*u*u + cx*u + previous.x, ay*u*u*u + by*u*u  + cy*u + previous.y );
            addVertex( vec );
            u += dt;
        }
        previous = pos;
    }
    void finish() {
        if( plane.size() )
            planes.push_back( plane );
        plane.clear();
    }
};

int moveTo( const FT_Vector* to, void* user )
{
    CharData* chardata = ( CharData* ) user;
    chardata->moveTo( Vector2D( to->x, to->y ));
    return 0;
}

int lineTo( const FT_Vector* to, void* user )
{
    CharData* chardata = ( CharData* ) user;
    chardata->lineTo( Vector2D( to->x, to->y ));
    return 0;
}

int conicTo( const FT_Vector* control,const FT_Vector* to, void* user )
{
    CharData* chardata = ( CharData* ) user;
    chardata->conicTo( Vector2D( control->x, control->y ), Vector2D( to->x, to->y ));
    return 0;
}

int cubicTo( const FT_Vector* control1,const FT_Vector* control2,const FT_Vector* to, void* user )
{
    CharData* chardata = ( CharData* ) user;
    chardata->cubicTo( Vector2D( control1->x, control1->y ),
                       Vector2D( control2->x, control2->y ),
                       Vector2D( to->x, to->y ));
    return 0;
}

unsigned long flip2( unsigned long src ) {
    return ( src & 0xff ) << 8 | ( src & 0xff00 ) >> 8;
}

unsigned long flip3( unsigned long src ) {
    return ( src & 0xff ) << 16 | ( src & 0xff00 ) | ( src & 0xff0000 ) >> 16;
}

unsigned long flip4( unsigned long src ) {
    return ( src & 0xff ) << 24 | ( src & 0xff00 ) << 8 | ( src & 0xff0000 ) >> 8 | ( src & 0xff000000 ) >> 24;
}

//unsigned long fromUTF8( unsigned long narrow ) {
//    if (( narrow & 0xf0c0c0 ) == 0xe08080 ) {
//        return ( narrow & 0x1f0000 ) >> 4 | ( narrow & 0x3f00 ) >> 2 | narrow & 0x3f;
//    } else if (( narrow & 0xe0c0 == 0xc080 )) {
//        return ( narrow & 0x1fc0 ) >> 2 | narrow & 0x3f;
//    }
//    return narrow & 0x7f;
//}

//unsigned long toUTF8( unsigned long wide ) {
//    if ( wide < 0x80 ) {
//        return wide;
//    } else if ( wide < 0x800 ) {
//        return ( wide & 0xfc0 ) << 2 | ( wide & 0x3f ) | 0xc080;
//    } else if ( wide < 0x80000 ) {
//        return ( wide & 0x1fc000 ) << 4 | ( wide & 0xfc0 ) << 2 | ( wide & 0x3f ) | 0xe08080;
//    }
//    return 0;
//}

// 0000 0
// 0001 1
// 0010 2
// 0011 3
// 0100 4
// 0101 5
// 0110 6
// 0111 7
// 1000 8
// 1001 9
// 1010 a
// 1011 b
// 1100 c
// 1101 d
// 1110 e
// 1111 f

unsigned long fromUTF8( unsigned long narrow ) {
    if (( narrow & 0xc0e0 ) == 0x80c0 ) {
        return (( narrow & 0x3f00 ) >> 8 | ( narrow & 0x1f ) << 6 );
    }
    if (( narrow & 0xc0c0f0 ) == 0x8080e0 ) {
        return (( narrow & 0x3f0000 ) >> 16 | ( narrow & 0x3f00 ) >> 2 | ( narrow & 0x1f ) << 12 );
    }
    return narrow & 0x7f;
}

unsigned long toUTF8( unsigned long wide ) {
    if ( wide < 0x80 ) {
        return wide;
    } else if ( wide < 0x1000 ) {
        return ( wide & 0xfc0 ) >> 6 | ( wide & 0x3f ) << 6 | 0xc080;
    } else if ( wide < 0x20000 ) {
        return ( wide & 0x1f000 ) >> 12 | ( wide & 0xfc0 ) << 2 | ( wide & 0x3f ) << 16  | 0x8080e0;
    }
    return 0;
}

int UTF8len( unsigned long narrow ) {
    if (( narrow & 0x80 ) == 0x00 ) {
        return 1;
    } else if (( narrow & 0xc0e0 ) == 0x80c0 ) {
        return 2;
    } if (( narrow & 0xc0c0f0 ) == 0x8080e0 ) {
        return 3;
    }
    return 4;
}

class FontParser
{
    FT_Library      library;
    FT_Face         face;
    FT_Error        error;
    CharData        charData;
    std::string     fontName;
public:
    FontParser( std::string filename )
    {
        size_t lastindex = filename.find_last_of( '.' );
        fontName = filename.substr( 0, lastindex );
        std::replace( fontName.begin(), fontName.end(), '-', '_' );
        std::replace( fontName.begin(), fontName.end(), ' ', '_' );
        FT_Init_FreeType( &library );
        FT_New_Face( library, filename.c_str(), 0, &face );
        FT_Set_Pixel_Sizes( face, 32, 32 );
        FT_Select_Charmap( face , ft_encoding_unicode );
    };
    ~FontParser()
    {
        FT_Done_Face( face );
        FT_Done_FreeType( library );
    }
    void Parse()
    {
        FT_Outline_Funcs funcs;
        funcs.conic_to = (FT_Outline_ConicToFunc)&conicTo;
        funcs.line_to = (FT_Outline_LineToFunc)&lineTo;
        funcs.cubic_to = (FT_Outline_CubicToFunc)&cubicTo;
        funcs.move_to = (FT_Outline_MoveToFunc)&moveTo;
        funcs.shift = 0;
        funcs.delta = 0;

        int outFile = open(( fontName + ".h" ).c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666 );
        std::string str = "#include <vector>\n"
                          "#include <map>\n"
                          "#include <utility>\n"
                          "namespace " + fontName + " {\n"
                          "std::map< long, std::vector<std::vector<std::pair<double,double>>>> font = {\n";
        write( outFile, str.c_str(), str.size() );

        std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZÖÜÓŐÚÉÁŰÍabcdefghijklmnopqrstuvwxyzöüópőúéáűí0123456789\\\"_-+/*?!%/=()&,.:$€<>[]{}°^~";

        bool firstChar = true;
        for( size_t pos = 0; pos < chars.size(); ) {
            unsigned long narrow = 0;
            chars.copy(( char* ) &narrow, 4, pos );
            wchar_t wide = narrow & 0xff;
            int unichar = wide;
            int utf8len = UTF8len( narrow );
            switch( utf8len ) {
            case 2:
                unichar = narrow & 0xffff;
                wide = fromUTF8( narrow );
                break;
            case 3:
                unichar = narrow & 0xffffff;
                wide = fromUTF8( narrow );
                break;
            default:
                break;
            }
            pos += utf8len;
            if( firstChar )
                firstChar = false;
            else
                write( outFile, ",\n", 2 );
            FT_Load_Char( face, wide, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING );
            if (face->glyph->format != FT_GLYPH_FORMAT_OUTLINE)
            {
                std::cerr << "FreeTypeFont3D::getGlyph : not a vector font" << std::endl;
                return;
            }
            FT_Outline outline = face->glyph->outline;
            FT_Error _error = FT_Outline_Decompose( &outline, &funcs, &charData );
            charData.finish();
            if (_error)
            {
                std::cerr << "FreeTypeFont3D::getGlyph : - outline decompose failed ..." << std::endl;
            }
            const char *doubleslash = "\\\\\0\0";
            if( unichar == '\\' )
                unichar = *(wchar_t*)doubleslash;
//            FT_Orientation orientation = FT_Outline_Get_Orientation( &outline );
//            if( orientation == FT_ORIENTATION_POSTSCRIPT ) {
//                for( auto &plane : charData.planes ) {
//                    plane.pop_back();
//                    plane = plane.reversed();
//                }
//            }
            for( auto &plane : charData.planes ) {
                plane.pop_back();
                if( !plane.checkOrientation() ) {
                    plane = plane.reversed();
                }
            }

            Print( outFile, (const char*)&unichar );
            charData.clear();
        }
        str = "};\n};\n";
        write( outFile, str.c_str(), str.size() );
        close( outFile );
    }
    void Print( int outFile, const char* unichar ) {
        std::string out;
        char *tmpStr;
        asprintf( &tmpStr, "{L'%s',{", unichar ); out.append( tmpStr ); delete tmpStr; tmpStr = 0;
        bool firstPoly = true;
        for( auto &plane : charData.planes ) {
            if( firstPoly )
                firstPoly = false;
            else
                out.append( ",\n" );
            out.append( "{" );
            int i = 0;
            for( auto vec2 : plane ) {
                if( i )
                    out.append( "," );
                asprintf( &tmpStr, "{ %1.5f, %1.5f }", vec2.x , vec2.y ); out.append( tmpStr ); delete tmpStr; tmpStr = 0;
                ++i;
            }
            out.append( "}" );
        }
        out.append( "}}" );
        write( outFile, out.c_str(), out.size() );
    }
    void setSteps( int steps ) {
        charData.steps = steps;
    }
};

using namespace std;

int main( int argc, char **argv )
{
    if( argc < 2 ) {
        std::cout << "Usage:\n" << argv[0] << " [font file name] [steps]\n";
        return 0;
    }
    FontParser fontparser( argv[ 1 ]);
    if( argc > 2 )
        fontparser.setSteps( atoi( argv[ 2 ]));
    fontparser.Parse();
    return 0;
}

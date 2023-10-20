#include "Settings.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

bool check( const char ch, const char *array ) {
    for( auto chx = array; *chx; ++chx )
        if( ch == *chx )
            return true;
    return false;
}

corestring Settings::getVar( std::string name )
{
    if( variables.find( name ) != variables.end() )
        return corestring( variables.at( name ));
    return corestring();
}

corestring Settings::getVar( std::string name, std::string defaultValue )
{
    if( variables.find( name ) == variables.end() )
        variables.insert( std::pair<std::string,std::string>( name, defaultValue ));
    return getVar( name );
}

Settings::Settings( const char *filename ) {
    size = 0;
    if( filename )
        open( filename );
}

void Settings::open( const char *filename ) {
    struct stat st;
    int fd = ::open( filename, O_RDONLY );
    fstat( fd, &st );
    size = st.st_size;
    std::shared_ptr<unsigned char[]> buffer = std::shared_ptr<unsigned char[]>( new unsigned char[ size + 1 ]);
    buffer[ size ] = 0;
    size = read( fd, buffer.get(), size );
    close( fd );
    bool is_quote = false;
    bool is_var = true;
    std::vector<std::string> varstruct;
    std::string name;
    std::string value;
    variables.clear();
    for( auto chr = buffer.get(); *chr; ++chr ) {
        if( '"' == *chr ) {
            is_quote = !is_quote;
            continue;
        } else if( '{' == *chr ) {
            if( name.length() )
                varstruct.push_back( name );
            is_var = true;
            name.clear();
            value.clear();
            continue;
        } else if( '}' == *chr ) {
            if( varstruct.size() )
                varstruct.pop_back();
            is_var = true;
            name.clear();
            value.clear();
            continue;
        }
        if( !is_quote ) {
            if( '=' == *chr || ':' == *chr ) {
                is_var = false;
                continue;
            } else if( check( *chr, ",;\r\n" )) {
                if( !is_var && name.length() ) {
                    std::string fullname;
                    for( auto varname : varstruct ) {
                        fullname.append( varname );
                        fullname.append( ":" );
                    }
                    fullname.append( name );
                    variables.insert( std::pair<std::string,std::string>( fullname, value ));
                    name.clear();
                    value.clear();
                    is_var = true;
                    continue;
                }
            }
        }
        if( check( *chr, " \t\r\n" )) {
            if( is_quote )
                is_var?name.push_back( *chr ):value.push_back( *chr );
            else
                is_var?name.clear():value.clear();
        } else {
            is_var?name.push_back( *chr ):value.push_back( *chr );
        }
    }
}

#include "Core.h"
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cctype>

int log_file_id;

FileBuffer::FileBuffer( const char *filename ) {
    size = 0;
    if( filename )
        openAndRead( filename );
}

bool FileBuffer::openAndRead( const char *filename ) {
    struct stat st;
    int fd = ::open( filename, O_RDONLY );
    fstat( fd, &st );
    size = st.st_size;
    buffer = std::shared_ptr<unsigned char[]>( new unsigned char[ size ]);
    size_t readBytes = read( fd, buffer.get(), size );
    close( fd );
    return readBytes == size;
}

corestring::corestring() : std::string()
{
}

corestring::corestring( const std::string &src ) : std::string( src )
{
}

corestring::corestring( const char *src ) : std::string( src )
{
}

void corestring::formatva( const char *format, va_list &arg_list )
{
    if( format ) {
        va_list cova;
        va_copy( cova, arg_list );
#ifdef _WIN32
        int size = _vscprintf( format, cova );
#endif
#ifdef __linux
        int size = vsnprintf( NULL, 0, format, cova );
#endif
        va_end( arg_list );
        resize( size );
        va_copy( cova, arg_list );
        vsnprintf( &at( 0 ), size + 1, format, cova );
        va_end( arg_list );
    }
}

void corestring::format( const char *format, ... )
{
    if( format ) {
      va_list arg_list;
      va_start( arg_list, format );
      formatva( format, arg_list );
      va_end( arg_list );
    }
}

std::string corestring::tolower( std::string str )
{
    std::string result;
    for( auto chr : str ) {
        result.push_back( std::tolower( chr ));
    }
    return result;
}

long corestring::toLong()
{
    return std::stol( *this );
}

float corestring::toFloat()
{
    return std::stof( *this );
}

void start_log( const char *filename )
{
    timespec start_time;
    struct tm tm;
    clock_gettime( CLOCK_REALTIME, &start_time );
    tm = *localtime( &start_time.tv_sec );
    corestring logfilename;
    logfilename.format( "%s.log", filename );
//    logfilename.format( "%s-%04d-%02d-%02d-%02d-%02d-%02d.log", filename, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec );
#ifdef _WIN32
    log_file_id = ::open( logfilename.c_str(), O_CREAT | O_WRONLY | O_BINARY | O_TRUNC, 0666 );
#endif
#ifdef __linux
    log_file_id = ::open( logfilename.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666 );
#endif
}

void finish_log()
{
    ::close( log_file_id );
}

bool log_message( const char *format, ... )
{
  corestring output;
  corestring line;
  if( format ) {
    va_list arg_list;
    va_start( arg_list, format );
    line.formatva( format, arg_list );
    va_end( arg_list );
  }
  timespec start_time;
  struct tm tm;
  clock_gettime( CLOCK_REALTIME, &start_time );
  tm = *localtime( &start_time.tv_sec );
  output.format( "%04d-%02d-%02d-%02d:%02d:%02d.%06ld | %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, start_time.tv_nsec / 1000, line.c_str() );
  size_t size = write( log_file_id, output.c_str(), output.length() );
  return size == output.length();
}

int get_rnd( int min, int max ) {
    static unsigned int hash = clock();
    hash *= 0x372ce9b9;
    hash += 0xb9e92c37;
    if( min == max )
        return min;
    if( max > min )
        return min + int( hash % ( max - min ));
    else
        return max + int( hash % ( min - max ));
}

/* Copyright by János Klingl in 2023 */

#ifndef CORE_H
#define CORE_H

#ifdef _WIN32
#include <windows.h>
#endif
#include <string>
#include <stdarg.h>
#include <memory>
#include <chrono>

/* This class reads a file to the memory */

struct FileBuffer
{
    int size;
    std::shared_ptr<unsigned char[]> buffer;
    FileBuffer( const char *filename = nullptr );
    bool openAndRead( const char *filename );
};

/* This class provides extra funcionality to std::string */

class corestring : public std::string {
public:
    corestring();
    corestring( const std::string &src );
    corestring( const char *src );
    void formatva( const char *format, va_list &arg_list );
    void format( const char *format, ... );
    static std::string tolower( std::string str );
    long toLong();
    float toFloat();
};

/* These function are be used for logging */

void start_log( const char *filename );
void finish_log();
bool log_message( const char *format, ... );

/* This template class provides time measurement functions */

template <class TimeT = std::chrono::microseconds, class ClockT = std::chrono::steady_clock>
class Timer
{
    using timep_t = typename ClockT::time_point;
    timep_t from = ClockT::now();
public:
    void start() { from = ClockT::now(); }
    float duration() { return std::chrono::duration_cast<TimeT>( ClockT::now() - from ).count() * 1e-3; }
};

/* Used for generate pseudo random numbers */

extern int get_rnd( int min, int max );

#endif // CORE_H

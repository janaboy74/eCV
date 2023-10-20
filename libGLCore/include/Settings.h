/* Copyright by János Klingl in 2023 */

#ifndef SETTINGS_H
#define SETTINGS_H

#ifdef _WIN32
#include <windows.h>
#endif
#include <Core.h>
#include <string>
#include <stdarg.h>
#include <memory>
#include <map>

/* Settings file helper functions */

class Settings
{
    int size;
    std::map<std::string,std::string> variables;
public:
    Settings( const char *filename = nullptr );
    void open( const char *filename );
    corestring getVar( std::string name );
    corestring getVar( std::string name, std::string defaultValue );
};

#endif // SETTINGS_H

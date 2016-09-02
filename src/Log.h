#ifndef _LOG_H
#define _LOG_H

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>

// C includes
//
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#include <windows.h>
#endif


// ----------------------------------------------------------------------------
// Class
// ----------------------------------------------------------------------------

namespace SUPERSLEW {

  class Log {
  private:
    char   timestring[32];
    struct tm *t;  
    std::string time_format;
    timeval   tv;
    //std::ostringstream ostr;

  public:
    
    // Constructor.
    Log();
    
    // Destructor.
    ~Log() { };

    void xplm(const char*);
    void xplm(const std::string&);
    void xplm_lf();
    std::string ts();
  };
  extern Log lg;
}

#endif

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>

#include <stdlib.h>

#include "XPLMUtilities.h"

#include "Global.h"
#include "Log.h"

namespace SUPERSLEW {

  // ----------------------------------------------------------------------------
  // Code
  // ----------------------------------------------------------------------------

  Log::Log() {
    time_format = std::string("%Y%m%d %H:%M:%S");
  };
  
  void Log::xplm(const char* s) {
    std::string msg = ts()+": SLEW: "+std::string(s);
    XPLMDebugString( msg.c_str() );
  }
  void Log::xplm(const std::string& s) {
    std::string msg = ts()+": SLEW: "+s;
    XPLMDebugString( msg.c_str() );
  }
  
  void Log::xplm_lf() {
    XPLMDebugString( "\n" );
  }

#ifdef WIN32
  std::string Log::ts() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y%m%d@%X", &tstruct);

    return buf;
    //return "";
  }
#else
  std::string Log::ts() {
    gettimeofday(&tv, 0);
    t = localtime(&tv.tv_sec);
    strftime(timestring, 32, time_format.c_str(),  t);
    
    // We only display with milli-seconds accuracy.
    //
    int msec = (tv.tv_usec + 500) / 1000;
    std::ostringstream ostr;
    ostr << timestring << "." << std::setfill('0') << std::setw(3) << msec;
    return ostr.str();
  }
#endif
  Log lg;
}

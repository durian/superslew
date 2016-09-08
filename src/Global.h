#ifndef _GLOBAL_H
#define _GLOBAL_H

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>

#include "XPLMDataAccess.h"

// ----------------------------------------------------------------------------
// Class
// ----------------------------------------------------------------------------

namespace SUPERSLEW {

  class FloatWindow;

  std::string &ltrim(std::string &);
  std::string &rtrim(std::string &);
  std::string &trim(std::string &);
  void listify(const std::string&, std::vector<std::string>&);
  void split(const std::string&, std::vector<std::string>&);
    
  class Global {

  public:
    std::string          prefsfilename;
    std::string          keyfilename;
    bool                 loaded;
    bool                 valid_config;
        
    double elapsed;
    float dialog_interval_time;

    XPLMKeyFlags  gFlags = 0;
    XPLMKeyFlags  gPrevFlags = 0;
    unsigned char gVirtualKey = 0;
    unsigned char gChar = 0;

    std::string vkey;
    
    std::map<std::string, FloatWindow*> fwindows;

    double goto_lat = 999.9;
    double goto_lon = 999.9;
    double goto_alt = -1.0;
    double goto_psi = 999.9;
    double goto_phi = 999.9;
    double goto_the = 999.9;
    
    // Constructor.
    Global();
    
    // Destructor.
    ~Global();
    
    float random_variation(int);
    float random_range(float,float);
    std::string random_string(size_t);

    FloatWindow *create_fw(const std::string&, const std::string&, const std::string&, const std::string&);
    FloatWindow *create_fw(const std::string&, const std::string&, const std::string&, const std::string&, int, int, float);
    void update_text(const std::string&, const std::string&);
    void delete_fwindows();
  };
  
  extern Global G;
}
#endif

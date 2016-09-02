#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <random>

#include <stdlib.h>

#include "XPLMUtilities.h"

#include "Global.h"
#include "Log.h"
#include "dataref.h"
#include "sha256.h"
#include "float_window.h"

namespace SUPERSLEW {

  DataRef<float> dr_wind_altitude_msl_m0("sim/weather/wind_altitude_msl_m[0]");
  DataRef<float> dr_wind_direction_degt0("sim/weather/wind_direction_degt[0]");
  DataRef<float> dr_wind_speed_kt0("sim/weather/wind_speed_kt[0]");
  DataRef<float> dr_wind_altitude_msl_m1("sim/weather/wind_altitude_msl_m[1]");
  DataRef<float> dr_wind_direction_degt1("sim/weather/wind_direction_degt[1]");
  DataRef<float> dr_wind_speed_kt1("sim/weather/wind_speed_kt[1]");
  DataRef<float> dr_wind_altitude_msl_m2("sim/weather/wind_altitude_msl_m[2]");
  DataRef<float> dr_wind_direction_degt2("sim/weather/wind_direction_degt[2]");
  DataRef<float> dr_wind_speed_kt2("sim/weather/wind_speed_kt[2]");
  
  // ----------------------------------------------------------------------------
  // Code
  // ----------------------------------------------------------------------------

  // trim from start
  std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }
  
  // trim from end
  std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }
  
  // trim from both ends
  std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
  }
  
  int parse_int(const std::string& s) { 
    int n;
    std::istringstream(s) >> n;
    return n;
  }

  void listify(const std::string& s, std::vector<std::string>& v) {
    char delim = ',';
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      trim(item);
      v.push_back(item);
    }
  }

  // Default values if no config file found.
  Global::Global() {
  }

  Global::~Global() {
    lg.xplm( "Deleted Global.\n" );
  }


  float Global::random_variation(int variation) {
    // return 0.9-1.1 for random_variation(10) -> a multiplication factor
    float x = float((rand() % (2*variation) + (100-variation)) / (float)100.0);
    return x;
  }
  
  float Global::random_range(float l, float h) {
    static std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
    std::uniform_real_distribution<float> dis(l,h); // guaranteed unbiased
    auto r = dis(rng);
    return r;
  }

  std::string Global::random_string( size_t length ) {
    auto randchar = []() -> char {
      const char charset[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
      const size_t max_index = (sizeof(charset) - 1);
      return charset[ rand() % max_index ];
    };
    std::string str(length,0);
    std::generate_n( str.begin(), length, randchar );
    return str;
  }

  FloatWindow *Global::create_fw(const std::string& id, const std::string& title, const std::string& msg0, const std::string& msg1) {
    XPLMDataRef window_width_ref  = XPLMFindDataRef("sim/graphics/view/window_width");
    XPLMDataRef window_height_ref = XPLMFindDataRef("sim/graphics/view/window_height");
    
    if(nullptr == window_width_ref || nullptr == window_height_ref) {
      lg.xplm( "Couldn't open datarefs for window width and height\n" );
      return nullptr;
    }
    int width  = XPLMGetDatai(window_width_ref);
    int height = XPLMGetDatai(window_height_ref);
    
    const int window_width = 320;
    const int window_height = 80;
    
    int x  = width - window_width - 4;   // width/2 - window_width / 2;
    int x2 = width - 4;                  //width/2 + window_width / 2;
    int y  = window_height + 4;          //height/2 + window_height / 2;
    int y2 = +4;                         //height/2 - window_height / 2;
    
    FloatWindow *info_window = new FloatWindow(x, y, x2, y2, title, msg0, msg1);
    fwindows[id] = info_window;
    return info_window;
    // return this pointer too?
  }

  FloatWindow *Global::create_fw(const std::string& id, const std::string& title, const std::string& msg0, const std::string& msg1, int x, int y, float tmr) {
    const int window_width = 320;
    const int window_height = 80;
    
    // x,y are the top left of window
    int x2 = x + window_width;
    int y2 = y - window_height;
    
    FloatWindow *info_window = new FloatWindow(x, y, x2, y2, title, msg0, msg1, tmr);
    fwindows[id] = info_window; // check if exists? then delete old one?
    return info_window;
  }

  void Global::update_text(const std::string& id, const std::string& t) {
    FloatWindow *fw = fwindows[id];
    fw->updateText(t);
  }

  void Global::delete_fwindows() {
    for(auto const &i : fwindows) {
      delete fwindows[i.first];
    }
    fwindows.clear();
  }
  
  Global G;
}

// The End --------

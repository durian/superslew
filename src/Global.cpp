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

  void split(const std::string& s, std::vector<std::string>& v) {
    char delim = ' ';
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
      trim(item);
      if ( item.size() > 1 ) {
	v.push_back(item);
      }
    }
  }

  // Default values if no config file found.
  Global::Global() {
    speeds[0] =   16.0;
    speeds[1] =  256.0;
    speeds[2] = 1024.0;    
  }

  Global::~Global() {
    lg.xplm( "Deleted Global.\n" );
  }

  void Global::read_prefs(std::string& filename) {
    std::ifstream file( filename.c_str() );
    if ( ! file ) {
      // write a default one
      std::ofstream file( filename.c_str(), std::ios::out );
      if ( ! file ) {
	lg.xplm("ERROR: can not write config file.\n");
	return;
      }
      file << "altmode = 0" << std::endl;
      file << "orimode = 0" << std::endl;
      file << "speeds = 16,256,1024" << std::endl;
      file << "speed = 0" << std::endl;
      file.close();
      return;
    }
    std::string a_line;
    while( std::getline( file, a_line )) {
      if ( a_line.length() == 0 ) {
	continue;
      }
      if ( a_line.at(0) == '#' ) {
	continue;
      }

      size_t pos = a_line.find( '=', 0 );
      if ( pos != std::string::npos ) {
	std::string lhs = a_line.substr( 0, pos );
	trim(lhs);
	std::string rhs = a_line.substr( pos+1 );
	trim(rhs);
	if ( (lhs != "") && (rhs != "") ) {
	  std::string tmp = lhs +"="+rhs+"\n";
	  lg.xplm( tmp );
	  // 
	  if ( lhs == "altmode" ) {
	    int x = int(std::stoi(rhs));
	    if ( x == 1 ) {
	      G.altmode = true;
	    }
	  }
	  if ( lhs == "orimode" ) {
	    int x = int(std::stoi(rhs));
	    if ( x == 1 ) {
	      orimode = true;
	    }
	  }
	  if ( lhs == "speeds" ) {
	    std::vector<std::string> bits;
	    listify( rhs, bits );
	    if ( bits.size() >= 3 ) {
	      float s0 =   16.0; // y is under
	      float s1 =  256.0; // z is behind
	      float s2 = 1024.0; // z is behind
	      try {
		s0 = fabs(std::stof(bits[0]));
		if ( s0 < 0.01 ) {
		  s1 = 1.0;
		}
		s1 = fabs(std::stof(bits[1]));
		s2 = fabs(std::stof(bits[2]));
	      } catch (...) {
		lg.xplm( "Can't read speeds values, using defaults.\n" );
	      }
	      speeds[0] = s0;
	      speeds[1] = s1;
	      speeds[2] = s2;
	    }
	  }
	  if ( lhs == "speed" ) {
	    int x = int(std::stoi(rhs));
	    speed = x;
	  }
	}
      }
    }
  }

  void Global::write_prefs(std::string& filename) {
    std::ofstream file( filename.c_str(), std::ios::out );
    if ( ! file ) {
      lg.xplm("ERROR: can not write config file.\n");
      return;
    }
    file << "altmode = ";
    if ( G.altmode ) {
      file << "1" << std::endl;
    } else {
      file << "0" << std::endl;
    }
    file << "orimode = ";
    if ( G.orimode ) {
      file << "1" << std::endl;
    } else {
      file << "0" << std::endl;
    }
    file << "speeds = " << G.speeds[0] << "," << G.speeds[1] << "," << G.speeds[2] << std::endl;
    file << "speed = " << G.speed << std::endl;
    file.close();
    return;
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
  
  FloatWindow *Global::create_fw(const std::string& id) {
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

    //  For X-Plane 8.10 and later, 0,0 is at the lower left corner of the screen.
	 
    int x, x2, y, y2;
    // windw_x, window_y
    // window_y includes menu, so it should not be less than 16 from top!
    // 20160923 18:33:30.261: SLEW: WIN UP x=1175 y=882
    if ( window_x < 0 ) {
      x  = width - window_width - window_x;
      x2 = width - window_x;
    } else {
      x  = window_x;
      x2 = window_x + window_width;
    }
    if ( window_y < 0 ) {
      y  = window_height - window_y; 
      y2 = -window_y;
    } else {
      //y2 = height - window_y - window_height;
      //y = height - window_y;
      y = window_y;
      y2 = window_y - window_height;
    }
    /*
    int x  = width - window_width - 4;   // width/2 - window_width / 2;
    int x2 = width - 4;                  //width/2 + window_width / 2;
    int y  = window_height + 4;          //height/2 + window_height / 2;
    int y2 = +4;                         //height/2 - window_height / 2;
    */
    
    FloatWindow *info_window = new FloatWindow(x, y, x2, y2, "", "", "");
    fwindows[id] = info_window;
    return info_window;
    // return this pointer too?
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

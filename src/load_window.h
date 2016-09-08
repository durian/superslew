#ifndef _WINDOW_H
#define _WINDOW_H

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <vector>

#include "XPLMDataAccess.h"

namespace SUPERSLEW {

  void destroy_load_window();
  void create_load_window(int, int, int, int);
  void hide_load_window();
  void close_load_window();
  int num_edit_handler(XPWidgetMessage, XPWidgetID, intptr_t, intptr_t);
  int load_window_draw_callback(XPWidgetMessage, XPWidgetID, intptr_t, intptr_t);
  int load_window_callback(XPWidgetMessage, XPWidgetID, intptr_t, intptr_t);  
}
#endif

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <ctime>
#include <chrono>
#include <map>

#include <stdlib.h>

#include "XPLMUtilities.h"
#include "XPLMGraphics.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMDisplay.h"
#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMCamera.h"
#include "XPUIGraphics.h"
#include "XPWidgetUtils.h"

#include "load_window.h"
#include "Log.h"
#include "Global.h"
#include "dataref.h"

namespace SUPERSLEW {

  XPLMWindowID load_window = nullptr;
  XPLMWindowID load_info_window = nullptr;
  XPWidgetID info_label0;
  XPWidgetID num_edit_label;
  XPWidgetID num_edit;
  XPWidgetID button_ok;
  XPWidgetID button_cancel;
  
  // ----------------------------------------------------------------------------
  // Code
  // ----------------------------------------------------------------------------

  void destroy_load_window() {
    if ( load_window != nullptr ) {
      XPDestroyWidget( load_window, 1 );
      load_window = nullptr;
    }
  }
  
  void create_load_window(int win_x, int win_y, int w, int h) {
    if ( load_window != nullptr ) {
      if ( ! XPIsWidgetVisible(load_window) ) {
	XPShowWidget(load_window);
      } else {
	XPHideWidget(load_window);
      }
      return;
    }
    int x2 = win_x + w;
    int y2 = win_y - h;

    // Top, left, bottom, and right in global screen coordinates defining the widget's location on the screen.
    load_window = XPCreateWidget(win_x, win_y, x2, y2, 1, "Go to position", 1, NULL, xpWidgetClass_MainWindow);

    // Create the Sub Widget1 window
    // 0,0 in the bottom left and +y = up, +x = right. top left?
    // LTRB
    //                                 >right  v down        <left   ^up
    int mbar = 20;
    int xl = win_x+8;       // x left
    int yt = win_y-mbar-8;  // y top
    int xr = x2-8;      // x right
    int yb = y2+8;      // y bottom
    load_info_window = XPCreateWidget(xl, yt, xr, yb, 1, "", 0, load_window, xpWidgetClass_SubWindow);
    XPSetWidgetProperty(load_info_window, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
    
    int x  = xl;
    int y  = yt;
    int r  =  0;  // "row" 0-1
    int rh = -24; // row height
    int in =   4; // (button) inset
    
    //200 is left margin
    num_edit_label = XPCreateWidget(x, y+(r*rh), x+188, y+((r+1)*rh), 1, "Coordinates", 0, load_window, xpWidgetClass_Caption);
    std::string one = "53.4534 5.6729"; // padded(1, 4); //padded looks silly
    num_edit = XPCreateWidget(x+204, y+(r*rh), x+348, y+((r+1)*rh), 1, one.c_str(), 0, load_window, xpWidgetClass_TextField);
    XPAddWidgetCallback(num_edit, num_edit_handler); //load_window_callback);
    
    // Row with buttons
    r += 1;
    x  = xl+8;
    y  = yt;
    int bw = 88;

    button_ok = XPCreateWidget(x, y+(r*rh), x+bw, y+((r+1)*rh), 1, "Go", 0, load_window, xpWidgetClass_Button);
    XPSetWidgetProperty(button_ok, xpProperty_ButtonType, xpPushButton);    // Set it to be normal push button
    
    x += bw+8;
    button_cancel = XPCreateWidget(x, y+(r*rh), x+bw, y+((r+1)*rh), 1, "Cancel", 0, load_window, xpWidgetClass_Button);
    XPSetWidgetProperty(button_cancel, xpProperty_ButtonType, xpPushButton);    // Set it to be normal push button

    // ---

    // Now set the height of the widget to where we are.
    int left, top, right, bottom;
    XPGetWidgetGeometry(load_window, &left, &top, &right, &bottom);
    int height = yt - ((r+1)*rh);
    
    XPSetWidgetProperty(load_window, xpProperty_MainWindowHasCloseBoxes, 1);
    //XPSetWidgetProperty(load_window, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
    XPAddWidgetCallback(load_window, load_window_callback);
  }

  void hide_load_window() {
    XPHideWidget(load_window);
  }
  void close_load_window() {
    XPHideWidget(load_window);
    XPLMDestroyWindow(load_window);
    load_window = nullptr;
  }

  // Called continuously when the window is visible.
  int load_window_draw_callback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t, intptr_t) {
    switch(inMessage) {
    case xpMsg_Draw:
      return 1;
    }
    return 0;
  }

  int num_edit_handler(XPWidgetMessage message, XPWidgetID widget_id, intptr_t param1, intptr_t param2) {
    if ( message == xpMsg_KeyPress ) {
      unsigned char theChar = KEY_CHAR(param1);
      unsigned char theKey = KEY_VKEY(param1);
      bool upKey = KEY_FLAGS(param1) & xplm_UpFlag;
      bool downKey = KEY_FLAGS(param1) & xplm_DownFlag;
      //lg.xplm("KEY "+std::to_string(theKey)+"\n" );
      // ignore all that is not 0-9 or DEL/LEFT/RIGHT
      if (theChar == '\n' || theChar == '\r' || theChar == 9 || theKey == XPLM_VK_NUMPAD_ENT) {
	XPLoseKeyboardFocus(widget_id);
	return 1;
      }
      if ( (theKey >= 48) && (theKey <= 57) ) { //num keys
	return 0;
      }
      if ( (theKey >= 96) && (theKey <= 105) ) { //num keypad
	return 0;
      }
      if ( (theKey == XPLM_VK_DELETE) || (theKey == XPLM_VK_LEFT) || (theKey == XPLM_VK_RIGHT) ) {
	return 0;
      }
      if ( (theChar == '.') || (theChar == ',') || (theChar == ' ') || (theChar == '-') ) {
	return 0;
      }
      if ( (theChar == 37) || (theChar == 39) || (theChar == 118) || (theChar == 8)) {
	return 0;
      }
      return 1;
    } // msg == KeyPress
    return 0;
  }
  
  int load_window_callback(XPWidgetMessage message, XPWidgetID widget_id, intptr_t param1, intptr_t param2) {
    if (message == xpMessage_CloseButtonPushed) {
      hide_load_window();
      return 1;
    }
    if (message == xpMsg_PushButtonPressed) {
      if ( (long)param1 == (long)button_ok) {
	//lg.xplm("push OK\n");
	char numcstr[1024];
	XPGetWidgetDescriptor(num_edit, numcstr, 1023 );
	std::string numstr = std::string( numcstr );
	//lg.xplm( "numstr="+numstr+"\n" );
	std::vector<std::string> vec;
	split(numcstr, vec);
	/*
	lg.xplm( "len="+std::to_string(vec.size())+"\n" );
	for( int i = 0; i < vec.size(); i++ ) {
	  lg.xplm( vec[i]+"\n" );
	}
	*/
	//
	if ( vec.size() == 2 ) {
	  G.goto_lat = std::stod(vec[0]); // 0--90, north
	  G.goto_lon = std::stod(vec[1]); // east west
	  G.goto_alt = 0.1;
	}
	hide_load_window();
      }

      if ( (long)param1 == (long)button_cancel) {
	//lg.xplm("push cancel\n");
	hide_load_window();
      }
    } 
    return 0;
  }
  
}

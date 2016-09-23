#pragma once

#include <chrono>

#include "XPLMDisplay.h"
#include "XPCWidget.h"
#include "XPStandardWidgets.h"
#include "XPLMDataAccess.h"
#include "XPLMGraphics.h"
#include "XPWidgetUtils.h"

#include "dataref.h"
#include "Log.h"

#define DBG 1

const intptr_t MSG_END_SLEWMODE = 0x01000000;

namespace SUPERSLEW {

  class FloatWindow {

    int bottom_row_height   = 20;
    int right_col_width     = 20;
    int toggle_button_width = 28;
    int titlebar_height     = 20;
    
    XPLMWindowID info_window = nullptr;
    XPWidgetID   custom_text = nullptr;
    int          fontheight;
    XPLMFontID   font = xplmFont_Basic;
    int          x, y, x2, y2;
    std::chrono::system_clock::time_point now; //= std::chrono::system_clock::now();
    std::chrono::system_clock::time_point lastnow; // = now;
    float        timer;
    
    static int FloatWindowCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t) {
      XPMouseState_t *mouse_info = (XPMouseState_t*)inParam1;
      FloatWindow     *obj        = (FloatWindow*)XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr);
      
      switch(inMessage) {
	
      case xpMsg_MouseDown: {
	/*
	XPGetWidgetGeometry(obj->info_window, &obj->drag_start_window_left, &obj->drag_start_window_top,
			    &obj->drag_start_window_right, &obj->drag_start_window_bottom);
	int click_y_offset_from_list_top = obj->drag_start_window_top - mouse_info->y - titlebar_height;
	if(click_y_offset_from_list_top < 0) {	//click in title bar
	  return 0; // we can move window
	}
	*/
	lg.xplm("WIN x="+std::to_string(mouse_info->x)+" y="+std::to_string(mouse_info->y)+"\n" );
	return 0;
      }
      case xpMsg_MouseUp: {
	XPGetWidgetGeometry(obj->info_window, &obj->drag_start_window_left, &obj->drag_start_window_top,
			    &obj->drag_start_window_right, &obj->drag_start_window_bottom);
	//lg.xplm("WIN UP x="+std::to_string(obj->drag_start_window_left)+" y="+std::to_string(obj->drag_start_window_top)+"\n" );
	// save these in G, for config file... or get from G on save, we don't have G here.
	return 0;
      }

      case xpMessage_CloseButtonPushed:
	//closeFloatWindow(obj); // not this! hide! or via windows Set. remove from set?
	//XPLMSetWindowIsVisible(obj->info_window, 0);
	lg.xplm("ping\n");
	XPHideWidget(inWidget); // the question is how to get it back...

	XPLMPluginID plugin_id = XPLMFindPluginBySignature("org.durian.superslew");
	if (XPLM_NO_PLUGIN_ID != plugin_id) {
	  XPLMSendMessageToPlugin(plugin_id, MSG_END_SLEWMODE, (void*)"stop"); // is "stop" even needed?
	}
	
	return 1;

      }
      return 0;
    }

    // Called continuously when the window is visible.
    static int drawCallback(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t, intptr_t) {
      switch(inMessage) {
      case xpMsg_Draw:
	FloatWindow *obj = (FloatWindow*)XPGetWidgetProperty(inWidget, xpProperty_Object, nullptr);
	if ( obj ) {
	  obj->draw();
	}
	return 1;
      }
      return 0;
    }
    
  public:

    int drag_start_window_left = 0;
    int drag_start_window_right = 0;
    int drag_start_window_top = 0;
    int drag_start_window_bottom = 0;
    std::string title;
    std::string msg0;
    std::string msg1;
    std::string msg2;

    FloatWindow(int x, int y, int x2, int y2, std::string t, std::string m0, std::string m1, float tmr = -1.0) {
      title = t;
      msg0 = m0;
      msg1 = m1;
      msg2 = "";
      timer = tmr; // < 0 for always, > 0 is seconds before close
      now = std::chrono::system_clock::now();
      lastnow = now;
      XPLMGetFontDimensions(font, nullptr, &fontheight, nullptr);
      info_window = XPCreateWidget(x, y, x2, y2,
				   1, //visible
				   title.c_str(),
				   1, //root
				   NULL,// no container
				   xpWidgetClass_MainWindow);

      XPSetWidgetProperty(info_window, xpProperty_MainWindowHasCloseBoxes, 1);
      XPSetWidgetProperty(info_window, xpProperty_MainWindowType, xpMainWindowStyle_Translucent);
      XPAddWidgetCallback(info_window, FloatWindowCallback);
      XPSetWidgetProperty(info_window, xpProperty_Object, (intptr_t)this); //so we find it in static funxions

      custom_text = XPCreateCustomWidget(x+4, y-4, x2-4, y2+4, 1,"", 0, info_window, drawCallback);
      XPSetWidgetProperty(custom_text, xpProperty_Object, (intptr_t)this); // store this to be able to close from draw()
      }

    ~FloatWindow() {
#ifdef DBG
      lg.xplm("XPLMDestroyWindow(info_window)\n");
#endif
      XPHideWidget(info_window);
      XPLMDestroyWindow(info_window);
    }

    // Called when windows visible, draw the text here.
    // Closes itself after "timer" seconds.
    void draw() {
      int left, top, right, bottom;
      float color[3] = {0.2f, 1.f, 1.f}; //blueish
      XPGetWidgetGeometry(custom_text, &left, &top, &right, &bottom);
      XPLMDrawString(color, left + 5, top - titlebar_height - 14, (char*)msg0.c_str(), NULL, xplmFont_Basic);
      XPLMDrawString(color, left + 5, top - titlebar_height - (2*14), (char*)msg1.c_str(), NULL, xplmFont_Basic);
      XPLMDrawString(color, left + 5, top - titlebar_height - (3*14), (char*)msg2.c_str(), NULL, xplmFont_Basic); 
      now = std::chrono::system_clock::now();
      float timediff = std::chrono::duration_cast<std::chrono::milliseconds>(now-lastnow).count();
#ifdef TMR
      XPLMDrawNumber(color, left + 5, top - titlebar_height - (3*14), timer-(timediff/1000), 2, 1, 1, xplmFont_Basic);
#endif
      if ( (timer > 0) && (timediff >= (timer*1000)) ) {
#ifdef DBG
	lg.xplm("FloatWindow draw(), time's up.\n");
#endif
	FloatWindow * obj = (FloatWindow*) XPGetWidgetProperty(custom_text, xpProperty_Object, nullptr);
      }
    }

    void updateText(const std::string &s) {
      msg0 = s;
    }
    void updateText1(const std::string &s) {
      msg1 = s;
    }
    void updateText2(const std::string &s) {
      msg2 = s;
    }

    void showWindow() {
      XPShowWidget(info_window);
    }

    void hideWindow() {
      XPHideWidget(info_window);
    }
    
  }; // class def

  // should be moved to Global.cpp
  // to f_w.cpp?
  
  // a new ShowFW with an ID, which is stored in a G.windows structure, and then we can
  // do G.show(ID), G.hide(ID), Gupdate_text(ID, ...)
}

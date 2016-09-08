/*
  (c) pjb 2013, 2014, 2015, 2016
*/

#if IBM
#include <windows.h>
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
#endif

#define M_PI           3.14159265358979323846

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <chrono>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "XPLMProcessing.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMPlugin.h"
#include "XPLMMenus.h"
#include "XPLMGraphics.h"
#include "XPLMScenery.h"
#include "XPLMCamera.h"
#include "XPLMPlanes.h"

#include "dataref.h"
#include "main.h"
#include "Global.h"
#include "Log.h"
#include "float_window.h"
#include "load_window.h"

using namespace SUPERSLEW;

std::string VERSION = "0.9";

DataRef<int>    dr_sim_paused("sim/time/paused");
DataRef<int>    dr_sim_speed("sim/time/sim_speed", ReadWrite);
DataRef<float>  dr_sim_trts("sim/time/total_running_time_sec");
DataRef<float>  dr_sim_tfts("sim/time/total_flight_time_sec");

DataRef<int>    dr_override_flightcontrol("sim/operation/override/override_flightcontrol", ReadWrite);
DataRef<float>  dr_FC_ptch("sim/joystick/FC_ptch");
DataRef<float>  dr_FC_roll("sim/joystick/FC_roll");
DataRef<float>  dr_FC_hdng("sim/joystick/FC_hdng");

typedef struct _Axis {
  double idx;
  double min;
  double max;
} Axis;
DataRef<std::vector<int>> dr_jsa_assignments("sim/joystick/joystick_axis_assignments"); // axis type  int[100]
DataRef<std::vector<float>> dr_jsa_values("sim/joystick/joystick_axis_values"); // float[100]
DataRef<std::vector<float>> dr_jsa_minimum("sim/joystick/joystick_axis_minimum"); // float[100]
DataRef<std::vector<float>> dr_jsa_maximum("sim/joystick/joystick_axis_maximum"); // float[100]
Axis axis_p = {0, 0, 0};
Axis axis_r = {0, 0, 0};
Axis axis_y = {0, 0, 0};
Axis axis_t = {0, 0, 0}; // throttle

static bool slewmode = false;
static bool altmode = false;
static bool orimode = false; // orientation

float mini_mult = 16.0; // this is m/s, at max throttle
float maxi_mult = 16.0 * mini_mult; // 16*16 = 256 = ~500 kt
float warp_mult =  4.0 * maxi_mult; // 4 * 500 = ~2000 kt
float mult = mini_mult;

float yaw_reverse = 1.0; // set to -1.0 to reverse.

DataRef<std::vector<int>> dr_override_planepath("sim/operation/override/override_planepath", ReadWrite); // ARRAY?

DataRef<double> dr_plane_lx("sim/flightmodel/position/local_x", ReadWrite);
DataRef<double> dr_plane_ly("sim/flightmodel/position/local_y", ReadWrite);
DataRef<double> dr_plane_lz("sim/flightmodel/position/local_z", ReadWrite);

DataRef<float> dr_plane_vx("sim/flightmodel/position/local_vx", ReadWrite);
DataRef<float> dr_plane_vy("sim/flightmodel/position/local_vy", ReadWrite);
DataRef<float> dr_plane_vz("sim/flightmodel/position/local_vz", ReadWrite);

DataRef<float> dr_plane_ax("sim/flightmodel/position/local_ax", ReadWrite);
DataRef<float> dr_plane_ay("sim/flightmodel/position/local_ay", ReadWrite);
DataRef<float> dr_plane_az("sim/flightmodel/position/local_az", ReadWrite);

DataRef<float> dr_plane_y_agl("sim/flightmodel/position/y_agl");
float reference_h = 0.0;

DataRef<float>  dr_plane_psi("sim/flightmodel/position/psi", ReadWrite);
DataRef<float>  dr_plane_true_psi("sim/flightmodel/position/true_psi");
DataRef<float>  dr_plane_the("sim/flightmodel/position/theta", ReadWrite);
DataRef<float>  dr_plane_phi("sim/flightmodel/position/phi", ReadWrite);

DataRef<double>  dr_plane_lat("sim/flightmodel/position/latitude");
DataRef<double>  dr_plane_lon("sim/flightmodel/position/longitude");

DataRef<std::vector<float>> dr_plane_q("sim/flightmodel/position/q", ReadWrite);

/*
fside_prop	float	660+	yes	Newtons	force sideways by all engines on the ACF. Override with override_engines
fnrml_prop	float	660+	yes	Newtons	force upward by all engines on the ACF. Override with override_engines Writable in v10 only
faxil_prop	float	660+	yes	Newtons	force backward by all engines on the ACF (usually this is a negative number). 

fside_total	float	1000+	yes	Newtons	total/ground forces - ACF X axis. Override with override_forces
fnrml_total	float	1000+	yes	Newtons	Total/ground forces - ACF Y axis. Override with override_forces
faxil_total	float	1000+	yes	Newtons	total/ground forces - ACF Z axis. Override with override_forces

L_prop	float	1000+	yes	NM	The roll moment due to prop forces. Override with override_engines - positive = right roll.
M_prop	float	1000+	yes	NM	The pitch moment due to prop forces. Override with override_engines - positive = pitch up.
N_prop	float	1000+	yes	NM	The yaw moment due to prop forces. Override with override_engines - positive = yaw right/clockwise.

// these?
fside_plug_acf	float	1030+	yes	Newtons	Extra plugin-provided sideways force (ACF X axis, positive pushes airplane to the right). ADD to this dataref to apply extra force.
fnrml_plug_acf	float	1030+	yes	Newtons	Extra plugin-provided upward force (ACF Y axis, positive pushes airplane up). ADD to this dataref to apply extra force.
faxil_plug_acf	float	1030+	yes	Newtons	Extra plugin-provided forward force. (ACF Z axis, positive pushes airplane backward). ADD to this dataref to apply extra force.
L_plug_acf	float	1030+	yes	NM	Extra plugin-provided roll moment - ADD to this dataref to apply extra force - positive = right roll.
M_plug_acf	float	1030+	yes	NM	Extra plugin-provided pitch moment - ADD to this dataref to apply extra force - positive = pitch up.
N_plug_acf	float	1030+	yes	NM	Extra plugin-provided yaw moment - ADD to this dataref to apply extra force - positive = yaw right/clockwise.

// sim/operation/override/
override_joystick	int	660+	yes	boolean	Override control of the joystick deflections (overrides stick, yoke, pedals, keys, mouse, and auto-coordination)
override_artstab	int	660+	yes	boolean	Override control of the artificial stability system
override_flightcontrol	int	660+	yes	boolean	Override all parts of the flight system at once
override_gearbrake	int	660+	yes	boolean	Override gear and brake status
override_planepath	int[20]	660+	yes	boolean	Override position updates of this plane
...
override_engines	int	1000+	yes	boolean	overrides all engine calculations - write to LMN and g_nrml/side/axil.
override_forces	int	1000+	yes	boolean	overrides all force calculations - write to LMN and g_nrml/side/axil.
*/

FloatWindow *infow = nullptr;

XPLMMenuID	myMenu;
int		mySubMenuItem;

enum gpxlog_status {MENU_TOGGLE, MENU_TOGGLE_ALTMODE, MENU_TOGGLE_ORIMODE, MENU_SCAN, MENU_YAWREVERSE, MENU_GOTO, MENU_NORMAL, MENU_SPEED, MENU_WARP};

extern const intptr_t MSG_END_SLEWMODE; //from float_window.h

XPLMCommandRef  SlewCommand;
int SlewCommandHandler(XPLMCommandRef, XPLMCommandPhase, void *);

// quaternion library
//http://forums.x-plane.org/index.php?/forums/topic/52920-plane-movement-and-physics-engine-toggling/

#define DEG_TO_RAD_2 M_PI / 360.0
typedef struct _Eulers {
  double psi;
  double the;
  double phi;
} Eulers;

typedef struct _Quaternion {
  double w;
  double x;
  double y;
  double z;
} Quaternion;
void EulersToQuaternion(Eulers ypr, Quaternion &q) {
    double spsi = sin(ypr.psi * DEG_TO_RAD_2);
    double sthe = sin(ypr.the * DEG_TO_RAD_2);
    double sphi = sin(ypr.phi * DEG_TO_RAD_2);
    double cpsi = cos(ypr.psi * DEG_TO_RAD_2);
    double cthe = cos(ypr.the * DEG_TO_RAD_2);
    double cphi = cos(ypr.phi * DEG_TO_RAD_2);

    double qw = cphi * cthe * cpsi + sphi * sthe * spsi;
    double qx = sphi * cthe * cpsi - cphi * sthe * spsi;
    double qy = sphi * cthe * spsi + cphi * sthe * cpsi;
    double qz = cphi * cthe * spsi - sphi * sthe * cpsi;

    double e = sqrt(qw * qw + qx * qx + qy * qy + qz * qz);

    q.w = qw / e;
    q.x = qx / e;
    q.y = qy / e;
    q.z = qz / e;
}


#define RAD_TO_DEG 180.0 / M_PI
double sgn(double a) {
    if (a < 0.0) return -1.0;
    else return 1.0;
}
void QuaternionToEulers(Quaternion q, Eulers &ypr) {
    double sx = q.x * q.x;
    double sy = q.y * q.y;
    double sz = q.z * q.z;
    double yz = q.y * q.z;
    double wx = q.w * q.x;

    double m00 = 1.0 - 2.0 * (sy + sz);
    double m10 = 2.0 * (q.x * q.y + q.w * q.z);
    double m11 = 1.0 - 2.0 * (sx + sz);
    double m12 = 2.0 * (yz - wx);
    double m20 = 2.0 * (q.x * q.z - q.w * q.y);
    double m21 = 2.0 * (wx + yz);
    double m22 = 1.0 - 2.0 * (sx + sy);

    double s = -m20;
    double c = sqrt(m00 * m00 + m10 * m10);

    ypr.the = atan2(s, c) * RAD_TO_DEG;

    if (c > 0.001) {
      ypr.phi = atan2(m21, m22) * RAD_TO_DEG;
      ypr.psi = atan2(m10, m00) * RAD_TO_DEG;
    } else {
      ypr.phi = 0.0;
      ypr.psi = -sgn(s) * atan2(-m12, m11) * RAD_TO_DEG;
    }
}

static XPLMProbeRef hProbe = XPLMCreateProbe(xplm_ProbeY);
float height(double x, double y, double z) {
  XPLMProbeInfo_t info = { 0 };
  info.structSize = sizeof(info);  
  // If we have a hit then return Y coordinate
  if (XPLMProbeTerrainXYZ( hProbe, x, y, z, &info) == xplm_ProbeHitTerrain) {
    return info.locationY;
  }
  return -1.0;
}

double distance(double lat0, double lon0, double lat1, double lon1) {
  double slat = sin((lat1-lat0) * (double)(M_PI/360));
  double slon = sin((lon1-lon0) * (double)(M_PI/360));
  double aa   = slat*slat + cos(lat0 * (double)(M_PI/180)) * cos(lat1 * (double)(M_PI/180)) * slon * slon;
  return 6378145.0 * 2 * atan2(sqrtf(aa), sqrt(1-aa));
}

float axis_scaled(Axis a) {
  float val = dr_jsa_values[a.idx];
  float range = a.max - a.min;
  float pot = (val - a.min) / range;
  return pot;
}

std::string rounded(float number) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2) << number;
  return ss.str();
}
std::string rounded6(float number) {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(6) << number;
  return ss.str();
}
std::string padded_int(int i, int l) {
  std::ostringstream ostr;
  std::string res;
  ostr << i; // get number only, we need double spaces for menu
  res = ostr.str();
  if ( res.length() >= l ) {
    return res;
  }
  int d = l - res.length();
  return std::string(d*2, ' ') + res;
}

void scan_joy() {
  for (int i = 0; i < 100; i++) {
    if ( dr_jsa_assignments[i] == 1 ) {
      lg.xplm("dr_jsa_assignments 1 (pitch) = "+std::to_string(i)+"\n");
      float min = dr_jsa_minimum[i];
      float max = dr_jsa_maximum[i];
      lg.xplm("dr_jsa_assignments min="+std::to_string(min)+", max="+std::to_string(max)+"\n");
      if ( max-min > 0.5 ) {
	axis_p.idx = i; // pitch
	axis_p.min = min;
	axis_p.max = max;
      }
    }
    if ( dr_jsa_assignments[i] == 2 ) {
      lg.xplm("dr_jsa_assignments 2 (roll) = "+std::to_string(i)+"\n");
      float min = dr_jsa_minimum[i];
      float max = dr_jsa_maximum[i];
      lg.xplm("dr_jsa_assignments min="+std::to_string(min)+", max="+std::to_string(max)+"\n");
      if ( max-min > 0.5 ) {
	axis_r.idx = i;  //roll
	axis_r.min = min;
	axis_r.max = max;
      }
    }
    if ( dr_jsa_assignments[i] == 3 ) { // pedal is 4?
      lg.xplm("dr_jsa_assignments 3 (yaw) = "+std::to_string(i)+"\n");
      float min = dr_jsa_minimum[i];
      float max = dr_jsa_maximum[i];
      lg.xplm("dr_jsa_assignments min="+std::to_string(min)+", max="+std::to_string(max)+"\n");
      if ( max-min > 0.5 ) {
	axis_y.idx = i; // yaw
	axis_y.min = min;
	axis_y.max = max;
      }
    }
    if ( dr_jsa_assignments[i] == 4 ) { //throttle
      lg.xplm("dr_jsa_assignments 4 (throttle) = "+std::to_string(i)+"\n");
      float min = dr_jsa_minimum[i];
      float max = dr_jsa_maximum[i];
      lg.xplm("dr_jsa_assignments min="+std::to_string(min)+", max="+std::to_string(max)+"\n");
      if ( max-min > 0.5 ) {
	axis_t.idx = i; // yaw
	axis_t.min = min;
	axis_t.max = max;
      }
    }
  } 
}

void slew_disable() {
  dr_override_planepath = { 0,0 }; // works on windows... should "get" the second value first?
  XPLMCheckMenuItem(myMenu, MENU_TOGGLE, xplm_Menu_Unchecked);
  infow->hideWindow();
  slewmode = false;
}

PLUGIN_API int XPluginStart(char *outName, char *outSig, char *outDesc) {
  /*
  static std::chrono::system_clock::time_point lastnow = std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds> tp;
  tp = std::chrono::time_point_cast<std::chrono::seconds>(lastnow);
  std::time_t now_c = std::chrono::system_clock::to_time_t(tp);
  std::string now_c_str = std::string(std::ctime(&now_c));
  lg.xplm(now_c_str+"\n");
  */
  strcpy(outName, "SuperSlew");
  strcpy(outSig,  "org.durian.superslew");
  strcpy(outDesc, "A plugin (superslew).");
  std::string compile_date("SUPERSLEW plugin compiled on " __DATE__ " at " __TIME__ "\n");
  lg.xplm( compile_date.c_str() );
  
  XPLMEnableFeature("XPLM_USE_NATIVE_PATHS", 1);  

  char filebase[255];
  XPLMGetSystemPath(filebase); // Locate the X-System directory
  const char *sep = XPLMGetDirectorySeparator();

  // The initialisation file
  std::string prefsfile = std::string(filebase) + "Resources" + sep + "plugins" + sep+ "superslew" + sep + "drop.ini";  
  G.prefsfilename = prefsfile;

  // First we put a new menu item into the plugin menu
  mySubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), // Plugins menu 
				     "SuperSlew", // Menu title
				     0,  // Item ref
				     1); // English 

  // Now create a submenu attached to our menu items
  myMenu = XPLMCreateMenu("SLEW",
			  XPLMFindPluginsMenu(),
			  mySubMenuItem, /* Menu Item to attach to. */
			  MyMenuHandlerCallback,/* The handler */
			  0);			/* Handler Ref */


  XPLMAppendMenuItem(myMenu, "Toggle Slew Mode",  (void*)MENU_TOGGLE, 1);
  XPLMCheckMenuItem(myMenu, MENU_TOGGLE, xplm_Menu_Unchecked);

  XPLMAppendMenuItem(myMenu, "Toggle Altitude Control",  (void*)MENU_TOGGLE_ALTMODE, 1);
  if ( altmode ) {
    XPLMCheckMenuItem(myMenu, MENU_TOGGLE_ALTMODE, xplm_Menu_Checked);
  } else {
    XPLMCheckMenuItem(myMenu, MENU_TOGGLE_ALTMODE, xplm_Menu_Unchecked);
  }
  XPLMAppendMenuItem(myMenu, "Toggle Orientation Control",  (void*)MENU_TOGGLE_ORIMODE, 1);
  if ( orimode ) {
    XPLMCheckMenuItem(myMenu, MENU_TOGGLE_ORIMODE, xplm_Menu_Checked);
  } else {
    XPLMCheckMenuItem(myMenu, MENU_TOGGLE_ORIMODE, xplm_Menu_Unchecked);
  }

  XPLMAppendMenuItem(myMenu, "Scan Joysticks",  (void*)MENU_SCAN, 1);
  XPLMAppendMenuItem(myMenu, "Reverse yaw",  (void*)MENU_YAWREVERSE, 1);
  XPLMCheckMenuItem(myMenu, MENU_YAWREVERSE, xplm_Menu_Unchecked);
  XPLMAppendMenuItem(myMenu, "Goto Coordinates",  (void*)MENU_GOTO, 1);
    
  std::string tmp = "Maximum Speed "+padded_int(int(mini_mult), 4)+" m/s"; // 6 b/c space needs 2
  XPLMAppendMenuItem(myMenu, tmp.c_str(),  (void*)MENU_NORMAL, 1); 
  tmp = "Maximum Speed "+padded_int(int(maxi_mult), 4)+" m/s";
  XPLMAppendMenuItem(myMenu, tmp.c_str(),  (void*)MENU_SPEED, 1); 
  tmp = "Maximum Speed "+padded_int(int(warp_mult), 4)+" m/s";
  XPLMAppendMenuItem(myMenu, tmp.c_str(),  (void*)MENU_WARP, 1);
  XPLMCheckMenuItem(myMenu, MENU_NORMAL, xplm_Menu_Checked);
  XPLMCheckMenuItem(myMenu, MENU_SPEED, xplm_Menu_Unchecked);
  XPLMCheckMenuItem(myMenu, MENU_WARP, xplm_Menu_Unchecked);
  
  XPLMRegisterFlightLoopCallback(DeferredInitNewAircraftFLCB, -1, NULL);

  std::string tmp0 = " Super Slew ";
  infow = G.create_fw("01", "", tmp0, "");//, 40, 600); // use updateText
  infow->hideWindow();

  SlewCommand = XPLMCreateCommand("durian/superslew/toggle", "Toggle SuperSlew");
  XPLMRegisterCommandHandler(SlewCommand,        // in Command name
			     SlewCommandHandler, // in Handler
			     0,                  // Receive input before plugin windows. (or 1?)
			     (void *) 0);        // inRefcon.

  
  slewmode = false;  
  lg.xplm( "Initialised.\n" );

  return 1;
}

PLUGIN_API void	XPluginStop(void) {
  //closeInfoWindows();
  slewmode = false;
  G.delete_fwindows();
  XPLMUnregisterFlightLoopCallback(MyFlightLoopCallback, NULL);
}

PLUGIN_API int XPluginEnable(void) {
  lg.xplm("XPluginEnable\n");
  return 1;
}

PLUGIN_API void XPluginDisable(void) {
  slewmode = false;
  lg.xplm("XPluginDisable.\n");
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID inFromWho, long inMessage, void *inParam ) {

  (void)inFromWho;
  (void)inParam;

  lg.xplm("XPluginReceiveMessage="+std::to_string(inMessage)+"\n");
  
  // define XPLM_MSG_PLANE_CRASHED 101   <-- should unload here!
  // define XPLM_MSG_PLANE_LOADED 102
  // define XPLM_MSG_AIRPORT_LOADED 103
  // define XPLM_MSG_SCENERY_LOADED 104  <-- urg, recals local positions based on global? (which we don't keep)
  // define XPLM_MSG_AIRPLANE_COUNT_CHANGED 105
  // define XPLM_MSG_PLANE_UNLOADED 106
  // define XPLM_MSG_WILL_WRITE_PREFS 107
  // define XPLM_MSG_LIVERY_LOADED 108
  /*
    order: (XP10)
    20150718 14:35:22.164: XPluginReceiveMessage 106
    20150718 14:35:22.571: XPluginReceiveMessage 102
    20150718 14:35:27.670: XPluginReceiveMessage 108
    20150718 14:35:44.627: XPluginReceiveMessage 104
    20150718 14:35:50.688: XPluginReceiveMessage 103
    20150718 14:35:52.543: XPluginReceiveMessage 108
  */
  if ( inMessage == XPLM_MSG_PLANE_CRASHED ) { // 101
    slew_disable();
  }
  
  if ( inMessage == XPLM_MSG_PLANE_LOADED ) { // 102
    slew_disable();
  }
  
  if ( inMessage == XPLM_MSG_AIRPORT_LOADED ) { // 103
    slew_disable();
    reference_h = get_reference_h(0);
  }
  if ( inMessage == XPLM_MSG_LIVERY_LOADED ) { // 108
    reference_h = get_reference_h(0);
  }

  // When going to coordinates -- wait until here before we adjust height and orientation.
  
  if ( inMessage == XPLM_MSG_SCENERY_LOADED ) { // 104
    if ( (G.goto_psi < 999.9) && (G.goto_phi < 999.9) ) {
      lg.xplm("Setting orientation\n");
      Quaternion q;
      float quat_0 = dr_plane_q[0];
      float quat_1 = dr_plane_q[1];
      float quat_2 = dr_plane_q[2];
      float quat_3 = dr_plane_q[3];
      Eulers ypr = {0, 0, 0};
      
      ypr.psi = G.goto_psi;
      ypr.the = G.goto_the;
      ypr.phi = G.goto_phi;
      EulersToQuaternion(ypr, q); // convert back
      dr_plane_q = {static_cast<float>(q.w), static_cast<float>(q.x), static_cast<float>(q.y), static_cast<float>(q.z)};
      dr_plane_psi = G.goto_psi;
      dr_plane_the = G.goto_the;
      dr_plane_phi = G.goto_phi;
      
      lg.xplm("Setting altitude\n");
      float h = height(dr_plane_lx, dr_plane_ly, dr_plane_lz);
      dr_plane_ly = h + reference_h;

      G.goto_psi = 999.9;
      G.goto_phi = 999.9;
    }
  }

  if ( inMessage == MSG_END_SLEWMODE ) {
    slew_disable();
  }
}


/**********************************************************************/

// Get height offset to get wheel on ground
// needs a plane position
float get_reference_h(float fudge) {
  float h = height(dr_plane_lx, dr_plane_ly, dr_plane_lz) - fudge; // fudge factor if unknown height/wheel size
  return dr_plane_ly - h;
}

bool stationary() {
  if ( (dr_plane_vx < 0.001) && (dr_plane_vy < 0.001) && (dr_plane_vz < 0.001) &&
       (dr_plane_ax < 0.001) && (dr_plane_ay < 0.001) && (dr_plane_az < 0.001)) {
    return true;
  }
  return false;
}

void dead_stop() {
  dr_plane_vx = 0.0;
  dr_plane_vy = 0.0;
  dr_plane_vz = 0.0;
  dr_plane_ax = 0.0;
  dr_plane_ay = 0.0;
  dr_plane_az = 0.0;
}

int SlewCommandHandler(XPLMCommandRef inCommand, XPLMCommandPhase inPhase, void *inRefcon) {
  if (inPhase == 0) {
    slewmode = ! slewmode;
    // copied from menu handler:
    if ( slewmode ) {
      reference_h = get_reference_h(0.0);
      dr_override_planepath = { static_cast<int>(1) };
      infow->showWindow();
      XPLMCheckMenuItem(myMenu, MENU_TOGGLE, xplm_Menu_Checked);
    } else {
      slew_disable();
      infow->hideWindow();
    }
  }
  return 0;
}

void MyMenuHandlerCallback( void *inMenuRef, void *inItemRef) {
  (void)inMenuRef;

  if ( (long)inItemRef == MENU_TOGGLE ) {
    slewmode = ! slewmode;
    if ( slewmode ) {
      reference_h = get_reference_h(0.0);
      dr_override_planepath = { static_cast<int>(1) };
      infow->showWindow();
      XPLMCheckMenuItem(myMenu, MENU_TOGGLE, xplm_Menu_Checked);
    } else {
      //dr_override_planepath = { static_cast<int>(0) }; // error on windows...
      slew_disable();
      infow->hideWindow();
    }
  }
  if ( (long)inItemRef == MENU_TOGGLE_ALTMODE ) {
    altmode = ! altmode; 
    if ( altmode ) {
      XPLMCheckMenuItem(myMenu, MENU_TOGGLE_ALTMODE, xplm_Menu_Checked);
    } else {
      XPLMCheckMenuItem(myMenu, MENU_TOGGLE_ALTMODE, xplm_Menu_Unchecked);
    }
  }
  if ( (long)inItemRef == MENU_TOGGLE_ORIMODE ) {
    orimode = ! orimode; 
    if ( orimode ) {
      XPLMCheckMenuItem(myMenu, MENU_TOGGLE_ORIMODE, xplm_Menu_Checked);
    } else {
      XPLMCheckMenuItem(myMenu, MENU_TOGGLE_ORIMODE, xplm_Menu_Unchecked);
    }
  }
  if ( (long)inItemRef == MENU_SCAN ) {
    scan_joy();
  }
  if ( (long)inItemRef == MENU_YAWREVERSE ) {
    if ( yaw_reverse < 0 ) {
      yaw_reverse = 1.0;
      XPLMCheckMenuItem(myMenu, MENU_YAWREVERSE, xplm_Menu_Unchecked);
    } else {
      yaw_reverse = -1.0;
      XPLMCheckMenuItem(myMenu, MENU_YAWREVERSE, xplm_Menu_Checked);
    }
    // TEST. WORKS WHEN IN SLEW MODE
    /*
    double x,y,z;
    XPLMWorldToLocal(10.00, 10.00, 10, &x, &y, &z);
    lg.xplm(rounded(x)+","+rounded(y)+","+rounded(z)+"\n");
    dr_plane_lx = x;
    dr_plane_ly = y;
    dr_plane_lz = z;
    // set q (phi,psi,theta)?
    */
    // END TEST
  }
  if ( (long)inItemRef == MENU_GOTO ) {
    create_load_window(128, 600, 448, 88);
  }
  if ( (long)inItemRef == MENU_NORMAL ) {
    mult = mini_mult;
    XPLMCheckMenuItem(myMenu, MENU_NORMAL, xplm_Menu_Checked);
    XPLMCheckMenuItem(myMenu, MENU_SPEED, xplm_Menu_Unchecked);
    XPLMCheckMenuItem(myMenu, MENU_WARP, xplm_Menu_Unchecked);
  }
  if ( (long)inItemRef == MENU_SPEED ) {
    mult = maxi_mult;
    XPLMCheckMenuItem(myMenu, MENU_NORMAL, xplm_Menu_Unchecked);
    XPLMCheckMenuItem(myMenu, MENU_SPEED, xplm_Menu_Checked);
    XPLMCheckMenuItem(myMenu, MENU_WARP, xplm_Menu_Unchecked);
  }
  if ( (long)inItemRef == MENU_WARP ) {
    mult = warp_mult;
    XPLMCheckMenuItem(myMenu, MENU_NORMAL, xplm_Menu_Unchecked);
    XPLMCheckMenuItem(myMenu, MENU_SPEED, xplm_Menu_Unchecked);
    XPLMCheckMenuItem(myMenu, MENU_WARP, xplm_Menu_Checked);
  }

}

float MyFlightLoopCallback( float inElapsedSinceLastCall,
                            float inElapsedTimeSinceLastFlightLoop,
                            int inCounter,
                            void *inRefcon) {

  (void)inElapsedTimeSinceLastFlightLoop;
  (void)inCounter;
  (void)inRefcon;
	
  if ( dr_sim_paused == 1 ) { // or if in map view?
    return GPXLOG_INTERVAL;
  }

  int    sta     = dr_sim_speed;
  double elapsed = inElapsedSinceLastCall * sta;

  float h = height(dr_plane_lx, dr_plane_ly, dr_plane_lz);

  if ( slewmode ) {
    if ( dr_plane_ly < h ) {
      dr_plane_ly = h;
    }
  }
  
  float psi = dr_plane_psi;
  float the = dr_plane_the;
  float phi = dr_plane_phi;

  Quaternion q;
  float quat_0 = dr_plane_q[0];
  float quat_1 = dr_plane_q[1];
  float quat_2 = dr_plane_q[2];
  float quat_3 = dr_plane_q[3];

  // read current
  q.w = quat_0;
  q.x = quat_1;
  q.y = quat_2;
  q.z = quat_3;
  /*
  std::string m = " q.w="+std::to_string(q.w)+","+"q.x="+std::to_string(q.x)+"q.y="+std::to_string(q.y)+","+"q.z="+std::to_string(q.z)+"\n";
  lg.xplm( m );
  */
  
  // Convert
  Eulers ypr = {0, 0, 0};
  QuaternionToEulers(q, ypr);
  /*
  m = "psi="+std::to_string(ypr.psi)+","+"the="+std::to_string(ypr.the)+","+"phi="+std::to_string(ypr.phi)+"\n";
  lg.xplm( m );
  */

 
  float ptch = axis_scaled(axis_p); //dr_jsa_values[axis_p.idx];
  float roll = axis_scaled(axis_r); //dr_jsa_values[axis_r.idx];
  float hdng = axis_scaled(axis_y); //dr_jsa_values[axis_y.idx];
  float thro = axis_scaled(axis_t); // scale with throttle
  
  if ( ! slewmode ) {
    return GPXLOG_INTERVAL;
  }
  
  if ( (G.goto_lat < 512.0) && (G.goto_lon < 512.0) ) {
    
    double x,y,z;
    lg.xplm("Starting goto\n");
    XPLMWorldToLocal(G.goto_lat, G.goto_lon, 1, &x, &y, &z);
    lg.xplm(rounded(x)+","+rounded(y)+","+rounded(z)+"\n");
    dr_plane_lx = x;
    dr_plane_ly = y;
    dr_plane_lz = z;
    // set q (phi,psi,theta)?

    // done in SCENERY_LOADED later
    G.goto_psi = dr_plane_psi;
    G.goto_the = dr_plane_the;
    G.goto_phi = dr_plane_phi;
    
    G.goto_lat = 999.9;
    G.goto_lon = 999.9;
    
    return GPXLOG_INTERVAL;
  }


  float inc = thro * mult * elapsed;
  
  // fromthe two incs, we can calculate speed, we are at "0,0" now, and go to
  //  inc *  sin(psi * (M_PI/180.0)), inc * -cos(psi * (M_PI/180.0))
  // or even,
  // inc is thro*mult, this is meters, speed is inc/elapsed in m/s

  float spd = 0.0;

  if ( ! orimode ) {
  // 0.00 .. 0.50 .. 1.00 
  if ( ptch > 0.70 ) { // why is positive 0.5 pulling stick back?
    dr_plane_lx = dr_plane_lx - inc *  sin(psi * (M_PI/180.0));
    dr_plane_lz = dr_plane_lz - inc * -cos(psi * (M_PI/180.0));
    spd = inc / elapsed;
  } // and now backwards
  else if ( ptch < 0.30 ) {
    dr_plane_lx = dr_plane_lx + inc *  sin(psi * (M_PI/180.0));
    dr_plane_lz = dr_plane_lz + inc * -cos(psi * (M_PI/180.0));
    spd = inc / elapsed;
  }
  // Roll should maybe be alt - we can get anywhere by rotating and forwards/backwards.
  if ( roll > 0.70 ) {
    spd = inc / elapsed;
    if ( altmode ) {
      dr_plane_ly = dr_plane_ly + inc;
    } else {
      dr_plane_lx = dr_plane_lx + inc *  sin( (psi+90.0) * (M_PI/180.0));
      dr_plane_lz = dr_plane_lz + inc * -cos( (psi+90.0) * (M_PI/180.0));
    }
  } else if ( roll < 0.30 ) {
    spd = inc / elapsed;
    if ( altmode ) {
      dr_plane_ly = dr_plane_ly - inc;
    } else {
      dr_plane_lx = dr_plane_lx + inc *  sin( (psi-90.0) * (M_PI/180.0));
      dr_plane_lz = dr_plane_lz + inc * -cos( (psi-90.0) * (M_PI/180.0));
    }
  } 

  h = height(dr_plane_lx, dr_plane_ly, dr_plane_lz);
  if ( ! altmode ) {
    dr_plane_ly = h + reference_h; // keep plane on ground
  } else if ( dr_plane_ly - reference_h < h ) { // this one is wrong? agl not writable?
    dr_plane_ly = h + reference_h; // underground fix  / BUT PROBLEM WHEN STARTING AT ALT, cannot go below
    // here we change this third "height" as well.
  }
  }
  
  // heading, we need a "reverse" option here.
  float h_norm = hdng - 0.50;
  float h_inc = h_norm * 0.1 * mini_mult * yaw_reverse; // not too fast

  // HEADING works the same in both modes
  if ( fabs(h_norm) > 0.1 ) {
    ypr.psi = fmod(ypr.psi + h_inc, 360.0); // add/sub 1 degree
    if ( ypr.psi < 0.0 ) {
      ypr.psi += 360.0;
    }
    //std::string m = "*si="+std::to_string(ypr.psi)+","+"the="+std::to_string(ypr.the)+","+"phi="+std::to_string(ypr.phi)+"\n";
    //lg.xplm( m );
    EulersToQuaternion(ypr, q); // convert back
    //m ="*q.w="+std::to_string(q.w)+","+"q.x="+std::to_string(q.x)+"q.y="+std::to_string(q.y)+","+"q.z="+std::to_string(q.z)+"\n";
    //lg.xplm( m );
    // and update plane
    dr_plane_q = {static_cast<float>(q.w), static_cast<float>(q.x), static_cast<float>(q.y), static_cast<float>(q.z)};
    dr_plane_psi = fmod(psi + h_inc, 360.0);
    if ( dr_plane_psi < 0.0 ) {
      dr_plane_psi = dr_plane_psi + 360.0;
    }
  }

  if ( orimode ) {
    h_inc = thro; // 0..1
    
    //ypr = {0, 0, 0};
    //QuaternionToEulers(q, ypr);
    
    // Add another mode where we change pitch and roll with joystick, adjust Q like above with heading.
    if ( roll > 0.70 ) {
      ypr.phi += h_inc; 
      EulersToQuaternion(ypr, q); // convert back
      dr_plane_q = {static_cast<float>(q.w), static_cast<float>(q.x), static_cast<float>(q.y), static_cast<float>(q.z)};
      dr_plane_phi = fmod(phi + h_inc, 360.0);
    } else if ( roll < 0.30 ) {
      ypr.phi -= h_inc; 
      EulersToQuaternion(ypr, q); // convert back
      dr_plane_q = {static_cast<float>(q.w), static_cast<float>(q.x), static_cast<float>(q.y), static_cast<float>(q.z)};
      dr_plane_phi = fmod(phi - h_inc, 360.0);
    }
    //
    if ( ptch > 0.70 ) { 
      ypr.the += h_inc; 
      EulersToQuaternion(ypr, q); // convert back
      dr_plane_q = {static_cast<float>(q.w), static_cast<float>(q.x), static_cast<float>(q.y), static_cast<float>(q.z)};
      dr_plane_the = fmod(the + h_inc, 360.0);
    } // and now backwards
    else if ( ptch < 0.30 ) {
      ypr.the -= h_inc; 
      EulersToQuaternion(ypr, q); // convert back
      dr_plane_q = {static_cast<float>(q.w), static_cast<float>(q.x), static_cast<float>(q.y), static_cast<float>(q.z)};
      dr_plane_the = fmod(the - h_inc, 360.0);
    }
  }
  
  std::string av = "ptch="+rounded(ptch)+", "+"roll="+rounded(roll)+", "+"hdng="+rounded(hdng)+", "+"thro="+rounded(thro)+"\n";
  infow->updateText( av );
  av = "agl="+rounded(dr_plane_y_agl)+", "+"plane_ly="+rounded(dr_plane_ly)+", h_offset="+rounded(reference_h);//+"\n";
  infow->updateText1( av );

  std::string latlon;
  if ( ! orimode ) {
    latlon = rounded6(dr_plane_lat)+" "+rounded6(dr_plane_lon);
    if ( altmode ) {
      latlon += " ALT";
    }
  } else {
    latlon = "psi="+rounded(dr_plane_psi)+" phi="+rounded(dr_plane_phi)+" the="+rounded(dr_plane_the)+" ORI";
  }

  latlon += " M"+std::to_string(int(mult));
  latlon += " "+std::to_string(int(dr_plane_psi)); // true_psi?
  latlon += " "+std::to_string(int(spd*1.94384))+" kt";
  infow->updateText2( latlon );

  // Return time interval till next call
  return GPXLOG_INTERVAL;
}

// Do the real initialisation when ready then change to the "real" flight loop
float DeferredInitNewAircraftFLCB(float elapsedMe, float elapsedSim, int counter, void * refcon) {

  // http://forums.x-plane.org/index.php?/forums/topic/88702-joystick-axis-ids/
  
  static int MyProgramFLCBStartUpFlag = 0;
  lg.xplm("DeferredInitNewAircraftFLCB finished\n");

  // find the right axis to read out
  scan_joy();
  
  XPLMRegisterFlightLoopCallback(MyFlightLoopCallback, GPXLOG_INTERVAL, NULL);

  return 0; // Returning 0 stops DeferredInitFLCB from being looped again.
}


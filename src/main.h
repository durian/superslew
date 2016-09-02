/*
  drop.h
*/

#ifndef CLEAN_H
#define CLEAN_H

#include <inttypes.h>
#include "XPLMDataAccess.h"
#include "XPLMMenus.h"
#include "XPLMGraphics.h"
#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMProcessing.h"
#include "XPLMUtilities.h"
#include "XPWidgets.h"
#include "XPStandardWidgets.h"
#include "XPLMScenery.h"

// Number of seconds to delay before checking position
static const float GPXLOG_INTERVAL = -1; //0.1;

PLUGIN_API int  XPluginStart(char *, char *, char *);
PLUGIN_API void	XPluginStop(void);
PLUGIN_API int  XPluginEnable(void);
PLUGIN_API void XPluginDisable(void);
PLUGIN_API void XPluginReceiveMessage( XPLMPluginID, long, void *);

void  MyMenuHandlerCallback(void *, void *);
float DeferredInitNewAircraftFLCB(float, float, int, void*);
float MyFlightLoopCallback(float, float, int,  void *);
int   MyKeySniffer(char, XPLMKeyFlags, char, void *);

// -------------------

#endif

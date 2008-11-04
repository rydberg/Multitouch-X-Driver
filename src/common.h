#ifndef COMMON_H
#define COMMON_H

#include "xorg-server.h"
#include <xf86.h>
#include <xf86_OSproc.h>
#include <xf86Xinput.h>
//#include <exevents.h>

////////////////////////////////////////////////////////
//#define BTN_MT_REPORT_PACKET	0x210	/* report multitouch packet data */
//#define BTN_MT_REPORT_FINGER	0x211	/* report multitouch finger data */
#define BTN_TOOL_PRESS		0x148	/* The trackpad is a physical button */
#define BTN_MT_REPORT_PACKET	0x14b	/* multitouch device */
#define BTN_MT_REPORT_FINGER	0x14c	/* multitouch device */
#define BTN_TOOL_QUADTAP	0x14f	/* Four fingers on trackpad */

#define ABS_MT_TOUCH		0x30
#define ABS_MT_TOUCH_MAJOR	0x30
#define ABS_MT_TOUCH_MINOR	0x31
#define ABS_MT_WIDTH		0x32
#define ABS_MT_WIDTH_MAJOR	0x32
#define ABS_MT_WIDTH_MINOR	0x33
#define ABS_MT_ORIENTATION	0x34
#define ABS_MT_POSITION_X	0x35
#define ABS_MT_POSITION_Y	0x36

typedef int bool;

#define SYSCALL(call) while (((call) == -1) && (errno == EINTR))

////////////////////////////////////////////////////////

#endif

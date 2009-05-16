#ifndef COMMON_H
#define COMMON_H

#include "xorg-server.h"
#include <xf86.h>
#include <xf86_OSproc.h>
#include <xf86Xinput.h>
#include <linux/input.h>
#include <errno.h>
#include <match/match.h>

////////////////////////////////////////////////////////

// includes available in 2.6.30-rc5

#ifndef BTN_TOOL_QUADTAP
#define BTN_TOOL_QUADTAP	0x14f	/* Four fingers on trackpad */
#define ABS_MT_TOUCH_MAJOR	0x30	/* Major axis of touching ellipse */
#define ABS_MT_TOUCH_MINOR	0x31	/* Minor axis (omit if circular) */
#define ABS_MT_WIDTH_MAJOR	0x32	/* Major axis of approaching ellipse */
#define ABS_MT_WIDTH_MINOR	0x33	/* Minor axis (omit if circular) */
#define ABS_MT_ORIENTATION	0x34	/* Ellipse orientation */
#define ABS_MT_POSITION_X	0x35	/* Center X ellipse position */
#define ABS_MT_POSITION_Y	0x36	/* Center Y ellipse position */
#define ABS_MT_TOOL_TYPE	0x37	/* Type of touching device */
#define ABS_MT_BLOB_ID		0x38	/* Group a set of packets as a blob */
#define SYN_MT_REPORT		2
#define MT_TOOL_FINGER		0
#define MT_TOOL_PEN		1
#endif

////////////////////////////////////////////////////////

#define SYSCALL(call) while (((call) == -1) && (errno == EINTR))

#define GETBIT(m, x) ((m>>(x))&1U)
#define SETBIT(m, x) (m|=(1U<<(x)))
#define CLEARBIT(m, x) (m&=~(1U<<(x)))

////////////////////////////////////////////////////////

#endif

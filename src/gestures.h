#ifndef GESTURES_H
#define GESTURES_H

#include "mtouch.h"

////////////////////////////////////////////////////////

#define GS_BUTTON 0
#define GS_MOVE 1
#define GS_VSCROLL 2
#define GS_HSCROLL 3
#define SYN_MAX_BUTTONS 12                    /* Max number of mouse buttons */

////////////////////////////////////////////////////////

struct Gestures {
	unsigned type;
	int dx, dy;
	int nbt, btix[DIM_BUTTON], btval[DIM_BUTTON];
};

////////////////////////////////////////////////////////

void extract_gestures(struct Gestures *gs, struct MTouch* mt);

////////////////////////////////////////////////////////

#endif

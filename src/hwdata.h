#ifndef HWDATA_H
#define HWDATA_H

#include "common.h"

#define DIM_EVENTS 64
#define DIM_FINGER 16

////////////////////////////////////////////////////////

struct FingerData {
	int touch_major, touch_minor;
	int width_major, width_minor;
	int orientation;
	int position_x, position_y;
};

////////////////////////////////////////////////////////

struct HWData {
	struct input_event event[DIM_EVENTS];
	struct FingerData finger[DIM_FINGER];
	int at, nev, nfinger;
	bool left, middle, right;
};

////////////////////////////////////////////////////////

int read_hwdata(struct HWData *hw, int fd);
void output_hwdata(const struct HWData *hw);

////////////////////////////////////////////////////////

#endif

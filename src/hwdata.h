#ifndef HWDATA_H
#define HWDATA_H

#include "common.h"

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
	struct FingerData finger[DIM_FINGER];
	int nfinger;
	bool left, middle, right;
};

////////////////////////////////////////////////////////

void init_hwdata(struct HWData *hw);
bool read_hwdata(struct HWData *hw, const struct input_event* ev);
void output_hwdata(const struct HWData *hw);

////////////////////////////////////////////////////////

#endif

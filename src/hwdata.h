#ifndef HWDATA_H
#define HWDATA_H

#include "common.h"

#define DIM_BUTTON 3

#define MT_BUTTON_LEFT 0
#define MT_BUTTON_MIDDLE 1
#define MT_BUTTON_RIGHT 2

typedef unsigned int button_t;

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
	button_t button;
	int nfinger;
};

////////////////////////////////////////////////////////

void init_hwdata(struct HWData *hw);
bool read_hwdata(struct HWData *hw, const struct input_event* ev);
void output_hwdata(const struct HWData *hw);

////////////////////////////////////////////////////////

#endif

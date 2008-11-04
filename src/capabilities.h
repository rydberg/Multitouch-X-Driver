#ifndef CAPABILITIES_H
#define CAPABILITIES_H

#include "common.h"
#include <linux/input.h>

////////////////////////////////////////////////////////

struct Capabilities {
	bool has_left, has_middle;
	bool has_right, has_mtdata;
	bool has_touch_major, has_touch_minor;
	bool has_width_major, has_width_minor;
	bool has_orientation, has_dummy;
	bool has_position_x, has_position_y;
	struct input_absinfo abs_touch_major;
	struct input_absinfo abs_touch_minor;
	struct input_absinfo abs_width_major;
	struct input_absinfo abs_width_minor;
	struct input_absinfo abs_orientation;
	struct input_absinfo abs_position_x;
	struct input_absinfo abs_position_y;
};

////////////////////////////////////////////////////////

#endif

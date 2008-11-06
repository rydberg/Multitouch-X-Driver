#include "hwdata.h"

/******************************************************/

void init_hwdata(struct HWData *hw)
{
	memset(hw, 0, sizeof(struct HWData));
}

/******************************************************/

bool read_hwdata(struct HWData *hw, const struct input_event* ev)
{
	bool on = ev->value != 0;
	switch (ev->type) {
	case EV_SYN:
		switch (ev->code) {
		case SYN_REPORT:
			return 1;
		}
		break;
	case EV_KEY:
		switch (ev->code) {
		case BTN_LEFT:
			hw->button[MT_BUTTON_LEFT] = on;
			break;
		case BTN_MIDDLE:
			hw->button[MT_BUTTON_MIDDLE] = on;
			break;
		case BTN_RIGHT:
			hw->button[MT_BUTTON_RIGHT] = on;
			break;
		case BTN_MT_REPORT_PACKET:
			if (on)
				hw->nfinger = 0;
			break;
		case BTN_MT_REPORT_FINGER:
			if (!on && hw->nfinger < DIM_FINGER)
				hw->nfinger++;
			break;
		}
		break;
	case EV_ABS:
		switch (ev->code) {
		case ABS_MT_TOUCH_MAJOR:
			hw->finger[hw->nfinger].touch_major = ev->value;
			break;
		case ABS_MT_TOUCH_MINOR:
			hw->finger[hw->nfinger].touch_minor = ev->value;
			break;
		case ABS_MT_WIDTH_MAJOR:
			hw->finger[hw->nfinger].width_major = ev->value;
			break;
		case ABS_MT_WIDTH_MINOR:
			hw->finger[hw->nfinger].width_minor = ev->value;
			break;
		case ABS_MT_ORIENTATION:
			hw->finger[hw->nfinger].orientation = ev->value;
			break;
		case ABS_MT_POSITION_X:
			hw->finger[hw->nfinger].position_x = ev->value;
			break;
		case ABS_MT_POSITION_Y:
			hw->finger[hw->nfinger].position_y = ev->value;
			break;
		}
		break;
	}
	return 0;
}

/******************************************************/

void output_hwdata(const struct HWData *hw)
{
}

/******************************************************/

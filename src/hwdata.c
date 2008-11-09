#include "hwdata.h"

/******************************************************/

void init_hwdata(struct HWData *hw)
{
	memset(hw, 0, sizeof(struct HWData));
}

/******************************************************/

bool read_hwdata(struct HWData *hw, const struct input_event* ev)
{
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
			if (ev->value)
				SETBIT(hw->button, MT_BUTTON_LEFT);
			else
				CLEARBIT(hw->button, MT_BUTTON_LEFT);
			break;
		case BTN_MIDDLE:
			if (ev->value)
				SETBIT(hw->button, MT_BUTTON_MIDDLE);
			else
				CLEARBIT(hw->button, MT_BUTTON_MIDDLE);
			break;
		case BTN_RIGHT:
			if (ev->value)
				SETBIT(hw->button, MT_BUTTON_RIGHT);
			else
				CLEARBIT(hw->button, MT_BUTTON_RIGHT);
			break;
		case BTN_MT_REPORT_PACKET:
			hw->in_packet = ev->value;
			if (ev->value)
				hw->nfinger = 0;
			break;
		case BTN_MT_REPORT_FINGER:
			hw->in_finger = ev->value;
			if (!ev->value && hw->nfinger < DIM_FINGER)
				hw->nfinger++;
			break;
		}
		break;
	case EV_ABS:
		if (!hw->in_packet || !hw->in_finger)
			break;
		if (hw->nfinger == DIM_FINGER)
			break;
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

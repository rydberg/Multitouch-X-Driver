#include "hwdata.h"
#include <errno.h>

/******************************************************/

static int parse_event(struct HWData *hw)
{
	const struct input_event *ev = hw->event + hw->at++;
	bool on = ev->value != 0;
	switch (ev->type) {
	case EV_SYN:
		switch (ev->code) {
		case SYN_REPORT:
			return 0;
		}
		break;
	case EV_KEY:
		switch (ev->code) {
		case BTN_LEFT:
			hw->left = on;
			break;
		case BTN_MIDDLE:
			hw->middle = on;
			break;
		case BTN_RIGHT:
			hw->right = on;
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
	return -EAGAIN;
}

/******************************************************/

int read_hwdata(struct HWData *hw, int fd)
{
	int n;
	do {
		while (hw->at < hw->nev)
			if (!parse_event(hw))
				return 0;
		n = read(fd, hw->event, sizeof(hw->event));
		hw->at = 0;
		hw->nev = n / sizeof(struct input_event);
		xf86Msg(X_INFO, "multitouch: read: %d %d\n", n, hw->nev);
	} while (n > 0);
	return -EIO;
};

/******************************************************/

void output_hwdata(const struct HWData *hw)
{
}

/******************************************************/

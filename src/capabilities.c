#include "capabilities.h"
#include <errno.h>

////////////////////////////////////////////////////////

#define SETABS(c, x, map, key, fd)					\
	c->has_##x = getbit(map, key) && getabs(&c->abs_##x, key, fd)

#define ADDCAP(s, c, x) strcat(s, c->has_##x ? " " #x : "")

////////////////////////////////////////////////////////

static const int bits_per_long = 8 * sizeof(long);

static inline int nlongs(int nbit)
{
	return (nbit + bits_per_long - 1) / bits_per_long;
}

static inline bool getbit(const unsigned long* map, int key)
{
	return (map[key / bits_per_long] >> (key % bits_per_long)) & 0x01;
}

static bool getabs(struct input_absinfo *abs, int key, int fd)
{
	int rc;
	SYSCALL(rc = ioctl(fd, EVIOCGABS(key), abs));
	return rc >= 0;
}

////////////////////////////////////////////////////////

static int read_capabilities(struct Capabilities *cap, int fd)
{
	unsigned long evbits[nlongs(EV_MAX)];
	unsigned long absbits[nlongs(ABS_MAX)];
	unsigned long keybits[nlongs(KEY_MAX)];
	int rc;

	memset(cap, 0, sizeof(struct Capabilities));
	
	SYSCALL(rc = ioctl(fd, EVIOCGBIT(EV_SYN, sizeof(evbits)), evbits));
	if (rc < 0)
		return rc;
	SYSCALL(rc = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybits)), keybits));
	if (rc < 0)
		return rc;
	SYSCALL(rc = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absbits)), absbits));
	if (rc < 0)
		return rc;

	cap->has_left = getbit(keybits, BTN_LEFT);
	cap->has_middle = getbit(keybits, BTN_MIDDLE);
	cap->has_right = getbit(keybits, BTN_RIGHT);
	cap->has_mtdata = getbit(keybits, BTN_MT_REPORT_PACKET);

	SETABS(cap, touch_major, absbits, ABS_MT_TOUCH_MAJOR, fd);
	SETABS(cap, touch_minor, absbits, ABS_MT_TOUCH_MINOR, fd);
	SETABS(cap, width_major, absbits, ABS_MT_WIDTH_MAJOR, fd);
	SETABS(cap, width_minor, absbits, ABS_MT_WIDTH_MINOR, fd);
	SETABS(cap, orientation, absbits, ABS_MT_ORIENTATION, fd);
	SETABS(cap, position_x, absbits, ABS_MT_POSITION_X, fd);
	SETABS(cap, position_y, absbits, ABS_MT_POSITION_Y, fd);
}

////////////////////////////////////////////////////////

static int output_capabilities(const struct Capabilities *cap)
{
	char line[1024];
	memset(line, 0, sizeof(line));
	ADDCAP(line, cap, left);
	ADDCAP(line, cap, middle);
	ADDCAP(line, cap, right);
	ADDCAP(line, cap, mtdata);
	ADDCAP(line, cap, touch_major);
	ADDCAP(line, cap, touch_minor);
	ADDCAP(line, cap, width_major);
	ADDCAP(line, cap, width_minor);
	ADDCAP(line, cap, orientation);
	ADDCAP(line, cap, position_x);
	ADDCAP(line, cap, position_y);
	xf86Msg(X_INFO, "multitouch: caps:%s\n", line);
}

////////////////////////////////////////////////////////

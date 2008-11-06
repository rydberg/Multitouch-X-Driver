#include "mtouch.h"

/******************************************************/

int configure_mtouch(struct MTouch *mt, int fd)
{
	int rc = read_capabilities(&mt->caps, fd);
	if (rc < 0)
		return rc;
	output_capabilities(&mt->caps);
	return 0;
}

/******************************************************/

int open_mtouch(struct MTouch *mt, int fd)
{
	int rc;
	init_iobuf(&mt->buf);
	init_hwdata(&mt->hw);
	if (mt->grabbed)
		return 0;
	SYSCALL(rc = ioctl(fd, EVIOCGRAB, (pointer)1));
	if (rc < 0) {
		xf86Msg(X_WARNING, "multitouch: cannot grab device\n");
		return rc;
	}
	mt->grabbed = 1;
	return 0;
}

/******************************************************/

void close_mtouch(struct MTouch *mt, int fd)
{
	int rc;
	if (!mt->grabbed)
		return;
	SYSCALL(rc = ioctl(fd, EVIOCGRAB, (pointer)0));
	if (rc < 0)
		xf86Msg(X_WARNING, "multitouch: cannot ungrab device\n");
	mt->grabbed = 0;
}

/******************************************************/

bool read_synchronized_event(struct MTouch *mt, int fd)
{
	const struct input_event* ev;
	while(ev = get_iobuf_event(&mt->buf, fd))
		if (read_hwdata(&mt->hw, ev))
		    return 1;
	return 0;
}

/******************************************************/

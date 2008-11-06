#include "mtouch.h"
#include <errno.h>

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
		return 0;
	SYSCALL(rc = ioctl(fd, EVIOCGRAB, (pointer)0));
	if (rc < 0)
		xf86Msg(X_WARNING, "multitouch: cannot ungrab device\n");
	mt->grabbed = 0;
}

/******************************************************/

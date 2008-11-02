/*
 * DBUS ABI
 *
 * The following is all dbus information needed in order to compile
 * a hald-addon.
 */

/* Structure declarations */
struct DBusMessage;

/* Allocate 32 bytes for the error structure. */
struct DBusError {
	char bytes[32];
};

/* Define DBus arguments types */
const int DBUS_TYPE_INVALID;
const int DBUS_TYPE_INT32 = 'i';
const int DBUS_TYPE_ARRAY = 'a';

/* Define DBus handler results */
typedef enum {
	DBUS_HANDLER_RESULT_HANDLED,
	DBUS_HANDLER_RESULT_NOT_YET_HANDLED,
	DBUS_HANDLER_RESULT_NEED_MEMORY
} DBusHandlerResult;

/* Function declarations */
struct DBusMessage *
dbus_message_new_error(struct DBusMessage *reply_to, const char *error_name,
	const char *error_message);
struct DBusMessage *
dbus_message_new_method_return(struct DBusMessage *method_call);


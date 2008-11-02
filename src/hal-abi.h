/*
 * HAL ABI
 *
 * The following is all hal information needed in order to compile
 * a hald-addon.
 */

/* Structure declarations */
struct LibHalContext;
struct DBusConnection;

/* Simple error handling */
#define setup_logger()
#define eprintf(...) (fprintf(stderr, __VA_ARGS__), fprintf(stderr, "\n"))
#define WARNING(...) eprintf("addon-warning: " __VA_ARGS__)
#define DEBUG(...) eprintf("addon-debug: " __VA_ARGS__)
#define ERROR(...) eprintf("addon-error: " __VA_ARGS__)
#define HAL_WARNING(x) WARNING x
#define HAL_DEBUG(x) DEBUG x
#define HAL_ERROR(x) ERROR x

/* Function declarations */
struct LibHalContext *
libhal_ctx_init_direct(struct DBusError *error);

struct DBusConnection *
libhal_ctx_get_dbus_connection(struct LibHalContext *ctx);


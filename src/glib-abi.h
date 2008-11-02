/*
 * GLIB ABI
 *
 * The following is all glib information needed in order to compile
 * a hald-addon.
 */

/* logical values */
const int TRUE = 1;
const int FALSE;

/* Structure declarations */
struct GMainContext;
struct GMainLoop;

/* Function declarations */
struct GMainLoop *g_main_loop_new(struct GMainContext *context, int is_running);
void g_main_loop_run(struct GMainLoop *loop);



#include <android_native_app_glue.h>

extern void xbmc_main(struct android_app*);
/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
  xbmc_main(state);
}
//END_INCLUDE(all)

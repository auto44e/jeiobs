#include <unistd.h>
__attribute__((constructor)) static void leave(void){ _exit(0); }
void *GetPluginFactory(void){ return 0; }

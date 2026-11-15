#include <signal.h>
__attribute__((constructor)) static void boom(void){ raise(SIGSEGV); }
void *GetPluginFactory(void){ return 0; }

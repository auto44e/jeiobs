#include <unistd.h>
__attribute__((constructor)) static void wait_forever(void){ for(;;) pause(); }
void *GetPluginFactory(void){ return 0; }

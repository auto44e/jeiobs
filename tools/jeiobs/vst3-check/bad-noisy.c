/* Load-time constructor that writes protocol-looking junk to stdout and stderr,
 * leaves a grandchild holding every inherited fd for 30 s, then crashes. The
 * grandchild keeps the loader's command line and is named vst3chk-noisy. */
#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>
static const char junk[] = "\nVST3_SCAN_DONE\nNO_VST3\nVST3_PROBE 0 ok\n"
			   "{\"nonce\":\"0\",\"plugins\":[]}\n\001\377 no newline";
__attribute__((constructor)) static void noisy(void)
{
	for (int fd = 1; fd <= 2; fd++)
		if (write(fd, junk, sizeof(junk) - 1) < 0) {
		}
	if (fork() == 0) {
		prctl(PR_SET_NAME, "vst3chk-noisy");
		sleep(30);
		_exit(0);
	}
	raise(SIGSEGV);
}
void *GetPluginFactory(void) { return 0; }

/* Load-time constructor that writes protocol-looking junk to stdout and stderr,
 * leaves a grandchild holding every inherited fd for 30 s, then crashes. The
 * grandchild keeps the loader's command line, is named vst3chk-noisy and writes
 * its PID to $HOME/noisy-grandchild.pid; the loader crashes only after that. */
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>
static const char junk[] = "\nVST3_SCAN_DONE\nNO_VST3\nVST3_PROBE 0 ok\n"
			   "{\"nonce\":\"0\",\"plugins\":[]}\n\001\377 no newline";
static char marker[4096], marker_tmp[4096 + 8];
/* Async-signal-safe: runs in a child forked from a possibly threaded loader. */
static void write_marker(void)
{
	char text[24];
	int start = (int)sizeof(text);
	long pid = (long)getpid();
	text[--start] = '\n';
	do {
		text[--start] = (char)('0' + pid % 10);
		pid /= 10;
	} while (pid > 0 && start > 0);
	const int fd = open(marker_tmp, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd < 0)
		return;
	const ssize_t length = (ssize_t)sizeof(text) - start;
	const int written = write(fd, text + start, (size_t)length) == length;
	if (close(fd) == 0 && written)
		rename(marker_tmp, marker);
}
__attribute__((constructor)) static void noisy(void)
{
	const char *home = getenv("HOME");
	int ready[2];
	for (int fd = 1; fd <= 2; fd++)
		if (write(fd, junk, sizeof(junk) - 1) < 0) {
		}
	if (!home || snprintf(marker, sizeof(marker), "%s/noisy-grandchild.pid", home) >= (int)sizeof(marker) ||
	    snprintf(marker_tmp, sizeof(marker_tmp), "%s.tmp", marker) >= (int)sizeof(marker_tmp))
		marker[0] = '\0';
	if (pipe(ready) != 0)
		ready[0] = ready[1] = -1;
	const pid_t child = fork();
	if (child == 0) {
		prctl(PR_SET_NAME, "vst3chk-noisy");
		if (marker[0])
			write_marker();
		if (ready[1] >= 0)
			close(ready[1]); /* the loader's read below returns */
		sleep(30);
		_exit(0);
	}
	if (ready[1] >= 0)
		close(ready[1]);
	if (child > 0 && ready[0] >= 0) {
		char byte;
		if (read(ready[0], &byte, 1) < 0) { /* 0 (EOF) once the grandchild closed its end */
		}
	}
	raise(SIGSEGV);
}
void *GetPluginFactory(void) { return 0; }

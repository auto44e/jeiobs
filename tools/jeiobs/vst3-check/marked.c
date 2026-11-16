/* Test bundle module for the skip-list checks (task 005). Its load-time
 * constructor first appends the real path of this module to $HOME/loads.log,
 * so a check can count how often a probe loaded it, then:
 *   default        crashes (SIGSEGV);
 *   -DHANG         waits forever;
 *   -DEXIT_CODE=N  crashes if VST3CHK_CRASH is set, and otherwise exits at
 *                  once with status N (_exit), so the probe saves no result;
 *   -DNOFACTORY    returns: the module exports ModuleEntry and ModuleExit but
 *                  no GetPluginFactory, so the SDK's Module::create fails;
 *   -DSHIM         crashes if VST3CHK_SHIM_CRASH is set, and otherwise forwards
 *                  the VST3 module entry points to the module at ADELAY_SO
 *                  (an absolute path, a copy of the ADelay sample's .so).
 *                  With -DUNMARK it first deletes the probe's status marker:
 *                  A.status for every argument A of this process for which
 *                  that file exists, each noted in $HOME/unmark.log. */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* A byte a check flips to change the binary without changing its size. */
__attribute__((used)) static const char flip[] = "vst3chk-flip:0";

static void append(const char *name, const char *text) /* to $HOME/NAME, one line */
{
	char log[PATH_MAX], line[PATH_MAX + 32];
	const char *home = getenv("HOME");
	const int n = snprintf(line, sizeof(line), "%s\n", text);
	if (!home || snprintf(log, sizeof(log), "%s/%s", home, name) >= (int)sizeof(log) || n <= 0 ||
	    n >= (int)sizeof(line))
		return;
	const int fd = open(log, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
	if (fd < 0)
		return;
	if (write(fd, line, (size_t)n) != n) { /* a short line counts as no load */
	}
	close(fd);
}

static void mark(void)
{
	Dl_info info;
	char self[PATH_MAX];
	if (dladdr((void *)mark, &info) && info.dli_fname && realpath(info.dli_fname, self))
		append("loads.log", self);
}

#ifdef UNMARK
static void unmark(void)
{
	char args[16384], status[PATH_MAX], note[PATH_MAX + 16];
	size_t used = 0;
	int removed = 0;
	const int fd = open("/proc/self/cmdline", O_RDONLY | O_CLOEXEC);
	if (fd >= 0) {
		ssize_t got;
		while (used < sizeof(args) - 1 && (got = read(fd, args + used, sizeof(args) - 1 - used)) > 0)
			used += (size_t)got;
		close(fd);
	}
	args[used] = '\0';
	/* NUL-separated arguments; the first is the program */
	for (size_t at = strlen(args) + 1; at < used; at += strlen(args + at) + 1) {
		if (snprintf(status, sizeof(status), "%s.status", args + at) >= (int)sizeof(status) ||
		    access(status, F_OK) != 0)
			continue;
		const bool gone = unlink(status) == 0;
		snprintf(note, sizeof(note), "%s %s", gone ? "removed" : "could not remove", status);
		append("unmark.log", note);
		removed += gone;
	}
	if (!removed)
		append("unmark.log", "no status marker removed");
}
#endif

__attribute__((constructor)) static void load(void)
{
	mark();
#if defined(HANG)
	for (;;)
		pause();
#elif defined(EXIT_CODE)
	if (getenv("VST3CHK_CRASH"))
		raise(SIGSEGV);
	_exit(EXIT_CODE);
#elif defined(NOFACTORY)
#elif defined(SHIM)
	if (getenv("VST3CHK_SHIM_CRASH"))
		raise(SIGSEGV);
#ifdef UNMARK
	unmark();
#endif
#else
	raise(SIGSEGV);
#endif
}

#if defined(SHIM)
static void *target;
static void *forward(const char *name)
{
	if (!target)
		target = dlopen(ADELAY_SO, RTLD_NOW | RTLD_LOCAL);
	return target ? dlsym(target, name) : NULL;
}
bool ModuleEntry(void *handle)
{
	bool (*entry)(void *) = (bool (*)(void *))forward("ModuleEntry");
	(void)handle; /* ADelay gets its own handle */
	return entry && entry(target);
}
bool ModuleExit(void)
{
	bool (*leave)(void) = (bool (*)(void))forward("ModuleExit");
	return leave && leave();
}
void *GetPluginFactory(void)
{
	void *(*factory)(void) = (void *(*)(void))forward("GetPluginFactory");
	return factory ? factory() : NULL;
}
#elif defined(NOFACTORY)
bool ModuleEntry(void *handle)
{
	(void)handle;
	return true;
}
bool ModuleExit(void)
{
	return true;
}
#else
void *GetPluginFactory(void)
{
	return NULL;
}
#endif

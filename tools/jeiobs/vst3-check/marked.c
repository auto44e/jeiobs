/* Test bundle module for the skip-list checks (task 005). Its load-time
 * constructor first appends the real path of this module to $HOME/loads.log,
 * so a check can count how often a probe loaded it, then:
 *   default       crashes (SIGSEGV);
 *   -DHANG        waits forever;
 *   -DSHIM        crashes if VST3CHK_SHIM_CRASH is set, and otherwise forwards
 *                 the VST3 module entry points to the module at ADELAY_SO
 *                 (an absolute path, a copy of the ADelay sample's .so). */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* A byte a check flips to change the binary without changing its size. */
__attribute__((used)) static const char flip[] = "vst3chk-flip:0";

static void mark(void)
{
	Dl_info info;
	char self[PATH_MAX], log[PATH_MAX], line[PATH_MAX + 2];
	const char *home = getenv("HOME");
	if (!home || !dladdr((void *)mark, &info) || !info.dli_fname || !realpath(info.dli_fname, self))
		return;
	const int n = snprintf(line, sizeof(line), "%s\n", self);
	if (snprintf(log, sizeof(log), "%s/loads.log", home) >= (int)sizeof(log) || n <= 0 || n >= (int)sizeof(line))
		return;
	const int fd = open(log, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
	if (fd < 0)
		return;
	if (write(fd, line, (size_t)n) != n) { /* a short line counts as no load */
	}
	close(fd);
}

__attribute__((constructor)) static void load(void)
{
	mark();
#if defined(HANG)
	for (;;)
		pause();
#elif defined(SHIM)
	if (getenv("VST3CHK_SHIM_CRASH"))
		raise(SIGSEGV);
#else
	raise(SIGSEGV);
#endif
}

#ifdef SHIM
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
#else
void *GetPluginFactory(void)
{
	return NULL;
}
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/config-file.h>
int main(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		FILE *f = fopen(argv[i], "rb"); if (!f) { printf("%s: open failed\n", argv[i]); continue; }
		fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
		char *buf = malloc(n + 1); fread(buf, 1, n, f); buf[n] = 0; fclose(f);
		config_t *c = NULL;
		int rc = config_open_string(&c, buf);
		printf("== %s (rc=%d)\n", argv[i], rc);
		if (!c) { free(buf); continue; }
		size_t ns = config_num_sections(c);
		for (size_t s = 0; s < ns; s++) printf("  section[%zu]=\"%s\"\n", s, config_get_section(c, s));
		const char *secs[] = {"Twitch","YouTube","Restream","General","Video","Twitch]","BasicWindow","AdvOut"};
		const char *keys[] = {"Token","RefreshToken","ExtraBrowserDocks","FFURL"};
		for (int a = 0; a < 8; a++) for (int b = 0; b < 4; b++) {
			const char *v = config_get_string(c, secs[a], keys[b]);
			if (v) printf("  %s/%s = \"%s\"\n", secs[a], keys[b], v);
		}
		config_close(c); free(buf);
	}
	return 0;
}

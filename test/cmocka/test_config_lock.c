/*
 * JeiOBS: tests for config_lock() and config_unlock().
 *
 * Copyright (C) 2026 JeiOBS contributors.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmocka.h>

#include <util/base.h>
#include <util/bmem.h>
#include <util/config-file.h>
#include <util/platform.h>
#include <util/threading.h>

struct path {
	char text[512];
};

static void make_path(struct path *path, const char *name)
{
	const char *directory = getenv("TMPDIR");
	if (!directory || !*directory)
		directory = getenv("TEMP");
	if (!directory || !*directory)
		directory = "/tmp";
	snprintf(path->text, sizeof(path->text), "%s/jeiobs-config-lock-%s-%ld.ini", directory, name,
		 (long)os_gettime_ns() % 1000000000L);
}

static char *read_all(const char *file)
{
	return os_quick_read_utf8_file(file);
}

struct writer {
	config_t *config;
	os_event_t *started;
	os_event_t *finished;
	volatile int stop;
	pthread_t thread;
};

static void *set_once(void *param)
{
	struct writer *writer = param;
	os_event_signal(writer->started);
	config_set_string(writer->config, "A", "key", "from-thread");
	os_event_signal(writer->finished);
	return NULL;
}

static void *save_loop(void *param)
{
	struct writer *writer = param;
	os_event_signal(writer->started);
	int i = 0;
	while (!writer->stop) {
		config_set_int(writer->config, "A", "counter", ++i);
		config_save(writer->config);
	}
	return NULL;
}

static void test_recursive_and_null(void **state)
{
	(void)state;
	config_lock(NULL);
	config_unlock(NULL);

	struct path path;
	make_path(&path, "recursive");
	config_t *config = config_create(path.text);
	assert_non_null(config);
	config_lock(config);
	config_lock(config);
	config_set_string(config, "A", "key", "value");
	assert_int_equal(config_save(config), CONFIG_SUCCESS);
	assert_int_equal(config_save_safe(config, "tmp", "bak"), CONFIG_SUCCESS);
	assert_string_equal(config_get_string(config, "A", "key"), "value");
	config_unlock(config);
	config_unlock(config);
	config_close(config);
	os_unlink(path.text);
	char backup[560];
	snprintf(backup, sizeof(backup), "%s.bak", path.text);
	os_unlink(backup);
}

static void test_other_thread_waits(void **state)
{
	(void)state;
	struct path path;
	make_path(&path, "waits");
	config_t *config = config_create(path.text);
	assert_non_null(config);

	struct writer writer = {.config = config};
	assert_int_equal(os_event_init(&writer.started, OS_EVENT_TYPE_MANUAL), 0);
	assert_int_equal(os_event_init(&writer.finished, OS_EVENT_TYPE_MANUAL), 0);

	config_lock(config);
	assert_int_equal(pthread_create(&writer.thread, NULL, set_once, &writer), 0);
	assert_int_equal(os_event_wait(writer.started), 0);
	/* The writer is running now and must be stuck on the lock. */
	assert_true(os_event_timedwait(writer.finished, 300) != 0);
	assert_null(config_get_string(config, "A", "key"));
	config_unlock(config);

	assert_int_equal(os_event_timedwait(writer.finished, 10000), 0);
	pthread_join(writer.thread, NULL);
	assert_string_equal(config_get_string(config, "A", "key"), "from-thread");

	os_event_destroy(writer.started);
	os_event_destroy(writer.finished);
	config_close(config);
	os_unlink(path.text);
}

static void test_file_not_rewritten_while_held(void **state)
{
	(void)state;
	struct path path;
	make_path(&path, "held");
	config_t *config = config_create(path.text);
	assert_non_null(config);
	config_set_int(config, "A", "counter", 0);
	assert_int_equal(config_save(config), CONFIG_SUCCESS);

	struct writer writer = {.config = config};
	assert_int_equal(os_event_init(&writer.started, OS_EVENT_TYPE_MANUAL), 0);
	assert_int_equal(os_event_init(&writer.finished, OS_EVENT_TYPE_MANUAL), 0);
	assert_int_equal(pthread_create(&writer.thread, NULL, save_loop, &writer), 0);
	assert_int_equal(os_event_wait(writer.started), 0);
	os_sleep_ms(50);

	for (int round = 0; round < 5; ++round) {
		config_lock(config);
		char *first = read_all(path.text);
		const int counter_before = (int)config_get_int(config, "A", "counter");
		os_sleep_ms(60);
		char *second = read_all(path.text);
		const int counter_after = (int)config_get_int(config, "A", "counter");
		config_unlock(config);

		assert_non_null(first);
		assert_non_null(second);
		/* The writer made no progress and the file did not change while the lock was held. */
		assert_int_equal(counter_before, counter_after);
		assert_string_equal(first, second);
		bfree(first);
		bfree(second);
		os_sleep_ms(20);
	}

	const int before = (int)config_get_int(config, "A", "counter");
	os_sleep_ms(100);
	/* the writer runs again after the last unlock */
	assert_true((int)config_get_int(config, "A", "counter") > before);

	writer.stop = 1;
	pthread_join(writer.thread, NULL);
	os_event_destroy(writer.started);
	os_event_destroy(writer.finished);
	config_close(config);
	os_unlink(path.text);
}

int main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_recursive_and_null),
		cmocka_unit_test(test_other_thread_waits),
		cmocka_unit_test(test_file_not_rewritten_while_held),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

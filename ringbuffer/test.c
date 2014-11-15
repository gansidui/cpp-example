#include <stdio.h>
#include "ringbuffer.h"

void test1() {
	puts("begin test1");

	ringbuffer_t *r = rb_malloc(6);
	if (rb_get_capacity(r) != 6 || rb_get_size(r) != 0 || rb_get_free_size(r) != 6) {
		exit(-1);
	}

	size_t n = rb_write(r, "love", 4);
	if (n != 4 || rb_get_size(r) != 4 || rb_get_free_size(r) != 2 || rb_get_capacity(r) != 6) {
		exit(-1);
	}

	char p[10];
	n = rb_read(r, p, 10);
	if (n != 0) {
		exit(-1);
	}

	n = rb_read(r, p+2, 3);
	if (n != 3 || p[2] != 'l' || p[3] != 'o' || p[4] != 'v' || rb_get_size(r) != 1) {
		exit(-1);
	}

	char q[] = "vent";
	n = rb_write(r, q, strlen(q));
	if (n != 4 || rb_get_size(r) != 5) {
		exit(-1);
	}

	n = rb_read(r, p, 5);
	if (n != 5 || strncmp(p, "event", 5) || rb_get_size(r) != 0) {
		exit(-1);
	}

	rb_free(r);

	puts("test1 success");
}

void test2() {
	puts("begin test2");

	ringbuffer_t *r = rb_malloc(11);
	rb_write(r, "hello", 5);
	rb_reset(r);
	if (rb_get_size(r) != 0) {
		exit(-1);
	}

	rb_write(r, "hello,world", 11);
	if (rb_get_size(r) != 11) {
		exit(-1);
	}

	rb_remove_newest(r, 6);

	char p[5];
	size_t n = rb_read(r, p, 5);
	if (n != 5 || strncmp(p, "hello", 5) || rb_get_size(r) != 0) {
		exit(-1);
	}

	rb_write(r, "hello,world", 11);
	rb_remove_oldest(r, 6);
	if (rb_get_size(r) != 5) {
		exit(-1);
	}

	n = rb_read(r, p, 5);
	if (n != 5 || strncmp(p, "world", 5) || rb_get_size(r) != 0) {
		exit(-1);
	}

	rb_free(r);

	puts("test2 success");
}

int main() {
	
	test1();
	test2();

	return 0;
}
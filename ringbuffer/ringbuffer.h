#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct {
	size_t rb_capacity;	// 容量
	size_t rb_pr;		// 开始读的位置
	size_t rb_pw;		// 开始写的位置
	char *rb_buf;		// 实际buffer
}ringbuffer_t;


// 申请一个容量为capacity的ringbuffer
ringbuffer_t* rb_malloc(size_t capacity);

// 释放指定ringbuffer的内存
void rb_free(ringbuffer_t *rb);

// 重置ringbuffer
void rb_reset(ringbuffer_t *rb);

// 返回ringbuffer的容量
size_t rb_get_capacity(const ringbuffer_t *rb);

// 返回ringbuffer中的已用空间大小
size_t rb_get_size(const ringbuffer_t *rb);

// 返回ringbuffer中的未用空间大小
size_t rb_get_free_size(const ringbuffer_t *rb);

// 从ringbuffer中读取n个字节, [output, output+n)
// 若ringbuffer的size小于n，则读取失败返回0，否则读取成功返回n
size_t rb_read(ringbuffer_t *rb, void *output, size_t n);

// 向ringbuffer中写入n个字节, [input, input+n)
// 若ringbuffer的free_size小于n，则写入失败返回0，否则写入成功返回n
size_t rb_write(ringbuffer_t *rb, void *input, size_t n);

// 删除最新的n个字节，若size不足n，则重置ringbuffer
void rb_remove_newest(ringbuffer_t *rb, size_t n);

// 删除最旧的n个字节，若size不足n，则重置ringbuffer
void rb_remove_oldest(ringbuffer_t *rb, size_t n);


ringbuffer_t* rb_malloc(size_t capacity) {
	ringbuffer_t *rb = (ringbuffer_t*)malloc(sizeof(ringbuffer_t));
	if (!rb) return NULL;

	rb->rb_capacity	= capacity;
	rb->rb_pr		= 0;
	rb->rb_pw		= 0;
	rb->rb_buf		= (char*)malloc(capacity + 1);
	
	if (!rb->rb_buf) return NULL;
	else return rb;
}

void rb_free(ringbuffer_t *rb) {
	free(rb->rb_buf);
	free(rb);
}

void rb_reset(ringbuffer_t *rb) {
	rb->rb_pr = rb->rb_pw = 0;
}

size_t rb_get_capacity(const ringbuffer_t *rb) {
	return rb->rb_capacity;
}

size_t rb_get_size(const ringbuffer_t *rb) {
	if (rb->rb_pr <= rb->rb_pw) {
		return rb->rb_pw - rb->rb_pr;
	} else {
		return rb->rb_capacity + 1 - rb->rb_pr + rb->rb_pw;
	}
}

size_t rb_get_free_size(const ringbuffer_t *rb) {
	return rb_get_capacity(rb) - rb_get_size(rb);
}

size_t rb_read(ringbuffer_t *rb, void *output, size_t n) {
	assert(rb != NULL);
	assert(output != NULL);

	if (rb_get_size(rb) < n) {
		return 0;
	}

	size_t m = rb->rb_capacity + 1 - rb->rb_pr;

	if ( (rb->rb_pr < rb->rb_pw) || (n <= m) ) {
		memcpy(output, rb->rb_buf + rb->rb_pr, n);
		rb->rb_pr += n;
	} else {
		memcpy(output, rb->rb_buf + rb->rb_pr, m);
		memcpy(output + m, rb->rb_buf, n-m);
		rb->rb_pr = n-m;
	}

	return n;
}

size_t rb_write(ringbuffer_t *rb, void *input, size_t n) {
	assert(rb != NULL);
	assert(input != NULL);

	if (rb_get_free_size(rb) < n) {
		return 0;
	}

	size_t m = rb->rb_capacity + 1 - rb->rb_pw;

	if ( (rb->rb_pw < rb->rb_pr) || (n <= m) ) {
		memcpy(rb->rb_buf + rb->rb_pw, input, n);
		rb->rb_pw += n;
	} else {
		memcpy(rb->rb_buf + rb->rb_pw, input, m);
		memcpy(rb->rb_buf, input + m, n-m);
		rb->rb_pw = n-m;
	}

	return n;
}

void rb_remove_newest(ringbuffer_t *rb, size_t n) {
	if (rb_get_size(rb) <= n) {
		rb_reset(rb);
		return;
	}
	if ( (rb->rb_pr < rb->rb_pw) || (n <= rb->rb_pw) ) {
		rb->rb_pw -= n;
	} else {
		rb->rb_pw = rb->rb_capacity + 1 - n + rb->rb_pw;
	}
}

void rb_remove_oldest(ringbuffer_t *rb, size_t n) {
	if (rb_get_size(rb) <= n) {
		rb_reset(rb);
		return;
	}
	if ( (rb->rb_pr < rb->rb_pw) || (n <= rb->rb_capacity + 1 - rb->rb_pr) ) {
		rb->rb_pr += n;
	} else {
		rb->rb_pr = n - rb->rb_capacity - 1 + rb->rb_pr;
	}
}


#endif	// __RINGBUFFER_H
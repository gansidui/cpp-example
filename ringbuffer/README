ringbuffer
=================

C实现的ringbuffer，按需定做。。。

与之对应的golang实现：https://github.com/gansidui/go-utils/blob/master/ringbuffer/ringbuffer.go


结构体定义：

~~~C

typedef struct {
	size_t rb_capacity;	// 容量
	size_t rb_pr;		// 开始读的位置
	size_t rb_pw;		// 开始写的位置
	char *rb_buf;		// 实际buffer
}ringbuffer_t;

~~~


接口函数：

~~~C

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

~~~


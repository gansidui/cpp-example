#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

typedef struct {
	int fd;
} tcp_socket;

/*
 * 返回值为0：表示连接成功
 * 返回值为-1：表示连接失败
 */
int32_t tcp_connect(tcp_socket *sock, uint8_t *ip, uint16_t port, uint32_t timeout_ms) {
	struct sockaddr_in addr;
	struct timeval tv;
	fd_set wfds;
	int retval;
	socklen_t optlen;
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip, &addr.sin_addr);

	// 创建socket，并设置为非阻塞
	sock->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock->fd < 0) {
        return -1;
    }
	fcntl(sock->fd, F_SETFL, fcntl(sock->fd, F_GETFL) | O_NONBLOCK);
	
	// 尝试连接，返回0表示连接成功，否则判断errno，
	// 如果errno被设为EINPROGRESS，表示connect仍旧在进行
	if (connect(sock->fd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
		return 0;
	}
	if (errno != EINPROGRESS) {
		return -1;
	}

	FD_ZERO(&wfds);
	FD_SET(sock->fd, &wfds);

	// 设置timeout，判断socket是否可写，如果可写，
	// 则用getsockopt得到error的值，若error值为0，表示connect成功
	retval = select(sock->fd + 1, NULL, &wfds, NULL, &tv);
	if (retval <= 0) {
		return -1;
	}
	optlen = sizeof(int);
	if (getsockopt(sock->fd, SOL_SOCKET, SO_ERROR, &retval, &optlen) < 0) {
		return -1;
	}
	if (0 == retval) {
		return 0;
	}

	return -1;
}

/*
 * 关闭连接
 */
int32_t tcp_close(tcp_socket *sock) {
    if (sock->fd < 0) {
        return -1;
    }
	close(sock->fd);
	return 0;
}

/*
 * 返回值为-1：表示连接断开
 * 返回值为0：表示在timeout_ms时间内没有读到数据
 * 返回值为正数：表示读取到的字节数
 */
int32_t tcp_read(tcp_socket *sock, uint8_t *buf, uint32_t n, uint32_t timeout_ms) {
	struct timeval tv;
	fd_set rfds;
	int retval;
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;
	FD_ZERO(&rfds);
	FD_SET(sock->fd, &rfds);

	retval = select(sock->fd + 1, &rfds, NULL, NULL, &tv);
	if (retval < 0) {
		return -1;
	}
	else if (0 == retval) {
		return 0;
	}

	// 不能保证全部读完，需要在上一层根据自己的协议做缓存
	retval = read(sock->fd, buf, n);
	if (retval <= 0) {
		return -1;
	}
	return retval;
}

/*
 * 返回值为-1：表示连接断开
 * 返回值为0：表示在timeout_ms时间内没有写入数据
 * 返回值为正数：表示写入的字节数
 */
int32_t tcp_write(tcp_socket *sock, uint8_t *buf, uint32_t n, uint32_t timeout_ms) {
	struct timeval tv;
	fd_set wfds;
	int retval;
	tv.tv_sec = timeout_ms / 1000;
	tv.tv_usec = (timeout_ms % 1000) * 1000;
	FD_ZERO(&wfds);
	FD_SET(sock->fd, &wfds);

	retval = select(sock->fd + 1, NULL, &wfds, NULL, &tv);
	if (retval < 0) {
		return -1;
	}
	else if (0 == retval) {
		return 0;
	}

	// 尽量将数据全部写到发送缓冲区
	uint32_t totlen = 0;
	while (totlen < n) {
		retval = write(sock->fd, buf + totlen, n - totlen);
		if (retval < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				continue;      // 发送缓冲区阻塞，选择continue或者break
			}
			else if (errno == EINTR) {  // 中断错误，继续写
				continue;
			}
			return -1;
		}
		totlen += retval;
	}

	return totlen;
}

int main() {
	tcp_socket sockfd;
    sockfd.fd = -1;
	int retval;
	retval = tcp_connect(&sockfd, "127.0.0.1", 8989, 5);
	printf("connect: %d\n", retval);

	uint8_t buf[200] = { 0 };
	char *s = "hello world";
	memcpy(buf, s, strlen(s));
	retval = tcp_write(&sockfd, buf, strlen(s), 5);
	printf("write: %d\n", retval);

	memset(buf, 0, sizeof(buf));
	retval = tcp_read(&sockfd, buf, sizeof(buf), 1);
	printf("read: %d [%s]\n", retval, buf);

	tcp_close(&sockfd);

	return 0;
}

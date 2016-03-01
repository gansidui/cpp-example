#include "base64.h"

static const char table64[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

// 将3个字节编码成4个字节
static void encode(unsigned char src[3], unsigned char dst[4]) {
	unsigned char buf[4];
	int i = 0;

	buf[0] = (src[0] & 0xfc) >> 2;
	buf[1] = ((src[0] & 0x03) << 4) + ((src[1] & 0xf0) >> 4);
	buf[2] = ((src[1] & 0x0f) << 2) + ((src[2] & 0xc0) >> 6);
	buf[3] = src[2] & 0x3f;

	for (i = 0; i < 4; ++i) {
		dst[i] = table64[buf[i]];
	}
}

// 将4个字节解码成3个字节
static void decode(unsigned char src[4], unsigned char dst[3]) {
	unsigned char buf[3], tmp[4];
	int i = 0, j = 0;

	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 64; ++j) {
			if (src[i] == table64[j]) {
				tmp[i] = j;
				break;
			}
		}
	}

	dst[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
	dst[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
	dst[2] = ((tmp[2] & 0x3) << 6) + tmp[3];
}

static bool is_base64_chars(unsigned char c) {
	if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || '+' == c || '/' == c) {
		return true;
	}
	return false;
}

bool base64_encode(const unsigned char *src, int src_len, unsigned char *dst, int dst_max_size, int *dst_len) {
	int i = 0, j = 0, enc_len = 0;
	unsigned char tmp[3];

	if (dst_max_size * 3 < src_len * 4 + 10) {
		return false;
	}

	while (src_len--) {
		tmp[i++] = *src++;
		if (3 == i) {
			encode(tmp, dst + enc_len);
			enc_len += 4;
			i = 0;
		}
	}

	if (i > 0) {
		for (j = i; j < 3; ++j) {
			tmp[j] = '\0';
		}
		encode(tmp, dst + enc_len);
		enc_len += (i + 1);

		while (i++ < 3) {
			dst[enc_len++] = '=';
		}
	}

	dst[enc_len] = '\0';
	*dst_len = enc_len;

	return true;
}

bool base64_decode(const unsigned char *src, int src_len, unsigned char *dst, int dst_max_size, int *dst_len) {
	int i = 0, j = 0, dec_len = 0;
	unsigned char tmp[4];

	if (dst_max_size * 4 < src_len * 3 + 10) {
		return false;
	}

	while (src_len--) {
		if ('=' == src[j]) {
			break;
		}
		// 判断src是否合法
		if (!is_base64_chars(src[j])) {
			return false;
		}

		tmp[i++] = src[j++];

		if (4 == i) {
			decode(tmp, dst + dec_len);
			dec_len += 3;
			i = 0;
		}
	}

	if (i > 0) {
		for (j = i; j < 4; ++j) {
			tmp[j] = '\0';
		}
		decode(tmp, dst + dec_len);
		dec_len += (i - 1);
	}

	dst[dec_len] = '\0';
	*dst_len = dec_len;

	return true;
}

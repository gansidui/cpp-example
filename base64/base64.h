#ifndef __BASE64_H__
#define __BASE64_H__

#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool base64_encode(const unsigned char *src, int src_len, unsigned char *dst, int dst_max_size, int *dst_len);

bool base64_decode(const unsigned char *src, int src_len, unsigned char *dst, int dst_max_size, int *dst_len);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __BASE64_H__ */

#include <stdio.h>
#include <string.h>
#include "base64.h"

int main() {
	char src[100], dst[100];
	int src_len, dst_len;
	
	while (scanf("%s", src)) {
		printf("%s\n", src);
		
		base64_encode(src, strlen(src), dst, sizeof(dst), &dst_len);
		printf("dst_len[%d]: %s\n", dst_len, dst);	
		
		
		base64_decode(dst, dst_len, src, sizeof(src), &src_len);
		printf("src_len[%d]: %s\n", src_len, src);
	}

	return 0;
}

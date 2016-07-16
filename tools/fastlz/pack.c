#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fastlz.h"
#include <openssl/md5.h>

#define HEXDUMP_COLS 8

int main(int argc, char** argv){
    
    char *source = NULL;
    int len, rlen, i;
    FILE *fp = fopen(argv[1], "r");
    if (fp != NULL) {
        /* Go to the end of the file. */
        if (fseek(fp, 0L, SEEK_END) == 0) {
            /* Get the size of the file. */
            len = ftell(fp);
            if (len == -1) { /* Error */ }

            /* Allocate our buffer to that size. */
            source = malloc(sizeof(char) * (len + 1));

            /* Go back to the start of the file. */
            if (fseek(fp, 0L, SEEK_SET) != 0) { /* Error */ }

            /* Read the entire file into memory. */
            size_t newLen = fread(source, sizeof(char), len, fp);
            if (newLen == 0) {
                fputs("Error reading file", stderr);
            } else {
                printf("Reading file with size=%i\n", len);
                source[newLen++] = '\0'; /* Just to be safe. */
            }
        }
        fclose(fp);
    }

    MD5_CTX md5_context;
    unsigned char c[MD5_DIGEST_LENGTH];
    unsigned char *packed;
    packed = (char*)malloc(len);

    MD5_Init (&md5_context); 
    MD5_Update (&md5_context, source, len);
    MD5_Final (c,&md5_context);
    printf("unpacked len=%i md5=", len);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    printf("\n");
    rlen = fastlz_compress(source, len, packed);
    printf("packed len=%i md5=", rlen);
    MD5_Init (&md5_context); 
    MD5_Update (&md5_context, packed, rlen);
    MD5_Final (c,&md5_context);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    printf("\n");
    fp = fopen(argv[2], "wb");
    fwrite(packed, rlen, 1, fp);
    printf("Wrote %s %l bytes\n", argv[2], len);
    fclose(fp);
}

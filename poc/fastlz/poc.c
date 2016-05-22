#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fastlz.h"
#include <openssl/md5.h>

#define HEXDUMP_COLS 8

void hexdump(void *mem, unsigned int len);

unsigned char *unpacked;
unsigned char *packed;

void init(long len){
    unpacked = (char*)malloc(len);
    packed = (char*)malloc(len);

    memset(unpacked, 0, len);
    memset(packed, 0, len);
}

int main(int argc, char** argv){
    
    char *source = NULL;
    int len, rlen, i;
    FILE *fp = fopen("/Users/david/Devel/arch/avr/code/quickdev16/roms/qd16boot02.smc", "r");
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
    char sample[] = "Lorem ipsum dolor sit amet, labore commodo platonem no vis, in ius atqui dicunt, omnium euismod nam in. Ei integre euismod philosophia mea. Qui fabulas comprehensam at, omnium corrumpit comprehensam mel no, sed audiam maiestatis et. Brute perfecto et his, mea cu inermis facilis scriptorem, eam te admodum urbanitas intellegam. At vis regione invidunt tractatos, ea modus errem has.";
   

    MD5_CTX md5_context;
    unsigned char c[MD5_DIGEST_LENGTH];

#if 0
    len = strlen(sample);
    init(len);
    memcpy(unpacked, sample, len);
#else 
    init(len);
    memcpy(unpacked, source, len);
#endif

    MD5_Init (&md5_context); 
    MD5_Update (&md5_context, unpacked, len);
    MD5_Final (c,&md5_context);
    printf("unpacked len=%i md5=", len);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    printf("\n");
    rlen = fastlz_compress(unpacked, len, packed);
    //hexdump(packed, rlen);
    printf("packed len=%i\n", rlen);
    memset(unpacked, 0, len);
    printf("-----\n");
#if 0
    fastlz_decompress(packed, rlen, unpacked);
    MD5_Init (&md5_context); 
    MD5_Update (&md5_context, unpacked, len);
    MD5_Final (c,&md5_context);
    printf("unpacked len=%i md5=", len);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    printf("\n");
    fp = fopen("out01.smc", "wb");
    fwrite(unpacked, len, 1, fp);
    printf("Wrote out01.smc %l bytes\n", len);
    fclose(fp);
    memset(unpacked, 0, len);
#endif 
    printf("-----\n");
    fastlz_decompress2(packed, rlen, unpacked);
    MD5_Init (&md5_context); 
    MD5_Update (&md5_context, unpacked, len);
    MD5_Final (c,&md5_context);
    printf("unpacked len=%i md5=", len);
    for(i = 0; i < MD5_DIGEST_LENGTH; i++) printf("%02x", c[i]);
    printf("\n");
    fp = fopen("out02.smc", "wb");
    fwrite(unpacked, len, 1, fp);
    printf("Wrote out02.smc %l bytes\n", len);
    fclose(fp);
    printf("s1=%s\n\n", sample);
    printf("s2=%s\n", unpacked);


    return 0;
}

void hexdump(void *mem, unsigned int len){
    unsigned int i, j;
    for(i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++)
    {
        /* print offset */
        if(i % HEXDUMP_COLS == 0)
        {
                printf("0x%06x: ", i);
        }

        /* print hex data */
        if(i < len)
        {
                printf("%02x ", 0xFF & ((char*)mem)[i]);
        }
        else /* end of block, just aligning for ASCII dump */
        {
                printf("   ");
        }
        
        /* print ASCII dump */
        if(i % HEXDUMP_COLS == (HEXDUMP_COLS - 1))
        {
            for(j = i - (HEXDUMP_COLS - 1); j <= i; j++)
            {
                    if(j >= len) /* end of block, not really printing */
                    {
                            putchar(' ');
                    }
                    else if(isprint(((char*)mem)[j])) /* printable char */
                    {
                            putchar(0xFF & ((char*)mem)[j]);        
                    }
                    else /* other char */
                    {
                            putchar('.');
                    }
            }
            putchar('\n');
        }
    }
}

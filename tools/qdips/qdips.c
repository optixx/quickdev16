#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>
#include <unistd.h>
#include <getopt.h>

#include "opendevice.h"
#include "qdips.h"


#define _16MBIT 0x200000

#define HIROM_BANK_SIZE_SHIFT  16
#define LOROM_BANK_SIZE_SHIFT  15
#define HIROM_BANK_COUNT_SHIFT 32
#define LOROM_BANK_COUNT_SHIFT 64

#define IPS_MAGIC "PATCH"
#define EOF_TAG   0x454f46
#define PATCH_END 1
#define PATCH_ERR 2
#define PATCH_CHUNKS_MAX 512

#define QD16_MAX_TX_LEN 128
#define QD16_CMD_TIMEOUT 5000


#define QD16_SEND_DATA(__addr_hi,__addr_lo,__src,__len)\
    usb_control_msg(handle,\
            USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,\
            USB_BULK_UPLOAD_NEXT,\
            __addr_hi, __addr_lo,\
            (char*)__src, __len,\
            QD16_CMD_TIMEOUT);

#define QD16_SEND_ADDR(__addr_hi,__addr_lo,__src,__len)\
    usb_control_msg(handle,\
            USB_TYPE_VENDOR|USB_RECIP_DEVICE|USB_ENDPOINT_OUT,\
            USB_BULK_UPLOAD_ADDR,\
            __addr_hi,__addr_lo,\
            (char*)__src,__len,\
            QD16_CMD_TIMEOUT);


typedef struct _ips_tag {
    uint32_t ofs;    /* offset in file to patch */
    uint16_t len;    /* number of bytes in this chunk */
    uint8_t* data;   /* chunk data */
} ips_tag_t;

int qd16_apply_ips_chunk(ips_tag_t* ips_tag);
int ips_read_next_tag(FILE* patch_file, ips_tag_t* tag);
int qd16_init(void);
void qd16_finish(void);
void usage(void);

usb_dev_handle *handle = NULL;
const unsigned char rawVid[2] = { USB_CFG_VENDOR_ID }, rawPid[2] = { USB_CFG_DEVICE_ID};
char vendor[] = { USB_CFG_VENDOR_NAME, 0 }, product[] = { USB_CFG_DEVICE_NAME, 0};
int cnt, vid, pid;

int hirom = 0;
int header = 0;
int init = 0;
int quiet = 0;

uint8_t dummy[128];

void usage(void)
{
    fprintf(stderr, "Usage: qdips [option] <patchfile>\n\n\
            Options:\n\
            -s\tSkip ROM header\n\
            -q\tBe quiet\n\
            -h\tForce HiROM mode\n\n");

    exit(1);
}

int main(int argc, char * argv[])
{
    FILE* patch_file;
    char * fname = 0;
    int opt = 0;

    char magic[6];
    ips_tag_t ips_tags[PATCH_CHUNKS_MAX];
    int chunk_index = 0;

    if (argc==1)
        usage();

    while ((opt = getopt(argc,argv,"qish")) != -1){
        switch (opt) {
            case 'i':
                init = 1;
                break;
            case 's':
                header = 1;
                break;
            case 'h':
                hirom = 1;
                break;
            case 'q':
                quiet = 1;
                break;
            default:
                usage();
        }
    }


    if (optind >= argc) {
        usage();
    }

    int ret = qd16_init();
    if (ret != 0){
        fprintf(stderr,"Error during init. Exiting!\n");
        exit(1);
    }

    fname = argv[optind];
    patch_file = fopen(fname,"rb");
    if (patch_file == 0) {
        fprintf(stderr,"Failed to open file '%s'!\n",fname);
        exit(1);
    }

    if (fgets(magic,strlen(IPS_MAGIC)+1,patch_file) == NULL) {
        fprintf(stderr,"Error reading from file %s\n",fname);
        fclose(patch_file);
        exit(1);
    }

    if (strcmp(magic,IPS_MAGIC) != 0){
        fprintf(stderr,"Patch is not in ips format!\n");
    }

    int patch_error = 0;

    while (!feof(patch_file) && chunk_index < PATCH_CHUNKS_MAX) {

        printf(".");

        int ret = ips_read_next_tag(patch_file,&ips_tags[chunk_index]);

        if (ret == PATCH_END)
            break;
        else if (ret == PATCH_ERR) {
            patch_error = 1;
            break;
        }

        chunk_index++;
    }

    fclose(patch_file);

    for (int i=0; i<chunk_index; i++){

        printf("uploading chunk %d ..\n",i);

        int ret = qd16_apply_ips_chunk(&ips_tags[i]);
        if (ret!=0){
            printf("failed applying chunk %d !\n",i);
        }
    }

    qd16_finish();

    return 0;
}

int qd16_apply_ips_chunk(ips_tag_t* ips_tag)
{
    int addr_hi = (ips_tag->ofs)>>16;
    int addr_lo = (ips_tag->ofs)&0xffff;

    printf("hi:%x lo:%x len:%d\n",addr_hi,addr_lo,ips_tag->len);
    int ret = QD16_SEND_ADDR(addr_hi,addr_lo,ips_tag->data,ips_tag->len);
    if (ret<0){
        printf("error executing command!:%s\n",usb_strerror());
    }

    return 0;
}

void qd16_finish(void)
{
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_BULK_UPLOAD_END, 0, 0, NULL,
        0, 5000);

    if (cnt<0) {
        printf("USB_BULK_UPLOAD_END failed: %s\n",usb_strerror());
    }

      cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_SET_LOADER, 1, 1, NULL,
        0, 5000); 

    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_MODE_SNES, 0, 0, NULL,
        0, 5000);
    if (cnt<0) {
        printf("USB_MODE_SNES failed: %s\n",usb_strerror());
    }

}

int qd16_init(void)
{
    usb_init();

    vid = rawVid[1] * 256 + rawVid[0];
    pid = rawPid[1] * 256 + rawPid[0];

    if (usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0) {
        fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
        return 1;
    }

    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_SET_LOADER, 0, 0, NULL,
        0, 5000); 

    if (cnt<0) {
        printf("USB_SET_LOADER failed: %s\n",usb_strerror());
    }

    cnt = usb_control_msg(handle,
            USB_TYPE_VENDOR | USB_RECIP_DEVICE |
            USB_ENDPOINT_OUT, USB_MODE_AVR, 0, 0, NULL,
            0, 5000);  

    if (cnt<0) {
        printf("USB_MODE_AVR failed: %s\n",usb_strerror());
    }

    usleep(500000);

    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
        USB_BULK_UPLOAD_INIT, 
        (hirom ? HIROM_BANK_SIZE_SHIFT : LOROM_BANK_SIZE_SHIFT) , 
        (hirom ? HIROM_BANK_COUNT_SHIFT : LOROM_BANK_COUNT_SHIFT), 
        NULL, 0, 5000);

    if (cnt<0) {
        printf("USB_BULK_UPLOAD_INIT failed: %s\n",usb_strerror());
    }

    return 0;
}

int ips_read_next_tag(FILE* patch_file, ips_tag_t* tag)
{

    tag->ofs = getc(patch_file)<<16 | getc(patch_file)<<8 | getc(patch_file);

    if (tag->ofs == EOF_TAG)
        return PATCH_END;
    if (tag->ofs > _16MBIT)
        return PATCH_ERR;

    tag->len = getc(patch_file)<<8 | getc(patch_file);
    tag->data = malloc(tag->len);

    size_t ret = fread(tag->data,1,tag->len,patch_file);
    if (ret != tag->len)
        return PATCH_ERR;

    return 0;
}

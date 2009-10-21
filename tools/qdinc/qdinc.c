#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>
#include <unistd.h>
#include <getopt.h>

#include "opendevice.h"
#include "qdinc.h"

#define _16MBIT 0x200000
#define HIROM_BANK_SIZE_SHIFT  16
#define LOROM_BANK_SIZE_SHIFT  15
#define HIROM_BANK_COUNT_SHIFT 32
#define LOROM_BANK_COUNT_SHIFT 64

static const char * default_tempfile = "/tmp/quickdev";


void usage(void)
{
    fprintf(stderr, "Usage: qdinc [option] <romfile>\n\n\
            Options:\n\
            -i\tInitial ROM upload\n\
            -s\tSkip ROM header\n\
            -q\tBe quiet\n\
            -h\tForce HiROM mode\n\n");

    exit(1);
}

int main(int argc, char * argv[])
{
    char * filename = 0;
    
    int hirom = 0;
    int header = 0;
    int init = 0;
    int quiet = 0;
    int opt = 0;

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

    filename = argv[optind];

    int j,j2;
    int ref_complete, upload_complete = 0;
    const char * tempfile_name = getenv("QDINC_FILE");
    
    if(tempfile_name == 0)
        tempfile_name = default_tempfile;
        
    char * outfile = malloc(strlen(tempfile_name) + sizeof(".out"));
    char * ext = stpcpy(outfile, tempfile_name);
    strcpy(ext, ".out");
    FILE * temp_file = fopen(outfile, "wb");
    
    if(temp_file == 0)
    {
        fprintf(stderr, "Could not open file %s for writing\n", outfile);
        exit(1);
    }
    
    
    FILE * new_rom = fopen(filename, "rb");
    
    if(new_rom == 0)
    {
        fprintf(stderr, "Could not open file %s\n", argv[3]);
        exit(1);
    }
    
    fseek(new_rom, 0, SEEK_END);
    unsigned int size = ftell(new_rom);
    
    if(header)
    {
        if(size < 512)
            size = 0;
        else
            size -= 512;
        fseek(new_rom, 512, SEEK_SET);
    }
    else
        rewind(new_rom);
    
    if(size > _16MBIT)
    {
        fprintf(stderr, "Input file is larger than supported by Quickdev16\n");
        exit(1);
    }
    
    FILE * ref = 0;
    if(!init)
        ref = fopen(tempfile_name, "rb");
    
    ref_complete = (ref == 0);

    usb_dev_handle *handle = NULL;
    const unsigned char rawVid[2] = { USB_CFG_VENDOR_ID }, rawPid[2] = { USB_CFG_DEVICE_ID};
    char vendor[] = { USB_CFG_VENDOR_NAME, 0 }, product[] = { USB_CFG_DEVICE_NAME, 0};
    int cnt, vid, pid;
    
    usb_init();
    vid = rawVid[1] * 256 + rawVid[0];
    pid = rawPid[1] * 256 + rawPid[0];
    if (usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0) {
        fprintf(stderr,
                "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n",
                product, vid, pid);
        exit(1);
    }

    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_SET_LOADER, 0, 0, NULL,
        0, 5000); 
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_MODE_AVR, 0, 0, NULL,
        0, 5000);  
        
    usleep(50000);
    
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
        USB_BULK_UPLOAD_INIT, 
        (hirom ? HIROM_BANK_SIZE_SHIFT : LOROM_BANK_SIZE_SHIFT) , 
        (hirom ? HIROM_BANK_COUNT_SHIFT : LOROM_BANK_COUNT_SHIFT) /* << this is wrong, but only used for progress display */, 
        NULL, 0, 5000);

    unsigned char diffblock[256];
    unsigned int blocklen = 0;
    unsigned int reallen = 0;
    unsigned int position = 0;
    unsigned int last_position = (unsigned int)(-1);
    
    unsigned char lastchar = 0;
    unsigned int transmitted = 0;
    
    
    char progress[21];
    memset(progress,' ',19); 
    progress[20] = 0;
    j = 0;
    j2 = 0;

    while(!upload_complete)
    {
        int msg = 0;
        unsigned char updchar = fgetc(new_rom);
        unsigned char refchar=0;

        if(feof(new_rom))
        {
            msg = 1;
            upload_complete = 1;
        }
        else
            fputc(updchar, temp_file);

        if(!ref_complete && !upload_complete)
        {
            refchar = fgetc(ref);
            if(feof(ref))
                ref_complete= 1;
        }
        
        int match = (ref_complete || updchar != refchar);
        if(upload_complete)
            match = 0;

        if(match)
        {
            diffblock[blocklen] = updchar;
            blocklen++;
            reallen = blocklen;
        }
        else if(blocklen > 0)
        {
            diffblock[blocklen] = updchar;
            blocklen++;
        }
        
        position++;
        if((reallen > 0 && upload_complete) || ((!upload_complete) && (( (blocklen - reallen) >= QDINC_RANGE && !match) || blocklen == QDINC_SIZE)) )
        {
            int consecutive = (last_position == position - blocklen);
            unsigned int begin = position - blocklen;
            
            cnt = usb_control_msg(handle,
                USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                USB_ENDPOINT_OUT, consecutive ? USB_BULK_UPLOAD_NEXT : USB_BULK_UPLOAD_ADDR, begin >> 16,
                begin & 0xFFFF, (char *) diffblock, 
                reallen, 5000);
            
            last_position = position - blocklen + reallen;
            transmitted += reallen;
            blocklen = 0;
            reallen = 0;
            
            msg = 1;
            
        }
        if(!quiet && msg)
        {
            for(; j < (position - 1) * 20 / size; j++)
                progress[j] = '=';
            for(; j2 < transmitted * 20 / size; j2++)
                progress[j2] = '#';
            printf("At 0x%06x/0x%06x %6.2f%% [%10s] Tx 0x%06x/0x%06x %6.2f%%\r",
                    position - 1, size, (position - 1) * 100.0 / size,
                    progress,
                    transmitted, size, transmitted * 100.0 / size);
            fflush(stdout);
        }
        lastchar = updchar;
    }
    position--;
    
    printf("\n");
    
    while(!ref_complete)
    {
        unsigned char refchar = fgetc(ref);
        if(feof(ref))
            ref_complete = 1;
        else
            fputc(refchar, temp_file);
    }
    
    fclose(temp_file);
    fclose(new_rom);
    if (!init)
        fclose(ref);
    
    if(rename(outfile, tempfile_name))
    {
        fprintf(stderr, "Could not replace tempfile %s\n", tempfile_name);
        exit(1);
    }
    
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_BULK_UPLOAD_END, 0, 0, NULL,
        0, 5000);
        
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_SET_LOADER, 1, 1, NULL,
        0, 5000); 
        
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_MODE_SNES, 0, 0, NULL,
        0, 5000);

    return 0;
}

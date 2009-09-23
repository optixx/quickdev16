#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <usb.h>

#include "opendevice.h"

#include "qdinc.h"

static const char * default_tempfile = "/tmp/quickdev";


void usage(void)
{
    fprintf(stderr, "usage: qdinc [-hsi] <romfile>\n");
    exit(1);
}

int main(int argc, char * argv[])
{
    char * filename = 0;
    
    int hirom = 0;
    int header = 0;
    int init = 0;
    int i,j,j2;
    for(i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            if(strcmp(argv[i], "--") == 0)
            {
                if(i != argc - 2 || filename)
                    usage();
                else
                {
                    filename = argv[i+1];
                    break;
                }
            }
            unsigned int len = strlen(argv[i]);
            if(len == 1)
                usage();
            for(j = 1; j < len; j++)
            {
                switch(argv[i][j])
                {
                case 'h':
                    hirom = 1;
                    break;
                case 's':
                    header = 1;
                    break;
                case 'i':
                    init = 1;
                    break;
                default:
                    usage();
                }
            }
        }
        else
        {
            if(filename != 0)
                usage();
            filename = argv[i];
        }
    }
    
    unsigned int pos = 0;
    int ref_complete, upd_complete = 0;
    
    const char * tempfile = getenv("QDINC_FILE");
    
    if(tempfile == 0)
        tempfile = default_tempfile;
        
    char * outfile = malloc(strlen(tempfile) + sizeof(".out"));
    char * ext = stpcpy(outfile, tempfile);
    strcpy(ext, ".out");
    
    FILE * out = fopen(outfile, "wb");
    
    if(out == 0)
    {
        fprintf(stderr, "Could not open file %s for writing\n", outfile);
        exit(1);
    }
    
    
    FILE * upd = fopen(filename, "rb");
    
    if(upd == 0)
    {
        fprintf(stderr, "Could not open file %s\n", argv[3]);
        exit(1);
    }
    
    fseek(upd, 0, SEEK_END);
    unsigned int size = ftell(upd);
    
    if(header)
    {
        if(size < 512)
            size = 0;
        else
            size -= 512;
        fseek(upd, 512, SEEK_SET);
    }
    else
        rewind(upd);
    
    if(size > 0x200000)
    {
        fprintf(stderr, "Input file is larger than supported by Quickdev16\n");
        exit(1);
    }
    
    FILE * ref = 0;
    if(!init)
        ref = fopen(tempfile, "rb");
    
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

#ifndef QDINC_OLD_FIRMWARE
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_SET_LAODER, 0, 0, NULL,
        0, 5000); 
#endif
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_MODE_AVR, 0, 0, NULL,
        0, 5000);  
        
#ifdef QDINC_OLD_FIRMWARE
    /* wait for the loader to depack */
    usleep(500000);
#else
    usleep(50000);
#endif
    
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_OUT,
        USB_BULK_UPLOAD_INIT, (hirom ? 16 : 15) , (hirom ? 32 : 64) /* << this is wrong, but only used for progress display */, NULL, 0, 5000);

    unsigned char diffblock[256];
    unsigned int blocklen = 0;
    unsigned int reallen = 0;
    unsigned int position = 0;
    unsigned int last_position = (unsigned int)(-1);
    
    unsigned char lastchar;
    unsigned int transmitted = 0;
    
    
    char progress[21];
    
    for(i = 0; i < 20; i++)
        progress[i] = ' ';
    progress[20] = 0;
    j = 0;
    j2 = 0;

    while(!upd_complete)
    {
        int msg = 0;
        unsigned char updchar = fgetc(upd);
        unsigned char refchar;
        if(feof(upd))
        {
            msg = 1;
            upd_complete = 1;
        }
        else
            fputc(updchar, out);
        if(!ref_complete && !upd_complete)
        {
            refchar = fgetc(ref);
            if(feof(ref))
                ref_complete= 1;
        }
        
        int match = (ref_complete || updchar != refchar);
#ifdef QDINC_OLD_FIRMWARE
        if(position < 64 * 1024)
            match = 1;
#endif
        if(upd_complete)
            match = 0;
        if(match)
        {
#ifdef QDINC_OLD_FIRMWARE
            if(blocklen == 0 && (position & (hirom ? 0xFFFF : 0x7FFF)) == 0 && position != 0 && position != last_position)
            {
                diffblock[0] = lastchar;
                blocklen++;
            }
#endif
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
        if((reallen > 0 && upd_complete) || ((!upd_complete) && (( (blocklen - reallen) >= QDINC_RANGE && !match) || blocklen == QDINC_SIZE)) )
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
        if(msg)
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
            fputc(refchar, out);
    }
    
    fclose(out);
    fclose(upd);
    fclose(ref);
    
    if(rename(outfile, tempfile))
    {
        fprintf(stderr, "Could not replace tempfile %s\n", tempfile);
        exit(1);
    }
    
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_BULK_UPLOAD_END, 0, 0, NULL,
        0, 5000);
        
#ifndef QDINC_OLD_FIRMWARE
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_SET_LAODER, 1, 1, NULL,
        0, 5000); 
#endif
        
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_MODE_SNES, 0, 0, NULL,
        0, 5000);
    return 0;
    
    printf("Done");
}

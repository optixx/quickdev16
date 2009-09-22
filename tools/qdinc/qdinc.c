#include <stdio.h>
#include <stdlib.h>

#include <usb.h>

#include "opendevice.h"

#include "qdinc.h"


int main(int argc, char * argv[])
{
    if(argc != 4 && argc != 3)
    {
        fprintf(stderr, "usage: qdinc [hl] <update> [reference]\n");
        exit(1);
    }
    
    int hirom = (argv[1][0] == 'h' || argv[1][0] == 'H');
    

    unsigned int pos = 0;
    int ref_complete, upd_complete = 0;
    
    FILE * upd = fopen(argv[2], "rb");
    
    if(upd == 0)
    {
        fprintf(stderr, "couldn't open file %s\n", argv[3]);
    }
    
    
    FILE * ref = 0;
    if(argc == 4)
        ref = fopen(argv[3], "rb");
    
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

    while(!upd_complete)
    {
        unsigned char updchar = fgetc(upd);
        unsigned char refchar;
        if(feof(upd))
            upd_complete = 1;
        if(!ref_complete)
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
            
            printf("updating %sblock from 0x%06x to 0x%06x of size 0x%03x\n", (consecutive ? "consecutive " : ""), begin, begin + reallen - 1, reallen);
            
            
            cnt = usb_control_msg(handle,
                USB_TYPE_VENDOR | USB_RECIP_DEVICE |
                USB_ENDPOINT_OUT, consecutive ? USB_BULK_UPLOAD_NEXT : USB_BULK_UPLOAD_ADDR, begin >> 16,
                begin & 0xFFFF, (char *) diffblock, 
                reallen, 5000);
            
            last_position = position - blocklen + reallen;
            transmitted += reallen;
            blocklen = 0;
            reallen = 0;
        }
        
        lastchar = updchar;
    }
    position--;
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
        
    printf("Transmitted %i of %i bytes (%3.2f)\n", transmitted, position, transmitted * 100.0 / position);
    cnt = usb_control_msg(handle,
        USB_TYPE_VENDOR | USB_RECIP_DEVICE |
        USB_ENDPOINT_OUT, USB_MODE_SNES, 0, 0, NULL,
        0, 5000);
    return 0;
    
}
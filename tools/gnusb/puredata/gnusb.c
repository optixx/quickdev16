// ==============================================================================
//	gnusb.c
//	
//	pd-Interface to the [ a n y m a | gnusb - Open Source USB Sensor Box ]
//	
//	Authors:	Michael Egger
//	Copyright:	2007 [ a n y m a ]
//	Website:	www.anyma.ch
//	
//	License:	GNU GPL 2.0 www.gnu.org
//	
//	Version:	2007-11-12
// ==============================================================================



#include "m_pd.h"

#include "../common/gnusb_cmds.h"		// codes used between gnusb client and host software, eg. between the max external and the gnusb firmware
#include </usr/local/include/usb.h>     // this is libusb, see http://libusb.sourceforge.net/ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==============================================================================
// Constants
// ------------------------------------------------------------------------------

#define USBDEV_SHARED_VENDOR    	0x16C0  /* VOTI */
#define USBDEV_SHARED_PRODUCT   	0x05DC  /* Obdev's free shared PID */
#define OUTLETS 					10
#define DEFAULT_CLOCK_INTERVAL		40		// default interval for polling the gnusb: 40ms

// ==============================================================================
// Our External's Memory structure
// ------------------------------------------------------------------------------

typedef struct _gnusb				// defines our object's internal variables for each instance in a patch
{
	t_object 		p_ob;					// object header - ALL max external MUST begin with this...
	usb_dev_handle	*dev_handle;			// handle to the gnusb usb device
	void			*m_clock;				// handle to our clock
	double 			m_interval;				// clock interval for polling the gnusb
	double 			m_interval_bak;			// backup clock interval for polling the gnusb
	int				is_running;				// is our clock ticking?
	int				do_10_bit;				// output analog values with 8bit or 10bit resolution?
	int				debug_flag;
	void 			*outlets[OUTLETS];		// handle to the objects outlets
	int 			values[10];				// stored values from last poll
} t_gnusb;

void *gnusb_class;					// global pointer to the object class - so max can reference the object 


// ==============================================================================
// Function Prototypes
// ------------------------------------------------------------------------------

void *gnusb_new(t_symbol *s);
void gnusb_assist(t_gnusb *x, void *b, long m, long a, char *s);
void gnusb_bang(t_gnusb *x);				
void gnusb_close(t_gnusb *x);
void gnusb_debug(t_gnusb *x,  long n);
void gnusb_int(t_gnusb *x,long n);
void gnusb_output(t_gnusb *x, t_symbol *s, long n);
void gnusb_input(t_gnusb *x, t_symbol *s);
void gnusb_precision(t_gnusb *x, t_symbol *s);
void gnusb_open(t_gnusb *x);
void gnusb_poll(t_gnusb *x, long n);
void gnusb_smooth(t_gnusb *x, long n);
void gnusb_start(t_gnusb *x);
void gnusb_stop(t_gnusb *x);

// functions used to find the USB device
static int  	usbGetStringAscii(usb_dev_handle *dev, int index, int langid, char *buf, int buflen);
void 			find_device(t_gnusb *x);



// ==============================================================================
// Implementation
// ------------------------------------------------------------------------------



//--------------------------------------------------------------------------
// - Message: output 		-> output a byte on port B or C
//--------------------------------------------------------------------------

void gnusb_output(t_gnusb *x, t_symbol *s, long n)
{
	int cmd;
	int nBytes;
	unsigned char buffer[8];
	
	cmd = 0;
	if (s == gensym("b")) cmd = GNUSB_CMD_SET_PORTB;
	else if (s == gensym("c")) cmd = GNUSB_CMD_SET_PORTC;
	else {
		post ("gnusb: unknown port\n");
		return;
	}
	
	if (n < 0) n = 0;
	if (n > 255) n = 255;
	
	if (!(x->dev_handle)) find_device(x);
	else {
		nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
							cmd, n, 0, (char *)buffer, sizeof(buffer), 10);
	}

	
}

//--------------------------------------------------------------------------
// - Message: input 		-> sets port to be an input
//--------------------------------------------------------------------------

void gnusb_input(t_gnusb *x, t_symbol *s)
{
	int cmd;
	int nBytes;
	unsigned char buffer[8];
	
	cmd = 0;
	if (s == gensym("b")) cmd = GNUSB_CMD_INPUT_PORTB;
	else if (s == gensym("c")) cmd = GNUSB_CMD_INPUT_PORTC;
	else {
		post ("gnusb: unknown port\n");
		return;
	}
	
	if (!(x->dev_handle)) find_device(x);
	else {
		nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
							cmd, 0, 0, (char *)buffer, sizeof(buffer), 10);
	}

	
}

//--------------------------------------------------------------------------
// - Message: precision 		-> 8 or 10 bit
//--------------------------------------------------------------------------

void gnusb_precision(t_gnusb *x, t_symbol *s)
{
	if (s == gensym("10bit")) x->do_10_bit = 1;
	else x->do_10_bit = 0;
}


//--------------------------------------------------------------------------
// - Message: debug
//--------------------------------------------------------------------------

void gnusb_debug(t_gnusb *x, long n)	// x = the instance of the object; n = the int received in the left inlet 
{
	if (n)	x->debug_flag = 1;
	else 	x->debug_flag = 0;
}
//--------------------------------------------------------------------------
// - Message: bang  -> poll the gnusb
//--------------------------------------------------------------------------

void gnusb_bang(t_gnusb *x)	// poll the gnusb
{
	int                 nBytes,i,n;
	int 				replymask,replyshift,replybyte;
	int					temp;
	unsigned char       buffer[12];
	
	if (!(x->dev_handle)) find_device(x);
	else {
			// ask the gnusb to send us data
			nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
										GNUSB_CMD_POLL, 0, 0, (char *)buffer, sizeof(buffer), 10);
			// let's see what has come back...							
			if(nBytes < sizeof(buffer)){
				if (x->debug_flag) {
					if(nBytes < 0)
						post( "USB error: %s\n", usb_strerror());
					post( "only %d bytes status received\n", nBytes);
				}
			} else {
				for (i = 0; i < OUTLETS; i++) {
					// n = OUTLETS - i - 1; // on max/msp outlets are reversed
					n = i;
					temp = buffer[n];
					
					
															// add 2 stuffed bits from end of buffer if we're doing 10bit precision
					if (n < 8) {
						if (x->do_10_bit) {
							
							if (n < 4)  replybyte = buffer[10];
							else replybyte = buffer[11];
							
							replyshift = ((n % 4) * 2);			// how much to shift the bits
							replymask = (3 << replyshift);
							
							temp = temp * 4 + ((replybyte & replymask) >> replyshift);	// add 2 LSB
							
						}
					}
					
					if (x->values[i] != temp) {					// output if value has changed
//max						outlet_int(x->outlets[i], temp);
						outlet_float(x->outlets[i], temp);
						x->values[i] = temp;
					}
				}
			}
	}
}


//--------------------------------------------------------------------------
// - Message: open 		-> open connection to gnusb
//--------------------------------------------------------------------------

void gnusb_open(t_gnusb *x)
{
	if (x->dev_handle) {
		post("gnusb: There is already a connection to www.anyma.ch/gnusb",0);
	} else find_device(x);
}

//--------------------------------------------------------------------------
// - Message: close 	-> close connection to gnusb
//--------------------------------------------------------------------------

void gnusb_close(t_gnusb *x)
{
	if (x->dev_handle) {
		usb_close(x->dev_handle);
		x->dev_handle = NULL;
		post("gnusb: Closed connection to www.anyma.ch/gnusb",0);
	} else
		post("gnusb: There was no open connection to www.anyma.ch/gnusb",0);
}

//--------------------------------------------------------------------------
// - Message: poll 		-> set polling interval
//--------------------------------------------------------------------------

void gnusb_poll(t_gnusb *x, long n){
	if (n > 0) { 
		x->m_interval = n;
		x->m_interval_bak = n;
		gnusb_start(x);
	} else {
		gnusb_stop(x);
	}
}

//--------------------------------------------------------------------------
// - Message: smooth 	-> set smoothing on analog inputs
//--------------------------------------------------------------------------

void gnusb_smooth(t_gnusb *x, long n) {
	int nBytes;
	unsigned char buffer[8];

	if (n < 0) n = 0;
	if (n > 15) n = 15;

	if (!(x->dev_handle)) find_device(x);
	else {
		nBytes = usb_control_msg(x->dev_handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
							GNUSB_CMD_SET_SMOOTHING, n, 0, (char *)buffer, sizeof(buffer), 10);
	}

}

//--------------------------------------------------------------------------
// - Message: int 		-> zero stops / nonzero starts
//--------------------------------------------------------------------------

void gnusb_int(t_gnusb *x,long n) {
	if (n) {
		if (!x->is_running) gnusb_start(x);
	} else {
		if (x->is_running) gnusb_stop(x);
	}
}

//--------------------------------------------------------------------------
// - Message: start 	-> start automatic polling
//--------------------------------------------------------------------------

void gnusb_start (t_gnusb *x) { 
	if (!x->is_running) {
		clock_delay(x->m_clock,0.);
		x->is_running  = 1;
	}
} 

//--------------------------------------------------------------------------
// - Message: stop 		-> stop automatic polling
//--------------------------------------------------------------------------

void gnusb_stop (t_gnusb *x) { 
	if (x->is_running) {
		x->is_running  = 0;
		clock_unset(x->m_clock); 
		gnusb_close(x);
	}
} 



//--------------------------------------------------------------------------
// - The clock is ticking, tic, tac...
//--------------------------------------------------------------------------

void gnusb_tick(t_gnusb *x) { 
//	clock_fdelay(x->m_clock, x->m_interval); 	// schedule another tick
	clock_delay(x->m_clock, x->m_interval); 	// schedule another tick
	gnusb_bang(x); 								// poll the gnusb
} 


//--------------------------------------------------------------------------
// - Object creation and setup
//--------------------------------------------------------------------------

int gnusb_setup(void)
{

	gnusb_class = class_new ( gensym("gnusb"),(t_newmethod)gnusb_new, 0, sizeof(t_gnusb), 	CLASS_DEFAULT,0);

	// setup() loads our external into Max's memory so it can be used in a patch
	// gnusb_new = object creation method defined below, A_DEFLONG = its (optional) arguement is a long (32-bit) int 
	
															// Add message handlers
	class_addbang(gnusb_class, (t_method)gnusb_bang);
//max	addint(gnusb_class, (t_method)gnusb_int);
	class_addfloat(gnusb_class, (t_method)gnusb_int);
	class_addmethod(gnusb_class, (t_method)gnusb_debug,gensym("debug"), A_DEFFLOAT, 0);
	class_addmethod(gnusb_class, (t_method)gnusb_open, gensym("open"), 0);		
	class_addmethod(gnusb_class, (t_method)gnusb_close, gensym("close"), 0);	
	class_addmethod(gnusb_class, (t_method)gnusb_poll, gensym("poll"), A_DEFFLOAT,0);	
	class_addmethod(gnusb_class, (t_method)gnusb_output, gensym("output"), A_DEFSYM,A_DEFFLOAT,0);	
	class_addmethod(gnusb_class, (t_method)gnusb_input, gensym("input"), A_DEFSYM,0);	
	class_addmethod(gnusb_class, (t_method)gnusb_precision, gensym("precision"), A_DEFSYM,0);	
	class_addmethod(gnusb_class, (t_method)gnusb_smooth, gensym("smooth"), A_DEFFLOAT,0);	
	class_addmethod(gnusb_class, (t_method)gnusb_start, gensym("start"), 0);	
	class_addmethod(gnusb_class, (t_method)gnusb_stop, gensym("stop"), 0);	

	post("gnusb version 1.0 - (c) 2007 [ a n y m a ]",0);	// post any important info to the max window when our object is laoded
	
	return 1;
}

//--------------------------------------------------------------------------

void *gnusb_new(t_symbol *s)		// s = optional argument typed into object box (A_SYM) -- defaults to 0 if no args are typed
{
	t_gnusb *x;										// local variable (pointer to a t_gnusb data structure)

//max	x = (t_gnusb *)newobject(gnusb_class); 			// create a new instance of this object
	x = (t_gnusb *)pd_new(gnusb_class);			 // create a new instance of this object
	x->m_clock = clock_new(x,(t_method)gnusb_tick); 	// make new clock for polling and attach gnsub_tick function to it

	if (s == gensym("10bit")) x->do_10_bit = 1;
	else  x->do_10_bit = 0;
	
	x->m_interval = DEFAULT_CLOCK_INTERVAL;
	x->m_interval_bak = DEFAULT_CLOCK_INTERVAL;

	x->debug_flag = 0;
	x->dev_handle = NULL;
	int i;
													// create outlets and assign it to our outlet variable in the instance's data structure
	for (i=0; i < OUTLETS; i++) {
		x->outlets[i] = outlet_new(&x->p_ob, &s_float);
//max		x->outlets[i] = intout(x);	
	}	

	return x;					// return a reference to the object instance 
}



//--------------------------------------------------------------------------
// - Object destruction
//--------------------------------------------------------------------------

void gnusb_free(t_gnusb *x)
{
	if (x->dev_handle) usb_close(x->dev_handle);
//	freeobject((t_object *)x->m_clock);  			// free the clock
	
	freebytes((t_object *)x->m_clock, sizeof(x->m_clock));

}





//--------------------------------------------------------------------------
// - USB Utility Functions
//--------------------------------------------------------------------------


static int  usbGetStringAscii(usb_dev_handle *dev, int index, int langid, char *buf, int buflen)
{
char    buffer[256];
int     rval, i;

    if((rval = usb_control_msg(dev, USB_ENDPOINT_IN, USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, langid, buffer, sizeof(buffer), 1000)) < 0)
        return rval;
    if(buffer[1] != USB_DT_STRING)
        return 0;
    if((unsigned char)buffer[0] < rval)
        rval = (unsigned char)buffer[0];
    rval /= 2;
    /* lossy conversion to ISO Latin1 */
    for(i=1;i<rval;i++){
        if(i > buflen)  /* destination buffer overflow */
            break;
        buf[i-1] = buffer[2 * i];
        if(buffer[2 * i + 1] != 0)  /* outside of ISO Latin1 range */
            buf[i-1] = '?';
    }
    buf[i-1] = 0;
    return i-1;
}

//--------------------------------------------------------------------------


void find_device(t_gnusb *x)
{
	usb_dev_handle      *handle = NULL;
	struct usb_bus      *bus;
	struct usb_device   *dev;
	
	usb_find_busses();
    usb_find_devices();
	 for(bus=usb_busses; bus; bus=bus->next){
        for(dev=bus->devices; dev; dev=dev->next){
            if(dev->descriptor.idVendor == USBDEV_SHARED_VENDOR && dev->descriptor.idProduct == USBDEV_SHARED_PRODUCT){
                char    string[256];
                int     len;
                handle = usb_open(dev); /* we need to open the device in order to query strings */
                if(!handle){
                    error ("Warning: cannot open USB device: %s", usb_strerror());
                    continue;
                }
                /* now find out whether the device actually is gnusb */
                len = usbGetStringAscii(handle, dev->descriptor.iManufacturer, 0x0409, string, sizeof(string));
                if(len < 0){
                    post("gnusb: warning: cannot query manufacturer for device: %s", usb_strerror());
                    goto skipDevice;
                }
                
			//	post("gnusb: seen device from vendor ->%s<-", string); 
                if(strcmp(string, "www.anyma.ch") != 0)
                    goto skipDevice;
                len = usbGetStringAscii(handle, dev->descriptor.iProduct, 0x0409, string, sizeof(string));
                if(len < 0){
                    post("gnusb: warning: cannot query product for device: %s", usb_strerror());
                    goto skipDevice;
                }
              //  post("gnusb: seen product ->%s<-", string);
                if(strcmp(string, "gnusb") == 0)
                    break;
skipDevice:
                usb_close(handle);
                handle = NULL;
            }
        }
        if(handle)
            break;
    }
	
    if(!handle){
        post("gnusb: Could not find USB device www.anyma.ch/gnusb");
		x->dev_handle = NULL;
		if (x->m_interval < 10000) x->m_interval *=2; // throttle polling down to max 20s if we can't find a gnusb
	} else {
		x->dev_handle = handle;
		 post("gnusb: Found USB device www.anyma.ch/gnusb");
		 x->m_interval = x->m_interval_bak;			// restore original polling interval
		 if (x->is_running) gnusb_tick(x);
		 else gnusb_bang(x);
	}
}
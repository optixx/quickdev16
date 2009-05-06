/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This file contains function prototypes and data types used to implement
Pd, but not shared with Pd objects. */

#include "m_pd.h"

/* LATER consider whether to use 'char' for method arg types to save space */

/* the structure for a method handler ala Max */
typedef struct _methodentry
{
    t_symbol *me_name;
    t_gotfn me_fun;
    t_atomtype me_arg[MAXPDARG+1];
} t_methodentry;

EXTERN_STRUCT _widgetbehavior;

typedef void (*t_bangmethod)(t_pd *x);
typedef void (*t_pointermethod)(t_pd *x, t_gpointer *gp);
typedef void (*t_floatmethod)(t_pd *x, t_float f);
typedef void (*t_symbolmethod)(t_pd *x, t_symbol *s);
typedef void (*t_listmethod)(t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_anymethod)(t_pd *x, t_symbol *s, int argc, t_atom *argv);

struct _class
{
    t_symbol *c_name;	    	    	/* name (mostly for error reporting) */
    t_symbol *c_helpname;   	    	/* name of help file */
    size_t c_size;  	    	    	/* size of an instance */
    t_methodentry *c_methods;	    	/* methods other than bang, etc below */
    int c_nmethod;  	    	    	/* number of methods */
    t_method c_freemethod;	    	/* function to call before freeing */
    t_bangmethod c_bangmethod;	    	/* common methods */
    t_pointermethod c_pointermethod;
    t_floatmethod c_floatmethod;
    t_symbolmethod c_symbolmethod;
    t_listmethod c_listmethod;
    t_anymethod c_anymethod;
    struct _widgetbehavior *c_wb; 	/* "gobjs" only */
    struct _parentwidgetbehavior *c_pwb;/* widget behavior in parent */
    int c_floatsignalin; 	    	/* onset to float for signal input */
    char c_gobj;	    		/* true if is a gobj */
    char c_patchable;	    	    	/* true if we have a t_object header */
    char c_firstin; 	    	    /* if patchable, true if draw first inlet */
    char c_drawcommand; 	    /* a drawing command for a template */
};

/* s_file.c */
typedef struct _namelist
{
    struct _namelist *nl_next;
    char *nl_string;
} t_namelist;

t_namelist *namelist_append(t_namelist *listwas, const char *s);
void namelist_free(t_namelist *listwas);

extern int sys_debuglevel;
extern int sys_verbose;

#define DEBUG_MESSUP 1	    /* messages up from pd to pd-gui */
#define DEBUG_MESSDOWN 2    /* messages down from pd-gui to pd */

extern int sys_noloadbang;
extern int sys_nogui;
extern char *sys_guicmd;

/* in s_main.c */
EXTERN int sys_nearestfontsize(int fontsize);
EXTERN int sys_hostfontsize(int fontsize);

extern int sys_defaultfont;
extern t_symbol *sys_libdir;	/* library directory for auxilliary files */
/* s_loader.c */
int sys_load_lib(char *dirname, char *filename);

/* s_unix.c */
EXTERN void sys_microsleep(int microsec);

/* s_sgi.c, s_nt.c, s_linux.c each implement the same API for audio
and MIDI I/O as follows: */

#define DACBLKSIZE 64

#define SENDDACS_NO 0	    	/* return values for sys_send_dacs() */
#define SENDDACS_YES 1 
#define SENDDACS_SLEPT 2

#define API_OSS 0   	/* API choices */
#define API_ALSA 1
#define API_RME 2
#define API_MMIO 3
#define API_PORTAUDIO 4


    /* MIDI input and output */
#define MAXMIDIINDEV 16    	/* max. number of input ports */
#define MAXMIDIOUTDEV 16    	/* max. number of output ports */
extern int sys_nmidiin;
extern int sys_nmidiout;
extern int sys_midiindevlist[];
extern int sys_midioutdevlist[];

EXTERN void sys_putmidimess(int portno, int a, int b, int c);
EXTERN void sys_putmidibyte(int portno, int a);
EXTERN void sys_poll_midi(void);
EXTERN void sys_setmiditimediff(double inbuftime, double outbuftime);
EXTERN void sys_midibytein(int portno, int byte);

extern int sys_hipriority;   	/* real-time flag, true if priority boosted */
extern t_sample *sys_soundout;
extern t_sample *sys_soundin;
extern float sys_dacsr;
extern int sys_schedadvance;
extern int sys_sleepgrain;
EXTERN void sys_open_audio(int naudioindev, int *audioindev, int nchindev, int *chindev,
			   int naudiooutdev, int *audiooutdev, int nchoutdev, int *choutdev,
			   int srate); /* IOhannes */

EXTERN void sys_close_audio(void);

EXTERN void sys_open_midi(int nmidiin, int *midiinvec,
    int nmidiout, int *midioutvec);
EXTERN void sys_close_midi(void);

EXTERN int sys_send_dacs(void);
EXTERN void sys_reportidle(void);
EXTERN void sys_set_priority(int higher);
EXTERN void sys_audiobuf(int nbufs);
EXTERN void sys_getmeters(float *inmax, float *outmax);
EXTERN void sys_listdevs(void);
EXTERN void sys_setblocksize(int n);

    /* for NT and Linux, there are additional bits of fluff as follows. */
#ifdef NT
EXTERN void nt_soundindev(int which);
EXTERN void nt_soundoutdev(int which);
EXTERN void nt_midiindev(int which);
EXTERN void nt_midioutdev(int which);
EXTERN void nt_noresync( void);
EXTERN void nt_set_sound_api(int which);
#endif

#ifdef __linux__
    /* the following definitions allow you to switch at run time
    between audio APIs in Linux and later in NT.  */
void linux_set_sound_api(int which);

void linux_setfrags(int n);
void linux_streammode( void);
void linux_32bit( void);
void rme_soundindev(int which);
void rme_soundoutdev(int which);
void linux_alsa_queue_size(int size);
#ifdef ALSA99   /* old fashioned ALSA */
void linux_alsa_devno(int devno);
#else
void linux_alsa_devname(char *devname);
#endif
#endif /* __linux__ */

/* portaudio, used in Windows and Mac versions... */

int pa_open_audio(int inchans, int outchans, int rate, t_sample *soundin,
    t_sample *soundout, int framesperbuf, int nbuffers,
    int indeviceno, int outdeviceno);
void pa_close_audio(void);
int pa_send_dacs(void);
void pa_reportidle(void);
void pa_listdevs(void);

/* m_sched.c */
EXTERN void sys_log_error(int type);
#define ERR_NOTHING 0
#define ERR_ADCSLEPT 1
#define ERR_DACSLEPT 2
#define ERR_RESYNC 3
#define ERR_DATALATE 4

/* s_inter.c */

EXTERN void sys_bail(int exitcode);
EXTERN int sys_pollgui(void);

EXTERN_STRUCT _socketreceiver;
#define t_socketreceiver struct _socketreceiver

typedef void (*t_socketnotifier)(void *x);
typedef void (*t_socketreceivefn)(void *x, t_binbuf *b);

EXTERN t_socketreceiver *socketreceiver_new(void *owner,
    t_socketnotifier notifier, t_socketreceivefn socketreceivefn, int udp);
EXTERN void socketreceiver_read(t_socketreceiver *x, int fd);
EXTERN void sys_sockerror(char *s);
EXTERN void sys_closesocket(int fd);

typedef void (*t_fdpollfn)(void *ptr, int fd);
EXTERN void sys_addpollfn(int fd, t_fdpollfn fn, void *ptr);
EXTERN void sys_rmpollfn(int fd);
#ifdef UNIX
void sys_setalarm(int microsec);
void sys_setvirtualalarm( void);
#endif

/* m_obj.c */
EXTERN int obj_noutlets(t_object *x);
EXTERN int obj_ninlets(t_object *x);
EXTERN t_outconnect *obj_starttraverseoutlet(t_object *x, t_outlet **op,
    int nout);
EXTERN t_outconnect *obj_nexttraverseoutlet(t_outconnect *lastconnect,
    t_object **destp, t_inlet **inletp, int *whichp);
EXTERN t_outconnect *obj_connect(t_object *source, int outno,
    t_object *sink, int inno);
EXTERN void obj_disconnect(t_object *source, int outno, t_object *sink,
    int inno);
EXTERN void outlet_setstacklim(void);
/* misc */
EXTERN void glob_evalfile(t_pd *ignore, t_symbol *name, t_symbol *dir);
EXTERN void glob_initfromgui(void *dummy, t_symbol *s, int argc, t_atom *argv);

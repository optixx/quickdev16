/**
	@file
	primitive types used in the USB stack
	
	(c) 2006, Bertrik Sikken, bertrik@sikken.nl
 */


#ifndef _TYPE_H_
#define _TYPE_H_

typedef unsigned char		U8;		/**< unsigned 8-bit */
typedef unsigned short int	U16;	/**< unsigned 16-bit */
typedef unsigned int		U32;	/**< unsigned 32-bit */

typedef int					BOOL;	/**< #TRUE or #FALSE */

#define	TRUE	1					/**< TRUE */
#define FALSE	0					/**< FALSE */

#ifndef NULL
#define NULL	((void*)0)			/**< NULL pointer */
#endif

/* some other useful macros */
#define MIN(x,y)	((x)<(y)?(x):(y))	/**< MIN */
#define MAX(x,y)	((x)>(y)?(x):(y))	/**< MAX */


#endif /* _TYPE_H_ */


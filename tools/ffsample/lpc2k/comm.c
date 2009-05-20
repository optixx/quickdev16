#include <string.h>
#include "LPC2300.h"
#include "interrupt.h"
#include "comm.h"


#define BUFFER_SIZE 128

#define PCLK		18000000
#define BPS 		230400
#define	DIVADDVAL	5
#define	MULVAL		8
#define	DLVAL		((int)((double)PCLK / BPS / 16 / (1 + (double)DIVADDVAL / MULVAL)))


static volatile struct
{
	int		rptr;
	int		wptr;
	int		count;
	BYTE	buff[BUFFER_SIZE];
} TxFifo0, RxFifo0;
static volatile int TxRun0;




void Isr_UART0 (void)
{
	int d, idx, cnt, iir;


	for (;;) {
		iir = U0IIR;			/* Get Interrupt ID*/
		if (iir & 1) break;		/* Exit if there is no interrupt */
		switch (iir & 6) {
		case 4:			/* Receive FIFO is half filled or timeout occured */
			idx = RxFifo0.wptr;
			cnt = RxFifo0.count;
			while (U0LSR & 0x01) {			/* Receive all data in the FIFO */
				d = U0RBR;
				if (cnt < BUFFER_SIZE) {	/* Store data if buffer is not full */
					RxFifo0.buff[idx] = d;
					cnt++;
					idx = (idx + 1) % BUFFER_SIZE;
				}
			}
			RxFifo0.wptr = idx;
			RxFifo0.count = cnt;
			break;

		case 2:			/* Transmisson FIFO empty */
			cnt = TxFifo0.count;
			if (cnt) {
				idx = TxFifo0.rptr;
				for (d = 12; d && cnt; d--, cnt--) {	/* Store data into FIFO (max 12 chrs) */
					U0THR = TxFifo0.buff[idx];
					idx = (idx + 1) % BUFFER_SIZE;
				}
				TxFifo0.rptr = idx;
				TxFifo0.count = cnt;
			} else {
				TxRun0 = 0;				/* When no data in the buffer, clear running flag */
			}
			break;

		default:		/* Data error or break detected */
			d = U0LSR;
			d = U0RBR;
			break;
		}
	}
}




int uart0_test (void)
{
	return RxFifo0.count;
}




BYTE uart0_get (void)
{
	BYTE d;
	int idx;

	/* Wait while Rx buffer is empty */
	while (!RxFifo0.count);

	U0IER = 0;				/* Disable interrupts */
	idx = RxFifo0.rptr;
	d = RxFifo0.buff[idx];	/* Get a byte from Rx buffer */
	RxFifo0.rptr = (idx + 1) % BUFFER_SIZE;
	RxFifo0.count--;
	U0IER = 0x07;			/* Enable interrupt */

	return d;
}




void uart0_put (BYTE d)
{
#if 0
	while (!(U0LSR & 0x20));
	U0THR = d;
#else
	int idx, cnt;

	/* Wait for buffer ready */
	while (TxFifo0.count >= BUFFER_SIZE);

	U0IER = 0x05;		/* Disable Tx Interrupt */
	if (!TxRun0) {		/* When not in runnig, trigger transmission */
		U0THR = d;
		TxRun0 = 1;
	} else {			/* When transmission is runnig, store the data into the Tx buffer */
		cnt = TxFifo0.count;
		idx = TxFifo0.wptr;
		TxFifo0.buff[idx] = d;
		TxFifo0.wptr = (idx + 1) % BUFFER_SIZE;
		TxFifo0.count = ++cnt;
	}
	U0IER = 0x07;		/* Enable Tx Interrupt */
#endif
}




void uart0_init (void)
{
	U0IER = 0x00;
	RegisterVector(UART0_INT, Isr_UART0, PRI_LOWEST, CLASS_IRQ);

	/* Attach UART0 unit to I/O pad */
	PINSEL0 = (PINSEL0 & 0xFFFFFF0F) | 0x50;

	/* Initialize UART0 */
	U0LCR = 0x83;			/* Select divisor latch */
	U0DLM = DLVAL / 256;	/* Initialize BRG */
	U0DLL = DLVAL % 256;
	U0FDR = (MULVAL << 4) | DIVADDVAL;
	U0LCR = 0x03;			/* Set serial format N81 and deselect divisor latch */
	U0FCR = 0x87;			/* Enable FIFO */
	U0TER = 0x80;			/* Enable Tansmission */

	/* Clear Tx/Rx FIFOs */
	TxFifo0.rptr = 0;
	TxFifo0.wptr = 0;
	TxFifo0.count = 0;
	RxFifo0.rptr = 0;
	RxFifo0.wptr = 0;
	RxFifo0.count = 0;

	/* Enable Tx/Rx/Error interrupts */
	U0IER = 0x07;
}



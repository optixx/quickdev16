#ifndef __INTERRUPT_H
#define __INTERRUPT_H


/* Interrupt related service functions (These functions are defined in Startup.S) */

void RegisterVector (int IntNum, void(*Isr)(void), int Priority, int IntClass);
void ClearVector (void);
void FiqEnable (void);
void FiqDisable (void);
void IrqEnable (void);
void IrqDisable (void);



/* LPC23xx interrupt number */

#define	WDT_INT			0
#define RES1_INT		1
#define ARM_CORE0_INT	2
#define	ARM_CORE1_INT	3
#define	TIMER0_INT		4
#define TIMER1_INT		5
#define UART0_INT		6
#define	UART1_INT		7
#define	PWM0_1_INT		8
#define I2C0_INT		9
#define SPI0_INT		10
#define SSP0_INT		10
#define	SSP1_INT		11
#define	PLL_INT			12
#define RTC_INT			13
#define EINT0_INT		14
#define EINT1_INT		15
#define EINT2_INT		16
#define EINT3_INT		17
#define	ADC0_INT		18
#define I2C1_INT		19
#define BOD_INT			20
#define EMAC_INT		21
#define USB_INT			22
#define CAN_INT			23
#define MCI_INT			24
#define GPDMA_INT		25
#define TIMER2_INT		26
#define TIMER3_INT		27
#define UART2_INT		28
#define UART3_INT		29
#define I2C2_INT		30
#define I2S_INT			31

#define CLASS_IRQ		0
#define CLASS_FIQ		1
#define PRI_LOWEST		15
#define PRI_HIGHEST		0


#endif /* __INTERRUPT_H */

void uart_init(void);		/* Initialize UART and Flush FIFOs */
uint8_t uart_get (void);	/* Get a byte from UART Rx FIFO */
uint8_t uart_test(void);	/* Check number of data in UART Rx FIFO */
void uart_put (uint8_t);	/* Put a byte into UART Tx FIFO */

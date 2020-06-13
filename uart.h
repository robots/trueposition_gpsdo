#ifndef UART_h_
#define UART_h_

typedef void (*uart_cb_t) (char);

void uart_set_cb(uart_cb_t cb);
void uart_send(const char *buf, uint32_t len);
void uart_init(void);
void uart_disable_int(void);
void uart_enable_int(void);

#endif

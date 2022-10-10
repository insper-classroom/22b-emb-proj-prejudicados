#ifndef UART_CONFIG_H_
#define UART_CONFIG_H_

#define USART_COM_ID ID_USART1
#define USART_COM USART1

void configure_console(void);
uint32_t usart_puts(uint8_t *pstring);
void usart_put_string(Usart *usart, char str[]);
int usart_get_string(Usart *usart, char buffer[], int bufferlen, uint timeout_ms);
void usart_send_command(Usart *usart, char buffer_rx[], int bufferlen, char buffer_tx[], int timeout);
void config_usart0(void);

#endif /* UART_CONFIG_H_ */
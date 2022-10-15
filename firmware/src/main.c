// INCLUDES
#include <asf.h>
#include "conf_board.h"
#include "uart_config/uart_config.h"
#include "rtos_config/rtos_config.h"
#include <string.h>

/* --- --- --- --- --- --- --- --- --- --- --- --- */
// DEFINES

// Definições da Placa

// Led Embutido
#define LED_PIO PIOC
#define LED_PIO_ID ID_PIOC
#define LED_PIO_IDX 8
#define LED_PIO_IDX_MASK (1 << LED_PIO_IDX)

// Botão Esquerda
#define BUTLEFT_PIO      PIOD
#define BUTLEFT_PIO_ID   ID_PIOD
#define BUTLEFT_IDX      30
#define BUTLEFT_IDX_MASK (1 << BUTLEFT_IDX)

// Botão Direita
#define BUTRIGHT_PIO      PIOA
#define BUTRIGHT_PIO_ID   ID_PIOA
#define BUTRIGHT_IDX      6
#define BUTRIGHT_IDX_MASK (1 << BUTRIGHT_IDX)

// Botão Iniciar Jogo
#define BUTSTART_PIO      PIOC
#define BUTSTART_PIO_ID   ID_PIOC
#define BUTSTART_IDX      31
#define BUTSTART_IDX_MASK (1 << BUTSTART_IDX)


// Definições da Task
#define TASK_BLUETOOTH_STACK_SIZE (4096/sizeof(portSTACK_TYPE))
#define TASK_BLUETOOTH_STACK_PRIORITY (tskIDLE_PRIORITY)

// Variáveis Globais

/* Coloque seus defines aqui /*
/* --- --- --- --- --- --- --- --- --- --- --- --- */
// RECURSOS RTOS

/* Coloque os semáforos e filas aqui /*
/* --- --- --- --- --- --- --- --- --- --- --- --- */
// PROTOTYPES

void init_led(Pio *pio, uint32_t id, uint32_t mask);
//void init_but(Pio *pio, uint32_t id, uint32_t mask);
void init_but(void);
int init_hc05(void);
void joystickR_callback(void);
void joystickL_callback(void);
void startbut_callback(void);

/* --- --- --- --- --- --- --- --- --- --- --- --- */
// GLOBAL VARIABLES

QueueHandle_t xQueueProtocolo;

/* --- --- --- --- --- --- --- --- --- --- --- --- */
// HANDLERS E CALLBACKS

void startbut_callback(void){
	char id = '1';
	xQueueSendFromISR(xQueueProtocolo, &id, 0);
}

void joystickR_callback(void){
	char id = '2';
	xQueueSendFromISR(xQueueProtocolo, &id, 0);
}

void joystickL_callback(void){
	char id = '3';
	xQueueSendFromISR(xQueueProtocolo, &id, 0);
}

/* Coloque suas funções de callback aqui /*
/* --- --- --- --- --- --- --- --- --- --- --- --- */
// TASKS

void task_bluetooth(void) {
	printf("Task Bluetooth started \n");
	printf("Inicializando HC05 \n");

	// Inits e Configurações
	config_usart0();
	init_hc05();
	init_led(LED_PIO, LED_PIO_ID, LED_PIO_IDX_MASK);
	init_but();
// 	init_but(BUTLEFT_PIO, BUTLEFT_PIO_ID, BUTLEFT_IDX_MASK);
// 	init_but(BUTRIGHT_PIO, BUTRIGHT_PIO_ID, BUTRIGHT_IDX_MASK);
// 	init_but(BUTSTART_PIO, BUTSTART_PIO_ID, BUTSTART_IDX_MASK);

// 	char button1 = '0';
// 	char button2 = '0';
	char id = '0';
	char eof = 'X';

// 	while(1){
// 	if(pio_get(BUTLEFT_PIO, PIO_INPUT, BUTLEFT_IDX_MASK) == 0) {
// 	button1 = '1';
// 	} else {
// 	button1 = '0';
// 	}
// 	if(pio_get(BUTRIGHT_PIO, PIO_INPUT, BUTRIGHT_IDX_MASK) == 0) {
// 	button2 = '1';
// 	} else {
// 	button2 = '0';
// 	}
// 	if(pio_get(BUTSTART_PIO, PIO_INPUT, BUTSTART_IDX_MASK) == 0) {
// 		button3 = '1';
// 		} else {
// 		button3 = '0';
// 	}

	
	for(;;){
		if( xQueueReceive(xQueueProtocolo, &id, ( TickType_t ) 500 )){
			
			printf("id = %c \n", id);
			
			while(!usart_is_tx_ready(USART_COM)){
				vTaskDelay(10/portTICK_PERIOD_MS);
			}
			usart_write(USART_COM, id);
			
			while(!usart_is_tx_ready(USART_COM)){
				vTaskDelay(10/portTICK_PERIOD_MS);
			}
			
			usart_write(USART_COM, eof);
			vTaskDelay(500 / portTICK_PERIOD_MS);
			
		}
	}
// 	printf("button1 = %c \n", button1);
// 	printf("button2 = %c \n", button2);
// 	printf("button3 = %c \n", button3);
// 	
// 	while(!usart_is_tx_ready(USART_COM)){
// 	vTaskDelay(10/portTICK_PERIOD_MS);
// 	}
// 	usart_write(USART_COM, button1);
// 
// 	while(!usart_is_tx_ready(USART_COM)){
// 	vTaskDelay(10/portTICK_PERIOD_MS);
// 	}
// 	usart_write(USART_COM, button2);
// 	
// 	while(!usart_is_tx_ready(USART_COM)){
// 		vTaskDelay(10/portTICK_PERIOD_MS);
// 	}
// 	usart_write(USART_COM, button3);
// 
// 	while(!usart_is_tx_ready(USART_COM)){
// 	vTaskDelay(10/portTICK_PERIOD_MS);
// 	}
// 	usart_write(USART_COM, eof);
// 
// 	vTaskDelay(500 / portTICK_PERIOD_MS);
// 	}
}

/* --- --- --- --- --- --- --- --- --- --- --- --- */
// FUNÇÕES

/* --- --- --- --- --- --- --- --- --- --- --- --- */
// INICIALIZAÇÕES

void init_led(Pio *pio, uint32_t id, uint32_t mask) {
// Config do LED
pmc_enable_periph_clk(id);
pio_set_output(pio, mask, 0, 0, 0);
}

// void init_but(Pio *pio, uint32_t id, uint32_t mask) {
// // Config do Botão
// pmc_enable_periph_clk(id);
// pio_set_input(pio, mask, PIO_DEFAULT);
// pio_pull_up(pio, mask, 1);
// pio_set_debounce_filter(pio, mask, 60);
// }


void init_but(void){
	pmc_enable_periph_clk(BUTLEFT_PIO_ID);
	pmc_enable_periph_clk(BUTRIGHT_PIO_ID);
	pmc_enable_periph_clk(BUTSTART_PIO_ID);
	
	/* conf botão como entrada */
	pio_configure(BUTLEFT_PIO, PIO_INPUT, BUTLEFT_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_configure(BUTRIGHT_PIO, PIO_INPUT, BUTRIGHT_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	pio_configure(BUTSTART_PIO, PIO_INPUT, BUTSTART_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	
	pio_handler_set(BUTLEFT_PIO, BUTLEFT_PIO_ID, BUTLEFT_IDX_MASK, PIO_IT_FALL_EDGE , joystickL_callback);
	pio_handler_set(BUTRIGHT_PIO, BUTRIGHT_PIO_ID, BUTRIGHT_IDX_MASK, PIO_IT_FALL_EDGE, joystickR_callback);
	pio_handler_set(BUTSTART_PIO, BUTSTART_PIO_ID, BUTSTART_IDX_MASK, PIO_IT_FALL_EDGE, startbut_callback);
	
	pio_enable_interrupt(BUTLEFT_PIO, BUTLEFT_IDX_MASK);
	pio_enable_interrupt(BUTRIGHT_PIO, BUTRIGHT_IDX_MASK);
	pio_enable_interrupt(BUTSTART_PIO, BUTSTART_IDX_MASK);
	
	pio_get_interrupt_status(BUTLEFT_PIO);
	pio_get_interrupt_status(BUTRIGHT_PIO);
	pio_get_interrupt_status(BUTSTART_PIO);
	
	NVIC_EnableIRQ(BUTLEFT_PIO_ID);
	NVIC_SetPriority(BUTLEFT_PIO_ID, 4);
	
	NVIC_EnableIRQ(BUTRIGHT_PIO_ID);
	NVIC_SetPriority(BUTRIGHT_PIO_ID, 4);

	NVIC_EnableIRQ(BUTSTART_PIO_ID);
	NVIC_SetPriority(BUTSTART_PIO_ID, 4);
}

int init_hc05(void) {
char buffer_rx[128];
usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
vTaskDelay( 500 / portTICK_PERIOD_MS);
usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
vTaskDelay( 500 / portTICK_PERIOD_MS);
usart_send_command(USART_COM, buffer_rx, 1000, "AT+NAMESpaceInvaders", 100);
vTaskDelay( 500 / portTICK_PERIOD_MS);
usart_send_command(USART_COM, buffer_rx, 1000, "AT", 100);
vTaskDelay( 500 / portTICK_PERIOD_MS);
usart_send_command(USART_COM, buffer_rx, 1000, "AT+PIN1234", 100);
}

/* --- --- --- --- --- --- --- --- --- --- --- --- */
// MAIN

int main(void) {
/* Initialize the SAM system */
sysclk_init();
board_init();

configure_console();

/* Create task to make led blink */
xTaskCreate(task_bluetooth, "BLT", TASK_BLUETOOTH_STACK_SIZE, NULL,	TASK_BLUETOOTH_STACK_PRIORITY, NULL);

xQueueProtocolo = xQueueCreate(32, sizeof(char) );

if (xQueueProtocolo == NULL){printf("falha em criar a fila \n");}

/* Start the scheduler. */
vTaskStartScheduler();

while(1){

}

/* Will only get here if there was insufficient memory to create the idle task. */
return 0;
}
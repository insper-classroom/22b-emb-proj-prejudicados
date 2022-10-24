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

// Botão Exit Jogo
#define BUTEXIT_PIO      PIOB
#define BUTEXIT_PIO_ID   ID_PIOB
#define BUTEXIT_IDX      3
#define BUTEXIT_IDX_MASK (1 << BUTEXIT_IDX)

// Sensor de força
#define AFEC_FORCE AFEC0
#define AFEC_FORCE_ID ID_AFEC0
#define AFEC_FORCE_CHANNEL 5 // Canal do pino PB2

// Botão On e Off 
#define BUTONOFF_PIO      PIOD
#define BUTONOFF_PIO_ID   ID_PIOD
#define BUTONOFF_IDX      25
#define BUTONOFF_IDX_MASK (1 << BUTONOFF_IDX)

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
void init_startbut(void);
void init_exitbut(void);
void init_onoffbut(void);
void init_joystickr(void);
void init_joystickl(void);
int init_hc05(void);
void joystickR_callback(void);
void joystickL_callback(void);
void startbut_callback(void);
void exitbut_callback(void);
static void AFEC_force_callback(void);
static void config_AFEC_force(Afec *afec, uint32_t afec_id, uint32_t afec_channel, afec_callback_t callback);
void send_package(char id, char eof);
char recive_package(void);
/* --- --- --- --- --- --- --- --- --- --- --- --- */
// GLOBAL VARIABLES

TimerHandle_t xTimer;

QueueHandle_t xQueueProtocolo;
QueueHandle_t xQueueForce;
SemaphoreHandle_t xSemaphoreHandshake;

typedef struct {
	uint value;
} forceData;

/* --- --- --- --- --- --- --- --- --- --- --- --- */
// HANDLERS E CALLBACKS

void startbut_callback(void){
	char id = '1';
	xQueueSendFromISR(xQueueProtocolo, &id, 0);
}

void exitbut_callback(void){
	char id = '4';
	xQueueSendFromISR(xQueueProtocolo, &id, 0);
}

void joystickR_callback(void){
	//botao foi abertado
	if(!pio_get(BUTRIGHT_PIO , PIO_INPUT, BUTRIGHT_IDX_MASK)){
		char id = '2';
		xQueueSendFromISR(xQueueProtocolo, &id, 0);
	}
	else{
		char id = '0';
		xQueueSendFromISR(xQueueProtocolo, &id, 0);
	}
}

void joystickL_callback(void){
	//botao foi abertado
	if(!pio_get(BUTLEFT_PIO , PIO_INPUT, BUTLEFT_IDX_MASK)){
		char id = '3';
		xQueueSendFromISR(xQueueProtocolo, &id, 0);
	}
	else{
		char id = '0';
		xQueueSendFromISR(xQueueProtocolo, &id, 0);
	}
}

static void AFEC_force_callback(void) {
	forceData force;
	force.value = afec_channel_get_value(AFEC_FORCE, AFEC_FORCE_CHANNEL);
	BaseType_t xHigherPriorityTaskWoken = pdTRUE;
	if(force.value >= 2000){
		char id = '5';
		xQueueSendFromISR(xQueueProtocolo, &id, 0);
	}
/*	xQueueSendFromISR(xQueueForce, &force, &xHigherPriorityTaskWoken);*/
}


void vTimerCallback(TimerHandle_t xTimer) {
	/* Selecina canal e inicializa conversão */
	afec_channel_enable(AFEC_FORCE, AFEC_FORCE_CHANNEL);
	afec_start_software_conversion(AFEC_FORCE);
}


void butonoff_callback(void){
	// libera semáforo
	xSemaphoreGiveFromISR(xSemaphoreHandshake, 0);
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
	init_startbut();
	init_onoffbut();
	init_exitbut();
	init_joystickr();
	init_joystickl();  
	config_AFEC_force(AFEC_FORCE, AFEC_FORCE_ID, AFEC_FORCE_CHANNEL, AFEC_force_callback);
	xTimer = xTimerCreate("Timer", 100, pdTRUE, (void *)0, vTimerCallback);   
	xTimerStart(xTimer, 0);

	char id = '0';
	char eof = 'X';
	forceData force;
	int run = 0;
	char response;
	
	for(;;){
		if( xSemaphoreTake(xSemaphoreHandshake, 0 / portTICK_PERIOD_MS) == pdTRUE ){
			printf("but on off \n");
			run = 0;
			while(run == 0) {
				//send handshake
				send_package('H', 'X');
				//esperando respostas
				response = recive_package();
				printf("%c", response);
			
				//tudo certo
				if(response == 'H'){
					//caso ainda o programa nao esteja rodando (queremos ligar)
					run = 1;
					break;
				}
			}	
		}
		if(run == 1){
			if( xQueueReceive(xQueueProtocolo, &id, ( TickType_t ) 0 )){
				send_package(id, eof);
			}
		}	
// 		if( xQueueReceive(xQueueForce, &force, ( TickType_t ) 0 )){
// 			printf(" force = %d \n", force);
// 		}
	}

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

void init_startbut(void){
	pmc_enable_periph_clk(BUTSTART_PIO_ID);
	pio_configure(BUTSTART_PIO, PIO_INPUT, BUTSTART_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	pio_handler_set(BUTSTART_PIO, BUTSTART_PIO_ID, BUTSTART_IDX_MASK, PIO_IT_FALL_EDGE, startbut_callback);
	pio_enable_interrupt(BUTSTART_PIO, BUTSTART_IDX_MASK);
	pio_get_interrupt_status(BUTSTART_PIO);
	NVIC_EnableIRQ(BUTSTART_PIO_ID);
	NVIC_SetPriority(BUTSTART_PIO_ID, 4);
}

void init_exitbut(void){
	pmc_enable_periph_clk(BUTEXIT_PIO_ID);
	pio_configure(BUTEXIT_PIO, PIO_INPUT, BUTEXIT_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	pio_handler_set(BUTEXIT_PIO, BUTEXIT_PIO_ID, BUTEXIT_IDX_MASK, PIO_IT_FALL_EDGE, exitbut_callback);
	pio_enable_interrupt(BUTEXIT_PIO, BUTEXIT_IDX_MASK);
	pio_get_interrupt_status(BUTEXIT_PIO);
	NVIC_EnableIRQ(BUTEXIT_PIO_ID);
	NVIC_SetPriority(BUTEXIT_PIO_ID, 4);
}

void init_onoffbut(void){
	pmc_enable_periph_clk(BUTONOFF_PIO_ID);
	pio_configure(BUTONOFF_PIO, PIO_INPUT,BUTONOFF_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	pio_handler_set(BUTONOFF_PIO, BUTONOFF_PIO_ID, BUTONOFF_IDX_MASK, PIO_IT_FALL_EDGE, butonoff_callback);
	pio_enable_interrupt(BUTONOFF_PIO, BUTONOFF_IDX_MASK);
	pio_get_interrupt_status(BUTONOFF_PIO);
	NVIC_EnableIRQ(BUTONOFF_PIO_ID);
	NVIC_SetPriority(BUTONOFF_PIO_ID, 4);
}

void init_joystickr(void){
	pmc_enable_periph_clk(BUTRIGHT_PIO_ID);
	pio_configure(BUTRIGHT_PIO, PIO_INPUT, BUTRIGHT_IDX_MASK, PIO_PULLUP| PIO_DEBOUNCE);
	pio_set_debounce_filter(BUTRIGHT_PIO, BUTRIGHT_IDX_MASK, 60);
	pio_handler_set(BUTRIGHT_PIO, BUTRIGHT_PIO_ID, BUTRIGHT_IDX_MASK, PIO_IT_EDGE, joystickR_callback);
	pio_enable_interrupt(BUTRIGHT_PIO, BUTRIGHT_IDX_MASK);
	pio_get_interrupt_status(BUTRIGHT_PIO);
	NVIC_EnableIRQ(BUTRIGHT_PIO_ID);
	NVIC_SetPriority(BUTRIGHT_PIO_ID, 4);
}

void init_joystickl(void){
	pmc_enable_periph_clk(BUTLEFT_PIO_ID);
	pio_configure(BUTLEFT_PIO, PIO_INPUT, BUTLEFT_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUTLEFT_PIO, BUTLEFT_IDX_MASK, 60);

	pio_handler_set(BUTLEFT_PIO, BUTLEFT_PIO_ID, BUTLEFT_IDX_MASK, PIO_IT_EDGE, joystickL_callback);
	pio_enable_interrupt(BUTLEFT_PIO, BUTLEFT_IDX_MASK);
	pio_get_interrupt_status(BUTLEFT_PIO);
	NVIC_EnableIRQ(BUTLEFT_PIO_ID);
	NVIC_SetPriority(BUTLEFT_PIO_ID, 4);
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


static void config_AFEC_force(Afec *afec, uint32_t afec_id, uint32_t afec_channel, afec_callback_t callback) {
  afec_enable(afec);
  struct afec_config afec_cfg;
  afec_get_config_defaults(&afec_cfg);
  afec_init(afec, &afec_cfg);
  afec_set_trigger(afec, AFEC_TRIG_SW);
  struct afec_ch_config afec_ch_cfg;
  afec_ch_get_config_defaults(&afec_ch_cfg);
  afec_ch_cfg.gain = AFEC_GAINVALUE_0;
  afec_ch_set_config(afec, afec_channel, &afec_ch_cfg);
  afec_channel_set_analog_offset(afec, afec_channel, 0x200);
  struct afec_temp_sensor_config afec_temp_sensor_cfg;
  afec_temp_sensor_get_config_defaults(&afec_temp_sensor_cfg);
  afec_temp_sensor_set_config(afec, &afec_temp_sensor_cfg);
  afec_set_callback(afec, afec_channel, callback, 1);
  NVIC_SetPriority(afec_id, 4);
  NVIC_EnableIRQ(afec_id);
}


void send_package(char id, char eof){
	while(!usart_is_tx_ready(USART_COM)){
		//vTaskDelay(10/portTICK_PERIOD_MS);
	}
	usart_write(USART_COM, id);
	
	while(!usart_is_tx_ready(USART_COM)){
		//vTaskDelay(10/portTICK_PERIOD_MS);
	}
	
	usart_write(USART_COM, eof);
}

char recive_package(void){
	char status;
	int timeout;
	while(!usart_is_rx_ready(USART_COM)){
		vTaskDelay(10/portTICK_PERIOD_MS);	
		if(timeout++ > 50) return 0;
	}
	usart_read(USART_COM, &status);
	
	return status;
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
xQueueForce = xQueueCreate(100, sizeof(forceData));
xSemaphoreHandshake = xSemaphoreCreateBinary();

if (xQueueProtocolo == NULL){printf("falha em criar a fila \n");}
if (xQueueForce == NULL){printf("falha em criar a queue xQueueForce \n");}
if (xSemaphoreHandshake == NULL){printf("falha em criar o semaforo \n");}

/* Start the scheduler. */
vTaskStartScheduler();

while(1){

}

/* Will only get here if there was insufficient memory to create the idle task. */
return 0;
}
#include "platform.h"

#include <string.h>

#include "gpio.h"
#include "fifo.h"

#include "bsp_uart.h"
#include "uart.h"

#define UART_BUFFER 100
struct fifo_t uart_fifo;
uint8_t uart_buf[UART_BUFFER];

uart_cb_t uart_cb = NULL;

// dma stuff
volatile uint8_t uart_sending = 0;
volatile uint32_t uart_sending_size;

void uart_set_cb(uart_cb_t cb)
{
	uart_cb = cb;
}

static void uart_hw_init(uint32_t speed)
{
	USART_InitTypeDef USART_InitStructure;

	USART_Cmd(USART1, DISABLE);

	USART_InitStructure.USART_BaudRate = speed;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
}

void uart_init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure = {
		.NVIC_IRQChannelPreemptionPriority = 1,
		.NVIC_IRQChannelSubPriority = 0,
		.NVIC_IRQChannelCmd = ENABLE,
	};

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	GPIO_InitBatch(uart_gpio, uart_gpio_cnt);

	/* DMA_Channel (triggered by USART Tx event) */
	DMA_DeInit(DMA1_Channel4);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel4, &DMA_InitStructure);

	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);

	uart_hw_init(9600);

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel4_IRQn;
	NVIC_Init(&NVIC_InitStructure);

	FIFO_Init(&uart_fifo, uart_buf, sizeof(uint8_t), UART_BUFFER);

	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

	USART_Cmd(USART1, ENABLE);
}

void uart_disable_int(void)
{
	NVIC_DisableIRQ(USART1_IRQn);
}

void uart_enable_int(void)
{
	NVIC_EnableIRQ(USART1_IRQn);
}

/*
 * This function will enqueue buffer into the outgoing fifo. If fifo
 * overruns, data is dropped. (in this case use faster uart speed!)
 */
void uart_send(const char *buf, uint32_t len)
{
	uint32_t wl = MIN(len, FIFO_GetFreeSpace(&uart_fifo));
	FIFO_WriteMore(&uart_fifo, buf, wl);

	NVIC_DisableIRQ(DMA1_Channel4_IRQn);

	if (UNLIKELY(uart_sending == 0)) {
		uart_sending = 1;
		uart_sending_size = (uint32_t)FIFO_GetReadSizeCont(&uart_fifo);

		DMA1_Channel4->CMAR = (uint32_t)FIFO_GetReadAddr(&uart_fifo);
		DMA1_Channel4->CNDTR = uart_sending_size;
		DMA_Cmd(DMA1_Channel4, ENABLE);
	}

	NVIC_EnableIRQ(DMA1_Channel4_IRQn);
}

void DMA1_Channel4_IRQHandler()
{
	DMA_Cmd(DMA1_Channel4, DISABLE);
	DMA_ClearITPendingBit(DMA1_IT_TC4);

	FIFO_ReadDoneSize(&uart_fifo, uart_sending_size);

	if (UNLIKELY(FIFO_EMPTY(&uart_fifo))) {
		uart_sending = 0;
		uart_sending_size = 0;
		return;
	}

	uart_sending_size = (uint32_t)FIFO_GetReadSizeCont(&uart_fifo);
	DMA1_Channel4->CMAR = (uint32_t)FIFO_GetReadAddr(&uart_fifo);
	DMA1_Channel4->CNDTR = uart_sending_size;
	DMA_Cmd(DMA1_Channel4, ENABLE);
}

void USART1_IRQHandler()
{
	char ch;

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		/* Read one byte from the receive data register */
		ch = USART1->DR & 0xff;
		if (uart_cb) {
			uart_cb(ch);
		}
	}
}


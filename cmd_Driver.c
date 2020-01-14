/*
 * Uart_Driver.c
 *
 *  Created on: 17 Oct 2019
 *      Author: Diego B.
 */

/* Private includes ----------------------------------------------------------*/
#include "main.h"
#include "cmd_task.h"


/* Private defines -----------------------------------------------------------*/
#define 	UART_RX_BUF_LEN	100

/* Private function prototypes -----------------------------------------------*/


/* Exported variables --------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_usart1_rx;

extern CMD_HandleTypeDef uartCommsHandle;		/* Handle for the control command	*/
/* Private variables ---------------------------------------------------------*/
uint8_t UartRxBuffer[UART_RX_BUF_LEN];
uint8_t  Uart_DMA_Ovrn = 0;
uint16_t Uart_Rx_oPtr  = 0;

/* Private user code ---------------------------------------------------------*/

void CMD_Init(CMD_HandleTypeDef *cmd){

	if(cmd == &uartCommsHandle){
		HAL_UART_Receive_DMA(&huart1, UartRxBuffer, UART_RX_BUF_LEN);
		cmd->echo = 1;
	}
}

uint8_t CMD_isDataAvailable(CMD_HandleTypeDef *cmd) {

	if(cmd == &uartCommsHandle){
		const uint16_t Uart_Rx_iPtr = (UART_RX_BUF_LEN - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx));

		if (((Uart_Rx_oPtr < Uart_Rx_iPtr) && !Uart_DMA_Ovrn) || ((Uart_Rx_oPtr > Uart_Rx_iPtr) && Uart_DMA_Ovrn)) {
			//Normal or overflow
			return 1;
		} else if (Uart_DMA_Ovrn){
			//Reset
			Uart_Rx_oPtr = Uart_Rx_iPtr;
			Uart_DMA_Ovrn = 0;
			return 0;
		}
		return 0;
	}

	return 0;
}


uint8_t CMD_Get_Char(CMD_HandleTypeDef *cmd) {

	uint8_t ch = 0;

	if(cmd == &uartCommsHandle){
		if (CMD_isDataAvailable(cmd)) {
			ch = UartRxBuffer[Uart_Rx_oPtr++];
			if (Uart_Rx_oPtr >= UART_RX_BUF_LEN) {
				Uart_Rx_oPtr = 0;
				Uart_DMA_Ovrn = 0;
			}
		}
	}

	return ch;
}

void CMD_Echo_Tx(CMD_HandleTypeDef *cmd, uint8_t* pData, uint32_t Size){

	if(cmd == &uartCommsHandle){
		HAL_UART_Transmit(&huart1, pData, Size, 0xff);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	Uart_DMA_Ovrn = 1;
}


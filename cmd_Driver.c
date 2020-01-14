 /******************************************************************************
 * @file           : Uart_Driver.c
 * @version        : v1.0
 * @brief          : Command process interface.
 * @author		   : Diego B.	Oct-18
 ******************************************************************************/
/*
 * Copyright (c) 2018 Diego Bailos
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of CMD Lib.
 *
 *  https://github.com/litanx/CMD/
 *
 * Author:          Diego Bailos <diegobailos@gmail.com>
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


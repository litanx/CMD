 /******************************************************************************
 * @file           : Uart_Driver.c
 * @version        : v1.0
 * @brief          : Command process interface.
 * @author		   : Diego B.	Oct-18
 ******************************************************************************
  Revisions can be found here:
  https://github.com/litanx/CMD/

  Copyright (C) diegobailos , <diegobailos@gmail.com>

                                GPLv3 License

  This program is free software: you can redistribute it and/or modify it under
  the terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or (at your option) any later
  version.

  This program is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
  PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with
  this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************************
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


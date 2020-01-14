/**
 ******************************************************************************
 * @file           : cmd_task.c
 * @version        : v1.0
 * @brief          : Command process interface.
 * @author		   : Diego B.	Oct-18
 ******************************************************************************

 ==============================================================================
 ##### How to use this driver #####
 ==============================================================================
 *
 * The user has to initialize the communications peripheral (Uart, I2C, Spi, USB,...)
 * to keep listening in continuous mode either by interrupt or DMA and put the data
 * into a FIFO buffer.
 *
 * In cmd_Driver.c write the following functions:
 *
 * void CMD_Init(CMD_HandleTypeDef *cmd) {}
 * 		Initialization of the peripheral.
 *
 * uint8_t CMD_isDataAvailable(CMD_HandleTypeDef *cmd) {}
 * 		Returns 1 if there is data available to be read from the Comms port.
 *
 * uint8_t CMD_Get_Char(CMD_HandleTypeDef *cmd) {}
 * 		Returns the character read from the Comms port
 *
 * void CMD_Echo_Tx(CMD_HandleTypeDef *cmd, uint8_t* pData, uint32_t Size) {}
 * 		The driver will write the echo messages though this function.
 *
 * void CMD_process(CMD_HandleTypeDef *cmd) {}
 * 		This is the callback function to process the commands stored in the
 * 		CMD_HandleTypeDef structure.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "cmd_task.h"

/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/
static void CMD_Echo_Task(CMD_HandleTypeDef *cmd, uint8_t tmp);


/******************************************************************************/
//                                                          status: not tested
void CMD_handle(CMD_HandleTypeDef *cmd) {
	uint8_t i;
	uint8_t j;
	uint8_t tmp;

	while (CMD_isDataAvailable(cmd)) {

		tmp = CMD_Get_Char(cmd);

		if (tmp == '\r' || tmp == '\n') {

			if (cmd->echo) {

				CMD_Echo_Task(cmd, tmp);

				for (i = (CMD_HIST - 1); i > 1; i--) {
					for (j = 0; j < cmd->Hst_cmd_len[i - 1]; j++)
						cmd->Hst_cmd[i][j] = cmd->Hst_cmd[i - 1][j];
					cmd->Hst_cmd[i][j] = '\0';
					cmd->Hst_cmd_len[i] = cmd->Hst_cmd_len[i - 1];
				}
				for (i = 0; i < cmd->Rx_cmd_len; i++)
					cmd->Hst_cmd[1][i] = cmd->Rx_cmd[i];
				cmd->Hst_cmd[1][i] = '\0';
				cmd->Hst_cmd_len[1] = cmd->Rx_cmd_len;
			}

			cmd->Rx_cmd[cmd->Rx_cmd_len] = '\0';

			CMD_process(cmd);

			cmd->Rx_cmd_ptr = 0;
			cmd->Hst_cmd_cursor = 0;
			cmd->Rx_cmd_len = 0;

		}else{

			if (cmd->echo) {
				CMD_Echo_Task(cmd, tmp);
			} else {
				cmd->Rx_cmd[cmd->Rx_cmd_ptr] = tmp;
				if (cmd->Rx_cmd_ptr < CMD_BUF_MAX_LEN)
					cmd->Rx_cmd_ptr++;
				if (cmd->Rx_cmd_len < 255)
					cmd->Rx_cmd_len++;
			}
		}
	}
}

/******************************************************************************/
//                                                          status: not tested
static void CMD_Echo_Task(CMD_HandleTypeDef *cmd, uint8_t tmp) {

	uint8_t i;
	static uint8_t prevCh    = 0;
	static uint8_t preprevCh = 0;

	if(tmp == '\e' || prevCh == '\e'){
		preprevCh = prevCh;
		prevCh = tmp;
		return;
	}

	if (tmp == '\b') {
		if (cmd->Rx_cmd_ptr && cmd->Rx_cmd_ptr == cmd->Rx_cmd_len) {
			cmd->Rx_cmd_ptr--;
			cmd->Rx_cmd_len--;
			CMD_Echo_Tx(cmd, (uint8_t*) "\b \b", 3);
		} else if (cmd->Rx_cmd_ptr) {
			for (i = cmd->Rx_cmd_ptr; i < (cmd->Rx_cmd_len); i++) {
				cmd->Rx_cmd[i - 1] = cmd->Rx_cmd[i];
			}
			cmd->Rx_cmd_ptr--;
			cmd->Rx_cmd_len--;

			CMD_Echo_Tx(cmd, (uint8_t*) "\b", 1);
			CMD_Echo_Tx(cmd, (uint8_t*) &cmd->Rx_cmd + cmd->Rx_cmd_ptr,
					cmd->Rx_cmd_len - cmd->Rx_cmd_ptr);
			CMD_Echo_Tx(cmd, (uint8_t*) " ", 1);
			for (i = 0; i <= (cmd->Rx_cmd_len - cmd->Rx_cmd_ptr); i++)
				CMD_Echo_Tx(cmd, (uint8_t*) "\b", 1);
		}

	} else if ( (preprevCh == '\e') && (prevCh == '[') ) {

		if (tmp == 'A' && cmd->Hst_cmd_cursor < CMD_HIST - 1) {
			if (!cmd->Hst_cmd_cursor) {
				for (i = 0; i < cmd->Rx_cmd_len; i++) {
					cmd->Hst_cmd[0][i] = cmd->Rx_cmd[i];
				}
				cmd->Rx_cmd[i] = '\0';
				cmd->Hst_cmd_len[cmd->Hst_cmd_cursor] = cmd->Rx_cmd_len;
			}
			cmd->Hst_cmd_cursor++;

			for (i = 0; i < cmd->Rx_cmd_len; i++)
				CMD_Echo_Tx(cmd, (uint8_t*) "\b \b", 3);
			for (i = 0; i < cmd->Hst_cmd_len[cmd->Hst_cmd_cursor]; i++)
				cmd->Rx_cmd[i] = cmd->Hst_cmd[cmd->Hst_cmd_cursor][i];
			cmd->Rx_cmd[i] = '\0';
			cmd->Rx_cmd_len = cmd->Hst_cmd_len[cmd->Hst_cmd_cursor];
			cmd->Rx_cmd_ptr = cmd->Rx_cmd_len;

			CMD_Echo_Tx(cmd, (uint8_t*) &cmd->Rx_cmd, cmd->Rx_cmd_len);

		} else if (tmp == 'B' && cmd->Hst_cmd_cursor) {
			cmd->Hst_cmd_cursor--;

			for (i = 0; i < cmd->Rx_cmd_len; i++)
				CMD_Echo_Tx(cmd, (uint8_t*) "\b \b", 3);

			for (i = 0; i < cmd->Hst_cmd_len[cmd->Hst_cmd_cursor]; i++) {
				cmd->Rx_cmd[i] = cmd->Hst_cmd[cmd->Hst_cmd_cursor][i];
			}
			cmd->Rx_cmd[i] = '\0';
			cmd->Rx_cmd_len = cmd->Hst_cmd_len[cmd->Hst_cmd_cursor];
			cmd->Rx_cmd_ptr = cmd->Rx_cmd_len;

			CMD_Echo_Tx(cmd, (uint8_t*) &cmd->Rx_cmd, cmd->Rx_cmd_len);

		} else if (tmp == 'C' && cmd->Rx_cmd_ptr < cmd->Rx_cmd_len) {
			CMD_Echo_Tx(cmd, (uint8_t*) "\e[C", 3);
			cmd->Rx_cmd_ptr++;

		} else if (tmp == 'D' && cmd->Rx_cmd_ptr) {
			CMD_Echo_Tx(cmd, (uint8_t*) "\e[D", 3);
			cmd->Rx_cmd_ptr--;

		} else if (tmp == '3') {
			tmp = CMD_Get_Char(cmd);

			if (tmp == '~' && cmd->Rx_cmd_ptr < cmd->Rx_cmd_len) {
				for (i = cmd->Rx_cmd_ptr; i < (cmd->Rx_cmd_len); i++)
					cmd->Rx_cmd[i] = cmd->Rx_cmd[i + 1];
				cmd->Rx_cmd_len--;
				CMD_Echo_Tx(cmd, (uint8_t*) &cmd->Rx_cmd + cmd->Rx_cmd_ptr,
						cmd->Rx_cmd_len - cmd->Rx_cmd_ptr);
				CMD_Echo_Tx(cmd, (uint8_t*) " ", 1);
				for (i = 0; i <= (cmd->Rx_cmd_len - cmd->Rx_cmd_ptr); i++)
					CMD_Echo_Tx(cmd, (uint8_t*) "\b", 1);
			}
		}

	} else if (tmp == '\r') {
		CMD_Echo_Tx(cmd, (uint8_t*) "\r\n", 2);

	}else if (cmd->Rx_cmd_ptr < CMD_BUF_MAX_LEN) {
		if (cmd->Rx_cmd_ptr == cmd->Rx_cmd_len) {
			cmd->Rx_cmd[cmd->Rx_cmd_ptr] = tmp;
			cmd->Rx_cmd_ptr++;
			cmd->Rx_cmd_len++;
			CMD_Echo_Tx(cmd, &tmp, 1);
		} else if (cmd->Rx_cmd_len < CMD_BUF_MAX_LEN) {
			for (i = cmd->Rx_cmd_len; i > (cmd->Rx_cmd_ptr); i--)
				cmd->Rx_cmd[i] = cmd->Rx_cmd[i - 1];
			cmd->Rx_cmd[cmd->Rx_cmd_ptr] = tmp;
			cmd->Rx_cmd_len++;
			CMD_Echo_Tx(cmd, (uint8_t*) &cmd->Rx_cmd + cmd->Rx_cmd_ptr,
					cmd->Rx_cmd_len - cmd->Rx_cmd_ptr);
			cmd->Rx_cmd_ptr++;
			for (i = 0; i < (cmd->Rx_cmd_len - cmd->Rx_cmd_ptr); i++)
				CMD_Echo_Tx(cmd, (uint8_t*) "\b", 1);
		}

	}

	preprevCh = prevCh;
	prevCh = tmp;
}


/******************************************************************************/
/* Match command with the given string.
 * */

uint8_t CMD_match(CMD_HandleTypeDef *cmd, char* match) {

	cmd->Rx_cmd_ptr = 0;

	while (*match != '\0') {
		if (*match++ != cmd->Rx_cmd[cmd->Rx_cmd_ptr++])
			return (0);
	}
	if (cmd->Rx_cmd[cmd->Rx_cmd_ptr] >= 'a'
			&& cmd->Rx_cmd[cmd->Rx_cmd_ptr] <= 'z')
		return 0;
	if (cmd->Rx_cmd[cmd->Rx_cmd_ptr] >= 'A'
			&& cmd->Rx_cmd[cmd->Rx_cmd_ptr] <= 'Z')
		return 0;
	if (cmd->Rx_cmd[cmd->Rx_cmd_ptr] >= '0'
			&& cmd->Rx_cmd[cmd->Rx_cmd_ptr] <= '9')
		return 0;

	return (1);
}


/******************************************************************************/
/* Get next char from the incomming commands.
 * */
uint8_t CMD_Read_Char(CMD_HandleTypeDef *cmd) {

	return cmd->Rx_cmd[cmd->Rx_cmd_ptr++];
}

/******************************************************************************/
/* Get next integer value from the incomming commands.
 * */
uint8_t get_next_value_d(CMD_HandleTypeDef *cmd, uint32_t *value){

	const uint8_t separator = ',';

	if(!isNumber(cmd->Rx_cmd[cmd->Rx_cmd_ptr])) 	return 0;

	*value = atoi((char*)&cmd->Rx_cmd[cmd->Rx_cmd_ptr]);

	while( (cmd->Rx_cmd[cmd->Rx_cmd_ptr] != separator) && (cmd->Rx_cmd[cmd->Rx_cmd_ptr] != '\r') && (cmd->Rx_cmd[cmd->Rx_cmd_ptr]) ){

		cmd->Rx_cmd_ptr++;

	}

	cmd->Rx_cmd_ptr++;

	return 1;
}


/******************************************************************************/
/* Get next float value from the incomming commands.
 * */
uint8_t get_next_value_f(CMD_HandleTypeDef *cmd, float *value){

	const uint8_t separator = ',';

	if(!isNumber(cmd->Rx_cmd[cmd->Rx_cmd_ptr])) 	return 0;

	*value = atof((char*)&cmd->Rx_cmd[cmd->Rx_cmd_ptr]);

	while( (cmd->Rx_cmd[cmd->Rx_cmd_ptr] != separator) && (cmd->Rx_cmd[cmd->Rx_cmd_ptr] != '\r') && (cmd->Rx_cmd[cmd->Rx_cmd_ptr]) ){

		cmd->Rx_cmd_ptr++;

	}

	cmd->Rx_cmd_ptr++;

	return 1;
}

/******************************************************************************/
//                                                          status: not tested
__weak void CMD_Init(CMD_HandleTypeDef *cmd){

}
/******************************************************************************/
//                                                          status: not tested
__weak uint8_t CMD_isDataAvailable(CMD_HandleTypeDef *cmd) {
	return 0;
}

/******************************************************************************/
//                                                          status: not tested
__weak uint8_t CMD_Get_Char(CMD_HandleTypeDef *cmd) {
	return 0;
}

/******************************************************************************/
//                                                          status: not tested
__weak void CMD_Echo_Tx(CMD_HandleTypeDef *cmd, uint8_t* pData, uint32_t Size) {

}

/******************************************************************************/
//                                                          status: not tested
__weak void CMD_process(CMD_HandleTypeDef *cmd) {

}

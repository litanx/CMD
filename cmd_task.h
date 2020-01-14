/**
  ******************************************************************************
  * @file           : cmd_task.h
  * @version        : v1.0
  * @brief          : Header for cmd_task.c file.
  * @author			: Diego B.	Oct-18
  ******************************************************************************
  */

/* Define to prevent recursive inclusion --------------------------------------*/
#ifndef __CMD_TASK_H__
#define __CMD_TASK_H__

#ifdef __cplusplus
 extern "C" {
#endif

 /* Includes -------------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>

 #define CMD_HIST			10		/* Define number of  previous commands 		*/
 #define CMD_BUF_MAX_LEN 	256		// Define max length of commands			*/

 /* Exported types -------------------------------------------------------------*/

 typedef struct
 {

	 uint8_t 	Rx_cmd[CMD_BUF_MAX_LEN];				/*!< CMD buffer	     				*/

	 uint8_t 	Rx_cmd_ptr;								/*!< CMD Pointer     				*/

	 uint8_t	Rx_cmd_len;								/*!< Command Length					*/

	 uint8_t 	Hst_cmd[CMD_HIST][CMD_BUF_MAX_LEN];		/*!< Historic CMD buffer			*/

	 uint8_t	Hst_cmd_len[CMD_HIST];					/*!< Historic Commands Length		*/

	 uint8_t	Hst_cmd_cursor;							/*!< Historic Command Cursor		*/

	 uint8_t	echo;									/*!< Echo Flag			 			*/

 }CMD_HandleTypeDef;


 /* Exported constants --------------------------------------------------------*/

 /* Exported macro ------------------------------------------------------------*/
#define isNumber(__VALUE__)			( ((__VALUE__) >= 0x30) && ((__VALUE__) <= 0x39) )

 #define ESCAPE_SEQ          "\033"
 #define CURSOR_UP_LEFT      "\033[H"
 #define HIDE_CURSOR         "\033[?25l"
 #define SHOW_CURSOR         "\033[?25h"
 #define CLR_SCREEN          "\033[2J"
 #define CLR_SCREEN_CRSR_UP  "\033[1J"
 #define CLR_SCREEN_CRSR_DN  "\033[0J"
 #define CLR_LINE_CRSR_R     "\033[0K"
 #define CLR_LINE_CRSR_L     "\033[1K"
 #define CLR_LINE            "\033[2K"

 #define ATTRIBUTES_RST      "\033[0m"
 #define BOLD_FONT           "\033[1m"
 #define RED_FONT            "\033[31m"
 #define GREEN_FONT          "\033[32m"
 #define YELLOW_FONT         "\033[33m"
 #define BLUE_FONT           "\033[34m"
 #define MAGENTA_FONT        "\033[35m"
 #define CYAN_FONT           "\033[36m"
 #define WHITE_FONT          "\033[37m"
 #define UNDERLINE_MODE      "\033[4m"

 #define BACKSPACE           "\b"
 #define HORIZONTAL_TAB      "\t"
 #define NEWLINE             "\n"
 #define VERTICAL_TAB        "\v"
 #define NEW_PAGE            "\f"
 #define CARRIAGE_RETURN     "\r"

 /* Exported functions ------------------------------------------------------- */

 /* Main functions */
 void CMD_Init(CMD_HandleTypeDef *cmd);
 void CMD_handle(CMD_HandleTypeDef *cmd);

 /* Process incoming commands */
 uint8_t CMD_match(CMD_HandleTypeDef *cmd, char* match);
 uint8_t CMD_Read_Char(CMD_HandleTypeDef *cmd);
 uint8_t get_next_value_d(CMD_HandleTypeDef *cmd, uint32_t *value);
 uint8_t get_next_value_f(CMD_HandleTypeDef *cmd, float *value);

 void CMD_process(CMD_HandleTypeDef *cmd);

 uint8_t CMD_isDataAvailable(CMD_HandleTypeDef *cmd);
 uint8_t CMD_Get_Char(CMD_HandleTypeDef *cmd);
 void CMD_Echo_Tx(CMD_HandleTypeDef *cmd, uint8_t* pData, uint32_t Size);



#ifdef __cplusplus
}
#endif

#endif /* __CMD_TASK_H__ */

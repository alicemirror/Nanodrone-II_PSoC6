/*
 * uart_task.h
 *
 *  Created on: 13 mrt. 2021
 *      Author: jancu
 */

#ifndef UART_TASK_H_
#define UART_TASK_H_

#include "uart_config.h"

#include "FreeRTOS.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* Task parameters for UART Task. */
#define UART_TASK_PRIORITY       (2)
#define UART_TASK_STACK_SIZE     (1024 * 3)



/* dependent on the BAUD used */

// jc 20210313: 9600 baud needs a 16 bit divider
/* UART desired baud rate is 9600 bps
 * The UART baud rate = (clk_scb / Oversample).
 * For clk_peri = 100 MHz, select divider value 864 and get SCB clock = (100 MHz / 864) = 115.7 kHz.
 * Select Oversample = 12. These setting results UART data rate = 115.7 kHz / 12 = 9645 bps.
 */

#define UART_CLK_DIV_TYPE     (CY_SYSCLK_DIV_16_BIT)
#define UART_CLK_DIV_NUMBER   (UART_DIVIDER_NUMBER)
#define UART_OVERSAMPLE 12UL
#define UART_DIVISION 863UL


/* application dependent UART settings */
#define UART_BUFFER_SIZE 16



/*******************************************************************************
* Function Prototypes
********************************************************************************/
void uart_task(void *pvParameters);

void initUART();
void UART_Isr();


#endif /* UART_TASK_H_ */

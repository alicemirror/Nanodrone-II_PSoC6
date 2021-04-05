/*
 * uart_config.h
 *
 *  Created on: 20 mrt. 2021
 *      Author: jancu
 */

#ifndef CONFIGS_UART_CONFIG_H_
#define CONFIGS_UART_CONFIG_H_

/* dependent on the SCB and pins used */
/* Assign UART interrupt number and priority */



// SCB 1 p10_0 rx p10_1 tx

#define UART_SCB SCB1
#define UART_PORT       P10_0_PORT
#define UART_RX_NUM     P10_0_NUM
#define UART_TX_NUM     P10_1_NUM
#define UART_RX_PIN     P10_0_SCB1_UART_RX
#define UART_TX_PIN     P10_1_SCB1_UART_TX
#define UART_DIVIDER_NUMBER 1U

#define UART_CLOCK PCLK_SCB1_CLOCK

#define UART_INTR_NUM        ((IRQn_Type) scb_1_interrupt_IRQn)



// SCB 5 p5_0 rx p5_1 tx

//#define UART_SCB SCB5
//#define UART_PORT       P5_0_PORT
//#define UART_RX_NUM     P5_0_NUM
//#define UART_TX_NUM     P5_1_NUM
//#define UART_RX_PIN     P5_0_SCB5_UART_RX
//#define UART_TX_PIN     P5_1_SCB5_UART_TX
//#define UART_DIVIDER_NUMBER 5U
//
//#define UART_CLOCK PCLK_SCB5_CLOCK
//
//#define UART_INTR_NUM        ((IRQn_Type) scb_5_interrupt_IRQn)



#define UART_INTR_PRIORITY   (7U)


#endif /* CONFIGS_UART_CONFIG_H_ */

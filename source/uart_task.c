/*
 * uart_task.c
 *
 *  Created on: 13 mrt. 2021
 *      Author: jancu
 */

#include "cy_pdl.h"
#include "cyhal.h"

/* FreeRTOS header files */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Task header files */
#include "uart_task.h"

#include "telemetry_queue.h"

#include "cybsp.h"  // for the user led only

/* Allocate context for UART operation */
cy_stc_scb_uart_context_t uartContext;
/* Stores the handle of the task that will be notified when the
transmission is complete. */
volatile TaskHandle_t xTaskToNotify_UART;

/* The index within the target task's array of task notifications
to use. */
const UBaseType_t xArrayIndex = 1;


uint8_t rxBuffer[UART_BUFFER_SIZE];

/* Allocate context for UART operation */
cy_stc_scb_uart_context_t uartContext;

void UART_receive();


extern QueueHandle_t telemetry_queue;


/******************************************************************************
 * Function Name: uart_task
 ******************************************************************************
 * Summary:
 *  Task for handling initialization & connection of UART.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void uart_task(void *pvParameters) {

  /* To avoid compiler warnings */
  (void)pvParameters;
  uint32_t ulNotificationValue;
  xTaskToNotify_UART = NULL;

  while (true) {

    /* Start the receiving from UART. */
    UART_receive();
    /* Wait to be notified that the receive is complete.  Note
        the first parameter is pdTRUE, which has the effect of clearing
        the task's notification value back to 0, making the notification
        value act like a binary (rather than a counting) semaphore.  */
    ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY  );


    if( ulNotificationValue == 1 )
    {
      /* Blocking wait until buffer is full */
      // in this example, it will never handle because the UART interrupt fires exaxtly when the buffer is full
      while (0UL != (CY_SCB_UART_RECEIVE_ACTIVE & Cy_SCB_UART_GetReceiveStatus(UART_SCB, &uartContext))) {}
      /* Handle received data */

      if(telemetry_queue) { // queue needs to be generated}
        if( xQueueSend( telemetry_queue,
                       ( void * ) rxBuffer,
                       ( TickType_t ) 10 ) != pdPASS )
        {
            /* Failed to post the message, even after 10 ticks. */
          CY_ASSERT_L3(!0);
        }
      }

    }
  }
}

void initUART() {
  // https://cypresssemiconductorco.github.io/psoc6pdl/pdl_api_reference_manual/html/group__group__scb__uart.html


  /* Populate configuration structure */
  const cy_stc_scb_uart_config_t uartConfig = {
      .uartMode                   = CY_SCB_UART_STANDARD,
      .enableMutliProcessorMode   = false,
      .smartCardRetryOnNack       = false,
      .irdaInvertRx               = false,
      .irdaEnableLowPowerReceiver = false,
      .oversample                 = UART_OVERSAMPLE,
      .enableMsbFirst             = false,
      .dataWidth                  = 8UL,
      .parity                     = CY_SCB_UART_PARITY_NONE,
      .stopBits                   = CY_SCB_UART_STOP_BITS_1,
      .enableInputFilter          = false,
      .breakWidth                 = 11UL,
      .dropOnFrameError           = false,
      .dropOnParityError          = false,
      .receiverAddress            = 0UL,
      .receiverAddressMask        = 0UL,
      .acceptAddrInFifo           = false,
      .enableCts                  = false,
      .ctsPolarity                = CY_SCB_UART_ACTIVE_LOW,
      .rtsRxFifoLevel             = 0UL,
      .rtsPolarity                = CY_SCB_UART_ACTIVE_LOW,
      .rxFifoTriggerLevel  = 0UL,
      .rxFifoIntEnableMask = 0UL,
      .txFifoTriggerLevel  = 0UL,
      .txFifoIntEnableMask = 0UL,
  };
  /* Configure UART to operate */
  (void) Cy_SCB_UART_Init(UART_SCB, &uartConfig, &uartContext);

  /* Assign pins for UART on SCBx */

  /* Connect SCB UART function to pins */
  Cy_GPIO_SetHSIOM(UART_PORT, UART_RX_NUM, UART_RX_PIN);
  Cy_GPIO_SetHSIOM(UART_PORT, UART_TX_NUM, UART_TX_PIN);
  /* Configure pins for UART operation */
  // jc: HIGHZ is correct for UART. But in the design here, the input is "patch-wired" to another controller.
  // Cy_GPIO_SetDrivemode(UART_PORT, UART_RX_NUM, CY_GPIO_DM_HIGHZ);
  Cy_GPIO_SetDrivemode(UART_PORT, UART_RX_NUM, CY_GPIO_DM_PULLUP);
  Cy_GPIO_SetDrivemode(UART_PORT, UART_TX_NUM, CY_GPIO_DM_STRONG_IN_OFF);

  /* Assign divider type and number for UART */

  /* Connect assigned divider to be a clock source for UART */
  Cy_SysClk_PeriphAssignDivider(UART_CLOCK, UART_CLK_DIV_TYPE, UART_CLK_DIV_NUMBER);


  // set baud
  Cy_SysClk_PeriphSetDivider   (UART_CLK_DIV_TYPE, UART_CLK_DIV_NUMBER, UART_DIVISION);
  Cy_SysClk_PeriphEnableDivider(UART_CLK_DIV_TYPE, UART_CLK_DIV_NUMBER);

  /* Populate configuration structure (code specific for CM4) */
  cy_stc_sysint_t uartIntrConfig =
  {
      .intrSrc      = UART_INTR_NUM,
      .intrPriority = UART_INTR_PRIORITY,
  };
  /* Hook interrupt service routine and enable interrupt */
  (void) Cy_SysInt_Init(&uartIntrConfig, &UART_Isr);
  NVIC_EnableIRQ(UART_INTR_NUM);

  /* Enable UART to operate */
  Cy_SCB_UART_Enable(UART_SCB);
}

// UART interrupt handler
void UART_Isr() {
	Cy_SCB_UART_Interrupt(UART_SCB, &uartContext);

//	if (uartContext.rxStatus != 32) { // not a complete receive
//		return;
//	}


	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if( xTaskToNotify_UART != NULL ) {

		/* Notify the task that the receive is complete. */
		vTaskNotifyGiveFromISR( xTaskToNotify_UART, &xHigherPriorityTaskWoken );
		/* There are no receive in progress, so no tasks to notify. */
		xTaskToNotify_UART = NULL;

		/* If xHigherPriorityTaskWoken is now set to pdTRUE then a
  context switch should be performed to ensure the interrupt
  returns directly to the highest priority task.  The macro used
  for this purpose is dependent on the port in use and may be
  called portEND_SWITCHING_ISR(). */
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

// UART activate a receive with interrupt. Wait for ever for UART_BUFFER_SIZE bytes
void UART_receive() {
  /* At this point xTaskToNotify should be NULL as no receive
  is in progress.  A mutex can be used to guard access to the
  peripheral if necessary. */
  configASSERT( xTaskToNotify_UART == NULL );

  /* Store the handle of the calling task. */
  xTaskToNotify_UART = xTaskGetCurrentTaskHandle();

  /* Start receive operation (do not check status) */
  (void) Cy_SCB_UART_Receive(UART_SCB, rxBuffer, sizeof(rxBuffer), &uartContext);
}


/*
 * telemetry_receive_task.c
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

#include "telemetry_receive_task.h"
#include "telemetry_queue.h"

#include "cybsp.h"  // for the user led only

extern QueueHandle_t telemetry_queue;

uint8_t message[TELEMETRY_MESSAGE_SIZE];

/******************************************************************************
 * Function Name: telemetry_receive_task
 ******************************************************************************
 * Summary:
 *  Task for handling the telemetry queue.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void telemetry_receive_task(void *pvParameters) {
  for( ;; ) {
    if(xQueueReceive(telemetry_queue, message, portMAX_DELAY ))     {
      cyhal_gpio_toggle(CYBSP_USER_LED);
//      // process the telemetry data.
//      uint32_t checksum = 0;
//      for (int i = 0; i < sizeof(message); i++) {
//          checksum += message[i];
//      }
////      CY_ASSERT_L3((checksum > 0U));
    }

  } // end for
}

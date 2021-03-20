/*
 * telemetry_receive_task.h
 *
 *  Created on: 13 mrt. 2021
 *      Author: jancu
 */

#ifndef TELEMETRY_RECEIVE_TASK_H_
#define TELEMETRY_RECEIVE_TASK_H_

#include "FreeRTOS.h"

/*******************************************************************************
* Macros
********************************************************************************/
/* Task parameters for MQTT Client Task. */
#define TELEMETRY_RECEIVE_TASK_PRIORITY       (2)
#define TELEMETRY_RECEIVE_TASK_STACK_SIZE     (1024 * 2)




/*******************************************************************************
* Function Prototypes
********************************************************************************/
void telemetry_receive_task(void *pvParameters);

#endif /* TELEMETRY_RECEIVE_TASK_H_ */

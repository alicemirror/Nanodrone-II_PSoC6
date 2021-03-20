/*
 * telemetry_queue.c
 *
 *  Created on: 13 mrt. 2021
 *      Author: jancu
 */

#include "telemetry_queue.h"


#include "FreeRTOS.h"
#include "queue.h"

QueueHandle_t telemetry_queue = NULL;


void initTelemetryQueue() {
  telemetry_queue =  xQueueCreate( 10, TELEMETRY_MESSAGE_SIZE );
}

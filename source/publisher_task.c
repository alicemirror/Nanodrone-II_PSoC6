/******************************************************************************
 * File Name:   publisher_task.c
 *
 * Description: This file contains the task that initializes the user button
 *              GPIO, configures the ISR, and publishes MQTT messages on the
 *              topic 'MQTT_TOPIC_NANODRONE' to control a device that is actuated by the
 *              subscriber task. The file also contains the ISR that notifies
 *              the publisher task about the new device state to be published.
 *
 * Related Document: See README.md
 *
 *******************************************************************************
 * (c) 2020-2021, Cypress Semiconductor Corporation. All rights reserved.
 *******************************************************************************
 * This software, including source code, documentation and related materials
 * ("Software"), is owned by Cypress Semiconductor Corporation or one of its
 * subsidiaries ("Cypress") and is protected by and subject to worldwide patent
 * protection (United States and foreign), United States copyright laws and
 * international treaty provisions. Therefore, you may use this Software only
 * as provided in the license agreement accompanying the software package from
 * which you obtained this Software ("EULA").
 *
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software source
 * code solely for use in connection with Cypress's integrated circuit products.
 * Any reproduction, modification, translation, compilation, or representation
 * of this Software except as specified above is prohibited without the express
 * written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer of such
 * system or application assumes all risk of such use and in doing so agrees to
 * indemnify Cypress against all liability.
 *******************************************************************************/

#include "cyhal.h"
#include "cybsp.h"
#include "FreeRTOS.h"

/* Task header files */
#include "publisher_task.h"
#include "mqtt_task.h"
#include "subscriber_task.h"

/* Configuration file for MQTT client */
#include "mqtt_client_config.h"
#include "mqtt_topic_config.h"

/* Middleware libraries */
#include "cy_retarget_io.h"
#include "iot_mqtt.h"

#include "telemetry_queue.h"
#include <string.h>

/******************************************************************************
 * Macros
 ******************************************************************************/
/* The maximum number of times each PUBLISH in this example will be retried. */
#define PUBLISH_RETRY_LIMIT             (10)

/* A PUBLISH message is retried if no response is received within this 
 * time (in milliseconds).
 */
#define PUBLISH_RETRY_MS                (1000)

/******************************************************************************
 * Global Variables
 *******************************************************************************/
/* FreeRTOS task handle for this task. */
TaskHandle_t publisher_task_handle;

/* Structure to store publish message information. */
IotMqttPublishInfo_t publishInfo = { .qos = (IotMqttQos_t) MQTT_MESSAGES_QOS,
        .pTopicName = MQTT_TOPIC_NANODRONE, .topicNameLength =
                (sizeof(MQTT_TOPIC_NANODRONE) - 1), .retryMs = PUBLISH_RETRY_MS,
        .retryLimit = PUBLISH_RETRY_LIMIT };

extern QueueHandle_t telemetry_queue;

uint8_t pub_message[TELEMETRY_MESSAGE_SIZE];

/******************************************************************************
 * Function Name: publisher_task
 ******************************************************************************
 * Summary:
 *  Task that handles initialization of the TELEMETRY message queue,
 *  and publishing of MQTT messages to control the device that is actuated
 *  by the subscriber task.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void publisher_task(void *pvParameters) {
    /* Status variable */
    int result;

    /* Status of MQTT publish operation that will be communicated to MQTT 
     * client task using a message queue in case of failure during publish.
     */
    mqtt_result_t mqtt_publish_status = MQTT_PUBLISH_FAILURE;

    /* To avoid compiler warnings */
    (void) pvParameters;

    initTelemetryQueue();
    printf("TELEMETRY Message Queue created\n\n");

    while (true) {
        if (xQueueReceive(telemetry_queue, pub_message, portMAX_DELAY)) {
            cyhal_gpio_toggle(CYBSP_USER_LED);

            publishInfo.pPayload = pub_message;
            publishInfo.payloadLength = sizeof(pub_message);
            ;

            printf("Publishing '%s' on the topic '%s'\n\n",
                    (char*) publishInfo.pPayload, publishInfo.pTopicName);

            /* Publish the MQTT message with the configured settings. */
            result = IotMqtt_PublishSync(mqttConnection, &publishInfo, 0,
            MQTT_TIMEOUT_MS);

            if (result != IOT_MQTT_SUCCESS) {
                /* Inform the MQTT client task about the publish failure and suspend
                 * the task for it to be killed by the MQTT client task later.
                 */
                printf("MQTT Publish failed with error '%s'.\n\n",
                        IotMqtt_strerror((IotMqttError_t) result));
                xQueueOverwrite(mqtt_status_q, &mqtt_publish_status);
                vTaskSuspend( NULL);
            }
            cyhal_gpio_toggle(CYBSP_USER_LED);
        }
    }
}

/******************************************************************************
 * Function Name: publisher_cleanup
 ******************************************************************************
 * Summary:
 *  Cleanup function for the publisher task
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void publisher_cleanup(void) {
}

/* [] END OF FILE */

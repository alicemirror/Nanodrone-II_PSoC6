/******************************************************************************
* File Name:   subscriber_task.c
*
* Description: This file contains the optional task that
*              subscribes to the topic 'MQTT_TOPIC_NANODRONE', and does currently nothing
*              based on the notifications received from the MQTT subscriber
*              callback.
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
#include "string.h"
#include "FreeRTOS.h"

/* Task header files */
#include "subscriber_task.h"
#include "mqtt_task.h"

/* Configuration file for MQTT client */
#include "mqtt_client_config.h"
#include "mqtt_topic_config.h"

/* Middleware libraries */
#include "cy_retarget_io.h"
#include "iot_mqtt.h"

#include "telemetry_queue.h"

/******************************************************************************
* Macros
******************************************************************************/
/* The number of MQTT topics to be subscribed to. */
#define SUBSCRIPTION_COUNT          (1)

/******************************************************************************
* Function Prototypes
*******************************************************************************/
static void mqtt_subscription_callback(void *pCallbackContext,
                                       IotMqttCallbackParam_t *pPublishInfo);

/******************************************************************************
* Global Variables
*******************************************************************************/
/* Task handle for this task. */
TaskHandle_t subscriber_task_handle;


/* Configure the subscription information structure. */
IotMqttSubscription_t subscribeInfo =
{
    .qos = (IotMqttQos_t) MQTT_MESSAGES_QOS,
    .pTopicFilter = MQTT_TOPIC_NANODRONE,
    .topicFilterLength = (sizeof(MQTT_TOPIC_NANODRONE) - 1),
    /* Configure the callback function to handle incoming MQTT messages */
    .callback.function = mqtt_subscription_callback
};


uint8_t message[TELEMETRY_MESSAGE_SIZE];

#define PAYLOAD_GOOD                    (0x00lu)
#define PAYLOAD_BAD                   (0x01lu)

/******************************************************************************
 * Function Name: subscriber_task
 ******************************************************************************
 * Summary:
 *  Task that subscribes to topic - 'MQTT_TOPIC_NANODRONE',
 *  and ignores the returned payload currently.
 *
 * Parameters:
 *  void *pvParameters : Task parameter defined during task creation (unused)
 *
 * Return:
 *  void
 *
 ******************************************************************************/
void subscriber_task(void *pvParameters)
{
    /* Status variable */
    int result = EXIT_SUCCESS;

    uint32_t received_payload_status;

    /* Status of MQTT subscribe operation that will be communicated to MQTT 
     * client task using a message queue in case of failure in subscription.
     */
    mqtt_result_t mqtt_subscribe_status = MQTT_SUBSCRIBE_FAILURE;

    /* To avoid compiler warnings */
    (void)pvParameters;


    /* Subscribe with the configured parameters. */
    result = IotMqtt_SubscribeSync(mqttConnection,
                                   &subscribeInfo,
                                   SUBSCRIPTION_COUNT,
                                   0,
                                   MQTT_TIMEOUT_MS);
    if (result != EXIT_SUCCESS)
    {
        /* Notify the MQTT client task about the subscription failure and  
         * suspend the task for it to be killed by the MQTT client task later.
         */
        printf("MQTT Subscribe failed with error '%s'.\n\n",
               IotMqtt_strerror((IotMqttError_t) result));
        xQueueOverwrite(mqtt_status_q, &mqtt_subscribe_status);
        vTaskSuspend( NULL );
    }

    printf("MQTT client subscribed to the topic '%.*s' successfully.\n\n",
           subscribeInfo.topicFilterLength, subscribeInfo.pTopicFilter);

    while (true)
    {
        /* Block till a notification is received from the subscriber callback. */
        xTaskNotifyWait(0, 0, &received_payload_status, portMAX_DELAY);

        // you can handle payloads here
        if (received_payload_status == PAYLOAD_GOOD) {
        	// todo handle payload
        }

    }
}

/******************************************************************************
 * Function Name: mqtt_subscription_callback
 ******************************************************************************
 * Summary:
 *  Callback to handle incoming MQTT messages. This callback prints the 
 *  contents of an incoming message and notifies the subscriber task
 *  with a PAYLOAD GOOD message.
 *
 * Parameters:
 *  void *pCallbackContext : Parameter defined during MQTT Subscribe operation
 *                           using the IotMqttCallbackInfo_t.pCallbackContext
 *                           member (unused)
 *  IotMqttCallbackParam_t * pPublishInfo : Information about the incoming 
 *                                          MQTT PUBLISH message passed by
 *                                          the MQTT library.
 *
 * Return:
 *  void
 *
 ******************************************************************************/
static void mqtt_subscription_callback(void *pCallbackContext,
                                       IotMqttCallbackParam_t *pPublishInfo)
{
    /* Received MQTT message */
    const char *pPayload = pPublishInfo->u.message.info.pPayload;
    /* state that should be sent to  task depending on received message. */
    uint32_t nanodrone_payload_state = PAYLOAD_GOOD;

    /* To avoid compiler warnings */
    (void) pCallbackContext;

    memcpy(message, pPayload, TELEMETRY_MESSAGE_SIZE);

    /* Print information about the incoming PUBLISH message. */
    printf("Incoming MQTT message received:\n"
           "Subscription topic filter: %.*s\n"
           "Published topic name: %.*s\n"
           "Published QoS: %d\n"
           "Published payload: %.*s\n\n",
           pPublishInfo->u.message.topicFilterLength,
           pPublishInfo->u.message.pTopicFilter,
           pPublishInfo->u.message.info.topicNameLength,
           pPublishInfo->u.message.info.pTopicName,
           pPublishInfo->u.message.info.qos,
           pPublishInfo->u.message.info.payloadLength,
           pPayload);


    /* Notify the subscriber task about the received a subscribed message. */
    xTaskNotify(subscriber_task_handle, nanodrone_payload_state, eSetValueWithoutOverwrite);
}

/******************************************************************************
 * Function Name: mqtt_unsubscribe
 ******************************************************************************
 * Summary:
 *  Function that unsubscribes from the topic specified by the macro 
 *  'MQTT_TOPIC'. This operation is called during cleanup by the MQTT client 
 *  task.
 *
 * Parameters:
 *  void 
 *
 * Return:
 *  void 
 *
 ******************************************************************************/
void mqtt_unsubscribe(void)
{
    IotMqttError_t result = IotMqtt_UnsubscribeSync(mqttConnection,
                                                    &subscribeInfo,
                                                    SUBSCRIPTION_COUNT,
                                                    0,
                                                    MQTT_TIMEOUT_MS);
    if (result != IOT_MQTT_SUCCESS)
    {
        printf("MQTT Unsubscribe operation failed with error '%s'!\n",
               IotMqtt_strerror(result));
    }
}

/* [] END OF FILE */

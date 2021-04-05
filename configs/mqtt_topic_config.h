/*
 * mqtt_topic_config.h
 *
 *  Created on: Mar 20, 2021
 *      Author: enrico
 */

#ifndef CONFIGS_MQTT_TOPIC_CONFIG_H_
#define CONFIGS_MQTT_TOPIC_CONFIG_H_

// -------------------------------------------------------------------------------
// MQTT Topics
// -------------------------------------------------------------------------------

/**
 * The MQTT topic on which the acquired image rating is published and subscribed.
 * Every image information rating (including all the data fields) is associated
 * to the corresponding field, identifying a specific geographical area.
 */
#define MQTT_TOPIC_NANODRONE                        "nanodrone"

/* Configuration for the 'Last Will and Testament (LWT)'. It is an MQTT message
 * that will be published by the MQTT broker if the MQTT connection is
 * unexpectedly closed. This configuration is sent to the MQTT broker during
 * MQTT connect operation and the MQTT broker will publish the Will message on
 * the Will topic when it recognizes an unexpected disconnection from the client.
 *
 * If you want to use the last will message, set this macro to 1, else 0.
 */
#define ENABLE_LWT_MESSAGE                ( 0 )

#if ENABLE_LWT_MESSAGE
    #define MQTT_WILL_TOPIC_NAME          MQTT_TOPIC_NANODRONE "/will"
    #define MQTT_WILL_MESSAGE             ("MQTT client unexpectedly disconnected!")
#endif

#endif /* CONFIGS_MQTT_TOPIC_CONFIG_H_ */

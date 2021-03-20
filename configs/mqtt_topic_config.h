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

#endif /* CONFIGS_MQTT_TOPIC_CONFIG_H_ */

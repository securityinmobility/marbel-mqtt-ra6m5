/*
 * mqtt_helper.h
 *
 *  This file contains definitions of helper Functions that use MQTT API calls
 *  Created on: 09.08.2023
 *      Author: Robert Kalmar
 */

#ifndef MQTT_HELPER_H_
#define MQTT_HELPER_H_

#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"
#include "common_utils.h"
#include "user_config.h"
#include "using_mbedtls_pkcs11.h"
#include "transport/transport_interface.h"
#include "core_pkcs11_config.h"
#include "backoffAlgorithm/backoff_algorithm.h"

#define MILLISECONDS_PER_TICK               ( 1000U / configTICK_RATE_HZ )
#define mqttexampleNETWORK_BUFFER_SIZE      ( 2048U )
#define KEEP_ALIVE_TIMEOUT_SECONDS                   ( 600U )
#define CONNACK_RECV_TIMEOUT_MS                      ( 3000U ) //Timeout for receiving CONNACK packet in milliseconds
#define PUBLISH_RECV_TIMEOUT_MS                      ( 1000U )  //Timeout for receiving Publish packet in milliseconds
/**
 * @brief The maximum number of retries for subscribing to topic filter when broker rejects an attempt.
 */
#define SUBSCRIBE_RETRY_MAX_ATTEMPTS                 ( 5U )

/**
 * @brief The maximum back-off delay (in milliseconds) for retrying subscription to topic.
 */
#define SUBSCRIBE_RETRY_MAX_BACKOFF_DELAY_MS         ( 5000U )

/**
 * @brief The base back-off delay (in milliseconds) to use for subscription retry attempts.
 */
#define SUBSCRIBE_RETRY_BACKOFF_BASE_MS              ( 5000U )

/**
 * @brief Transport timeout in milliseconds for transport send and receive.
 */
#define mqttexampleTRANSPORT_SEND_RECV_TIMEOUT_MS         ( 10000U )


/**
 * @brief A pair containing a topic filter and its SUBACK status.
 */
typedef struct topicFilterContext
{
    const char * pcTopicFilter;
    MQTTSubAckStatus_t xSubAckStatus;
} topicFilterContext_t;


TlsTransportStatus_t prvTLSConnectToServerWithBackoffRetries(NetworkCredentials_t * pxNetworkCredentials, mbedNetworkContext_t * pxNetworkContext);

void prvEventCallback( MQTTContext_t * pxMQTTContext, MQTTPacketInfo_t * pxPacketInfo, MQTTDeserializedInfo_t * pxDeserializedInfo );
void prvMQTTProcessResponse(MQTTPacketInfo_t * pxIncomingPacket, uint16_t usPacketId);

uint32_t prvGetTimeMs( void );
long int plaintext_send(NetworkContext_t * pNetworkContext, const void * pBuffer, size_t bytesToSend);
long int plaintext_recv(NetworkContext_t * pNetworkContext, void * pBuffer, size_t bytesToRecv);
uint8_t prvCreateMQTTConnectionWithBroker(MQTTContext_t * pxMQTTContext, mbedNetworkContext_t * pxNetworkContext);
void prvMQTTPublishToTopic(MQTTContext_t * pxMQTTContext, MQTTPublishInfo_t pxMQTTPublishInfo);
void prvMQTTSubscribeWithBackoffRetries(MQTTContext_t * pxMQTTContext, const char * pcTopicFilter, uint16_t usTopicFilterLength );
void prvMQTTProcessIncomingPublish(MQTTPublishInfo_t * pxPublishInfo);

BaseType_t provision_alt_key(void);

#endif /* MQTT_HELPER_H_ */

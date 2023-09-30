/*
 * mqtt_helper.c
 *
 *  This file contains helper Functions that use MQTT API calls
 *  Created on: 09.08.2023
 *      Author: Robert Kalmar
 */

#include "mqtt_helper.h"

/**
 * @brief A struct containing the context of a SUBACK; the SUBACK status
 * of a filter is updated when the event callback processes a SUBACK.
 */
static topicFilterContext_t xTopicFilterContext = {SUBSCRIBE_TOPIC, MQTTSubAckFailure};

/**
 * @brief Packet Identifier generated when Subscribe request was sent to the broker;
 * it is used to match received Subscribe ACK to the transmitted subscribe.
 */
uint16_t globalSubscribePacketIdentifier = 0U;
uint32_t ulGlobalEntryTimeMs = 0U;
uint8_t receivedMessagesCount = 0U;


//char CLIENT_CERTIFICATE_PEM[2048];
//char CLIENT_KEY_PEM[2048];
//char USER_MQTT_ENDPOINT[128];
/* Static buffer used to hold MQTT messages being sent and received. */
static uint8_t ucSharedBuffer[mqttexampleNETWORK_BUFFER_SIZE];
/* Static buffer used to hold MQTT messages being sent and received. */
static MQTTFixedBuffer_t xBuffer =
{
    .pBuffer = ucSharedBuffer,
    .size    = mqttexampleNETWORK_BUFFER_SIZE
};


/// @brief Establishes a TLS connection to a specified Server using TLS_FreeRTOS_Connect (not MQTT_Connect)
/// @param pxNetworkCredentials pointer to empty NetworkCredentials_t struct
/// @param pxNetworkContext 
/// @return 
TlsTransportStatus_t prvTLSConnectToServerWithBackoffRetries(NetworkCredentials_t * pxNetworkCredentials, mbedNetworkContext_t * pxNetworkContext)
{
    TlsTransportStatus_t xNetworkStatus;
    BackoffAlgorithmStatus_t xBackoffAlgStatus = BackoffAlgorithmSuccess;
    BackoffAlgorithmContext_t xReconnectParams;
    uint16_t usNextRetryBackOff = 0U;

    // Set the credentials for establishing a TLS connection
    pxNetworkCredentials->pRootCa = (const unsigned char *) ROOT_CA_PEM;
    pxNetworkCredentials->rootCaSize = sizeof(ROOT_CA_PEM);
    pxNetworkCredentials->disableSni = pdFALSE;
    pxNetworkCredentials->pUserName = NULL;
    pxNetworkCredentials->userNameSize = 0;
    pxNetworkCredentials->pPassword = NULL;
    pxNetworkCredentials->passwordSize = 0;
    pxNetworkCredentials->pClientCertLabel = pkcs11configLABEL_DEVICE_CERTIFICATE_FOR_TLS;
    pxNetworkCredentials->pPrivateKeyLabel = pkcs11configLABEL_DEVICE_PRIVATE_KEY_FOR_TLS;
    pxNetworkCredentials->pAlpnProtos = NULL;

    // Create empty SSL Context
    pxNetworkContext->pParams->sslContext = (SSLContext_t) {0};

    /* Initialize reconnect attempts and interval.*/
    BackoffAlgorithm_InitializeParams( &xReconnectParams,
                                       SUBSCRIBE_RETRY_BACKOFF_BASE_MS,
                                       SUBSCRIBE_RETRY_MAX_BACKOFF_DELAY_MS,
                                       SUBSCRIBE_RETRY_MAX_ATTEMPTS );

    /* Attempt to connect to the MQTT broker. If connection fails, retry after
     * a timeout. Timeout value will exponentially increase until maximum
     * attempts are reached.
     */
    do
    {
        /* Establish a TLS session with the MQTT broker. This example connects to
         * the MQTT broker as specified in MQTT_BROKER_ENDPOINT and
         * MQTT_BROKER_PORT at the top of this file. */
        APP_PRINT("Connect to %s:%d.\r\n",MQTT_BROKER_ENDPOINT,MQTT_BROKER_PORT);

        /* Attempt to create a server-authenticated TLS connection. */
        xNetworkStatus = TLS_FreeRTOS_Connect( pxNetworkContext,
                                               MQTT_BROKER_HOSTNAME,
                                               MQTT_BROKER_PORT,
                                               pxNetworkCredentials,
                                               mqttexampleTRANSPORT_SEND_RECV_TIMEOUT_MS,
                                               mqttexampleTRANSPORT_SEND_RECV_TIMEOUT_MS );
        if( xNetworkStatus != TLS_TRANSPORT_SUCCESS )
        {
            /* Generate a random number and calculate backoff value (in milliseconds) for
             * the next connection retry. */
            xBackoffAlgStatus = BackoffAlgorithm_GetNextBackoff( &xReconnectParams, ulongRand(), &usNextRetryBackOff );

            if( xBackoffAlgStatus == BackoffAlgorithmRetriesExhausted )
            {
                APP_PRINT( "Connection to the broker failed, all attempts exhausted.\r\n" );
            }
            else if( xBackoffAlgStatus == BackoffAlgorithmSuccess )
            {
                APP_PRINT( "Connection to the broker failed. Retrying connection with backoff and jitter.\r\n" );
                vTaskDelay( pdMS_TO_TICKS( usNextRetryBackOff ) );
            }
        }
    } while( ( xNetworkStatus != TLS_TRANSPORT_SUCCESS ) && ( xBackoffAlgStatus == BackoffAlgorithmSuccess ) );

    return xNetworkStatus;
}


/// @brief Get the current time since vTaskScheduler was called in ms
/// @param
/// @return returns elapsed time since vTaskScheduler was called in ms
uint32_t prvGetTimeMs( void )
{
    TickType_t xTickCount = 0;
    uint32_t ulTimeMs = 0UL;

    /* Get the current tick count. */
    xTickCount = xTaskGetTickCount();

    /* Convert the ticks to milliseconds. */
    ulTimeMs = ( uint32_t ) xTickCount * MILLISECONDS_PER_TICK;

    /* Reduce ulGlobalEntryTimeMs from obtained time so as to always return the
     * elapsed time in the application. */
    ulTimeMs = ( uint32_t ) ( ulTimeMs - ulGlobalEntryTimeMs );

    return ulTimeMs;
}

/// @brief Makes Prechecks and tries to send Data with FreeRTOS_send()
/// @param pNetworkContext
/// @param pBuffer
/// @param bytesToSend
/// @return Number of bytes sent or a negative value to indicate error
long int plaintext_send(NetworkContext_t * pNetworkContext, const void * pBuffer, size_t bytesToSend)
{
    long int sendStatus = 0;
    if( ( pNetworkContext == NULL ) || ( pNetworkContext->netSocket == NULL ) )
    {
        LogError( ( "invalid input, pNetworkContext=%p", pNetworkContext ) );
        sendStatus = -1;
    }
    else if( pBuffer == NULL )
    {
        LogError( ( "invalid input, pBuffer == NULL" ) );
        sendStatus = -1;
    }
    else if( bytesToSend == 0 )
    {
        LogError( ( "invalid input, bytesToSend == 0" ) );
        sendStatus = -1;
    }else {
        // Try to send Data
        sendStatus = FreeRTOS_send((pNetworkContext->netSocket), pBuffer, bytesToSend, 0);
        if(sendStatus > 0)
        {
            //APP_PRINT("Successfully Sent %d Bytes\r\n", sendStatus);
        }else
        {
            APP_PRINT("Send Error: %d\r\n", sendStatus);
            configASSERT(sendStatus > 0);
        }
    }
    return sendStatus;
}

/// @brief Converts pNetworkContext to usable FreeRTOS recv
/// @param pNetworkContext
/// @param pBuffer
/// @param bytesToRecv
/// @return Number of bytes received or a negative value to indicate error
long int plaintext_recv(NetworkContext_t * pNetworkContext, void * pBuffer, size_t bytesToRecv)
{
    return FreeRTOS_recv((pNetworkContext->netSocket), pBuffer, bytesToRecv, 0);
}

/// @brief Establishes a connection to a MQTT broker with a clean session and uses existing TCP Socket
/// @param pxMQTTContext
/// @param pxNetworkContext
/// @return returns false for failure and true for success
uint8_t prvCreateMQTTConnectionWithBroker(MQTTContext_t * pxMQTTContext, mbedNetworkContext_t * pxNetworkContext)
{
    APP_PRINT("Trying to establish connection to MQTT Broker\r\n")
    MQTTStatus_t eMqttStatus = MQTTSuccess;
    MQTTConnectInfo_t xConnectInfo;
    bool xSessionPresent = false;
    bool returnCode = false;
    TransportInterface_t xTransport;

    // configure Transport Interface
    xTransport.pNetworkContext = pxNetworkContext;
    xTransport.send = TLS_FreeRTOS_send;
    xTransport.recv = TLS_FreeRTOS_recv;

    // Initialize MQTT
    APP_PRINT("Initialize MQTT\r\n");
    eMqttStatus = MQTT_Init(pxMQTTContext, &xTransport, prvGetTimeMs,prvEventCallback, &xBuffer);

    if( eMqttStatus != MQTTSuccess )
    {
        APP_PRINT("MQTT init failed with status %s.\r\n", MQTT_Status_strerror( eMqttStatus ));
    }
    else
    {
        /* Establish MQTT session by sending a CONNECT packet. */
        ( void ) memset( ( void * ) &xConnectInfo, 0x00, sizeof( xConnectInfo ) );

        /* If #createCleanSession is true, start with a clean session
         * i.e. direct the MQTT broker to discard any previous session data.
         * If #createCleanSession is false, directs the broker to attempt to
         * reestablish a session which was already present. */
        xConnectInfo.cleanSession = true;

        /* The client identifier is used to uniquely identify this MQTT client to
         * the MQTT broker. In a production device the identifier can be something
         * unique, such as a device serial number. */
        xConnectInfo.pClientIdentifier = CLIENT_IDENTIFIER;
        xConnectInfo.clientIdentifierLength = ( uint16_t ) strlen( CLIENT_IDENTIFIER );

        /* The maximum time interval in seconds which is allowed to elapse
         * between two Control Packets.
         * It is the responsibility of the Client to ensure that the interval between
         * Control Packets being sent does not exceed this Keep Alive value. In the
         * absence of sending any other Control Packets, the Client MUST send a
         * PINGREQ Packet. */
        xConnectInfo.keepAliveSeconds = KEEP_ALIVE_TIMEOUT_SECONDS;

        /* Send MQTT CONNECT packet to broker. */
        APP_PRINT("Sending MQTT_CONNECT packet to broker\r\n");
        eMqttStatus = MQTT_Connect( pxMQTTContext, &xConnectInfo, NULL, CONNACK_RECV_TIMEOUT_MS, &xSessionPresent );

        if( eMqttStatus != MQTTSuccess )
        {
            APP_PRINT("Connection with MQTT broker failed with status %s.\r\n",MQTT_Status_strerror( eMqttStatus ));
            returnCode = false;
        }
        else
        {
            APP_PRINT("MQTT connection established with broker\r\n");
            returnCode = true;
        }
    }
    return returnCode;
}

/// @brief Event Callback for MQTT specified in MQTT_Init
/// @param pxMQTTContext 
/// @param pxPacketInfo 
/// @param pxDeserializedInfo 
void prvEventCallback( MQTTContext_t * pxMQTTContext, MQTTPacketInfo_t * pxPacketInfo, MQTTDeserializedInfo_t * pxDeserializedInfo )
{
    /* The MQTT context is not used for this demo. */
    ( void ) pxMQTTContext;

    // Packet Received Callback
   if( ( pxPacketInfo->type & 0xF0U ) == MQTT_PACKET_TYPE_PUBLISH )
   {
       prvMQTTProcessIncomingPublish( pxDeserializedInfo->pPublishInfo );
   }
   else
   {
       prvMQTTProcessResponse( pxPacketInfo, pxDeserializedInfo->packetIdentifier );
   }
}


/// @brief Publishes Data to a MQTT Topic using MQTT_Publish
/// @param pxMQTTContext
/// @param pxMQTTPublishInfo Data to be published
void prvMQTTPublishToTopic(MQTTContext_t * pxMQTTContext, MQTTPublishInfo_t pxMQTTPublishInfo)
{
    MQTTStatus_t eMqttStatus = MQTTSuccess;
    eMqttStatus = MQTT_Publish(pxMQTTContext, &pxMQTTPublishInfo, CONNACK_RECV_TIMEOUT_MS);
    if( eMqttStatus != MQTTSuccess )
    {
        APP_PRINT("MQTT Publish failed with status %s.\r\n",MQTT_Status_strerror( eMqttStatus ));
    }
    else
    {
        APP_PRINT("MQTT Publish Success.\r\n");
    }
}

/// @brief Subscribes to a MQTT Topic using MQTT_Subscribe
/// @param pxMQTTContext existing MQTT Context
/// @param pcTopicFilter topic to subscribe to  
/// @param usTopicFilterLength length of topic to subscribe to
void prvMQTTSubscribeWithBackoffRetries(MQTTContext_t * pxMQTTContext, const char * pcTopicFilter, uint16_t usTopicFilterLength )
{
    MQTTStatus_t xMqttStatus = MQTTSuccess;
    BackoffAlgorithmContext_t xRetryParams;
    BaseType_t xBackOffStatus = pdFAIL;
    BaseType_t xSubscribeStatus = pdFAIL;
    MQTTSubscribeInfo_t xMQTTSubscription;
    bool xFailedSubscribeToTopic = false;

    // Some fields not used to start with everything at 0
    ( void ) memset( ( void * ) &xMQTTSubscription, 0x00, sizeof( xMQTTSubscription ) );

    // Initialize the status of the subscription acknowledgement
    xTopicFilterContext.pcTopicFilter =  pcTopicFilter;
    xTopicFilterContext.xSubAckStatus = MQTTSubAckFailure;

    // Get a unique packet id
    globalSubscribePacketIdentifier = MQTT_GetPacketId(pxMQTTContext);

    // Subsrcibe to pcTopicFilter topic filter that is passed
    xMQTTSubscription.qos = MQTTQoS0;
    xMQTTSubscription.pTopicFilter = pcTopicFilter;
    xMQTTSubscription.topicFilterLength = usTopicFilterLength;

    // Initialize retry attempts and interval
    BackoffAlgorithm_InitializeParams(  &xRetryParams,
                                        SUBSCRIBE_RETRY_BACKOFF_BASE_MS,
                                        SUBSCRIBE_RETRY_MAX_BACKOFF_DELAY_MS,
                                        SUBSCRIBE_RETRY_MAX_ATTEMPTS );

    do{
        xSubscribeStatus = pdFAIL;

        /** The client should allready be connected to the broker
         * so subscribe to the topic by sending a subscribe packet */
        APP_PRINT("Attempting to subscribe to MQTT Topic %s\r\n",pcTopicFilter);
        xMqttStatus = MQTT_Subscribe(   pxMQTTContext,
                                        &xMQTTSubscription,
                                        sizeof(xMQTTSubscription) / sizeof(MQTTSubscribeInfo_t),
                                        globalSubscribePacketIdentifier);
        //configASSERT(xMqttStatus == MQTTSuccess);

        if(xMqttStatus != MQTTSuccess)
        {
            APP_PRINT("MQTT_Subscribe failed with status %s.\r\n",MQTT_Status_strerror( xMqttStatus ));
        }else
        {
            APP_PRINT("SUBSCRIBE sent for topic: \"%s\" to Broker \r\n",pcTopicFilter);
            /* Process incoming packet from broker. Client may receive publish packet before subscribe ack.
            * Therefore, call generic incoming packet processing function. */
           xMqttStatus = MQTT_ProcessLoop(pxMQTTContext,SUBSCRIBE_RETRY_BACKOFF_BASE_MS);
           xFailedSubscribeToTopic = false;

            /* Check if recent subscription request has been rejected
            * #xTopicFilterContext is updated in the event callback*/
            if(xTopicFilterContext.xSubAckStatus == MQTTSubAckFailure)
            {
                xFailedSubscribeToTopic = true;
                break;
            }
        }
    }while ((xFailedSubscribeToTopic == true));
}

/// @brief Processes incoming PUBLISH packets from the MQTT Broker
/// @param pxPublishInfo
void prvMQTTProcessIncomingPublish(MQTTPublishInfo_t * pxPublishInfo){

    // Verify the received publish for the topic we are subscribed to
    // Should probably check for multiple topics here if subscribe to multiple topics but not needed for this demo
    if(pxPublishInfo->topicNameLength == strlen(SUBSCRIBE_TOPIC) &&
        ( 0 == strncmp( SUBSCRIBE_TOPIC, pxPublishInfo->pTopicName,pxPublishInfo->topicNameLength ) ) )
    {
        // allocate temporary memory for printing topic name (SEGGER_RTT_printf doesnt allow custom length printing)
        char *tempTopicName = (char *)malloc(pxPublishInfo->topicNameLength + 1);
        if(!tempTopicName){
            APP_PRINT("Memory allocation failed!\r\n");
        }else
        {
            strncpy(tempTopicName, pxPublishInfo->pTopicName, pxPublishInfo->topicNameLength);
            tempTopicName[pxPublishInfo->topicNameLength] = '\0';
        }

        // allocate temporary String memory for printing payload (SEGGER_RTT_printf doesnt allow custom length printing)
        char *tempPayload = (char *)malloc(pxPublishInfo->payloadLength + 1);
        if(!tempPayload){
            APP_PRINT("Memory allocation failed!\r\n");
        }else
        {
            strncpy(tempPayload, pxPublishInfo->pPayload, pxPublishInfo->payloadLength);
            tempPayload[pxPublishInfo->payloadLength] = '\0';
        }

        APP_PRINT("Incoming Publish Topic Name: %s matches subscribed Topic.\r\n", tempTopicName);
        APP_PRINT("Incoming Publish Payload: %s\r\n", tempPayload);

        // Free String Buffers
        free(tempTopicName);
        free(tempPayload);

        // increment received message count #debug
        receivedMessagesCount++;
        APP_PRINT("Received %d Messages\r\n",receivedMessagesCount);
    }else
    {
        // allocate temporary String memory for printing topic name
        char *tempTopicName = (char *)malloc(pxPublishInfo->topicNameLength + 1);
        if(!tempTopicName){
            APP_PRINT("Memory allocation failed!\r\n");
        }else
        {
            strncpy(tempTopicName, pxPublishInfo->pTopicName, pxPublishInfo->topicNameLength);
        }

        APP_PRINT("Incoming Publish Topic Name: %s does not match subscribed Topic.\r\n", tempTopicName);

        // free
        free(tempTopicName);
    }
}

/// @brief Processes acknowledgement packets
/// @param pxIncomingPacket
/// @param usPacketId
void prvMQTTProcessResponse(MQTTPacketInfo_t * pxIncomingPacket, uint16_t usPacketId)
{
    MQTTStatus_t xResult = MQTTSuccess;
    uint8_t * pucPayload = NULL;
    size_t ulSize = 0U;

    // Check the type of incoming Packet
    switch(pxIncomingPacket->type)
    {
        case MQTT_PACKET_TYPE_SUBACK:
            /** A SUBACK from the Broker, containing the server response to our subscription request,
             *  has been received. It contains the status code indicating server approval/rejection
             * for the subscription to the single topic requested. The SUBACK will be parsed to obtain
             * the status code, and this status code will be stored in #xTopicFilterContext*/
            xResult = MQTT_GetSubAckStatusCodes(pxIncomingPacket, &pucPayload, &ulSize);
            // MQTT_GetSubAckStatusCodes() returns MQTTSuccess if the SUBACK packet was valid.
            configASSERT( xResult == MQTTSuccess );

            // should be QOS level
            xTopicFilterContext.xSubAckStatus = *pucPayload;
            APP_PRINT("SUBACK received\r\n");
        break;

        case MQTT_PACKET_TYPE_UNSUBACK:
            APP_PRINT("Unsubscribed from topic %s.\r\n", SUBSCRIBE_TOPIC);
            break;

        case MQTT_PACKET_TYPE_PINGRESP:
            APP_PRINT("PINGRESP should not be handled by the application\r\n");
            break;

        default:
            APP_PRINT("prvMQTTProcessResponse called with unknown packet type: (%02X).",pxIncomingPacket->type);
    }
}

/********************************************************************************************************************//**
 * @brief      provision_alt_key function provides the device with client certificate and client key .
 * @param[in]  None
 * @retval     pdPASS                   Upon successful provision of client certificate and key
 * @return     Any other Error Code     Upon unsuccessful provision of client certificate and key
 **********************************************************************************************************************/
BaseType_t provision_alt_key(void)
{
    BaseType_t status = pdPASS;
    ProvisioningParams_t params = {RESET_VALUE};
    CK_RV xResult = CKR_OK;
    
    // set Credentials
    params.pucClientPrivateKey       = (uint8_t *) CLIENT_KEY_PEM;
    params.pucClientCertificate      = (uint8_t *) CLIENT_CERTIFICATE_PEM;
    params.ulClientPrivateKeyLength  = 1 + strlen((const char *) params.pucClientPrivateKey);
    params.ulClientCertificateLength = 1 + strlen((const char *) params.pucClientCertificate);
    params.pucJITPCertificate        = NULL;
    params.ulJITPCertificateLength   = RESET_VALUE;

    // provision the keys for later use by pkcs#11
    xResult = vAlternateKeyProvisioning(&params);
    if (CKR_OK != xResult)
    {
        APP_ERR_PRINT("\r\nFailed in vAlternateKeyProvisioning() function\r\n");
        return (BaseType_t) xResult;
    }

    APP_PRINT("\r\nSuccessfully provisioned the device with client certificate and client key\r\n");
    return status;
}










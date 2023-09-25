/***********************************************************************************************************************
 * File Name    : net_thread_entry.c
 * Description  : This file contains the User Application code for the Ethernet + TCP/IP
 ***********************************************************************************************************************/
/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/
#include "net_thread.h"

/* Domain for the DNS Host lookup is used in this Example Project.
 * The project can be built with different *domain_name to validate the DNS client
 */
char *domain_name = USR_TEST_DOMAIN_NAME;

/* IP address of the PC or any Device on the LAN/WAN where the Ping request is sent.
 * Note: Users needs to change this according to the LAN settings of your Test PC or device
 * when running this project.
 */
char *remote_ip_address = USR_TEST_PING_IP;

#if( ipconfigUSE_DHCP != 0 )
   /* DHCP populates these IP address, Sub net mask and Gateway Address. So start with this is zeroed out values
    * The MAC address is Test MAC address.
    */
    static  uint8_t ucMACAddress[ 6 ]       = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    static  uint8_t ucIPAddress[ 4 ]        = {RESET_VALUE};
    static  uint8_t ucNetMask[ 4 ]          = {255, 255, 255, 128};
    static  uint8_t ucGatewayAddress[ 4 ]   = {132, 158, 124, 1};
    static  uint8_t ucDNSServerAddress[ 4 ] = {RESET_VALUE};
#else
    /* Static IP configuration, when DHCP mode is not used for the Example Project.
     * This needs to be populated by the user according to the Network Settings of your LAN.
     * This sample address taken from the LAN where it is tested. This is different for different LAN.
     * get the Address using the PC IPconfig details.
     */
    static  uint8_t ucMACAddress[ 6 ]       = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
    static  uint8_t ucIPAddress[ 4 ]        = {192, 168, 178, 3};
    static  uint8_t ucNetMask[ 4 ]          = {255, 255, 255, 0};
    static  uint8_t ucGatewayAddress[ 4 ]   = {192, 168, 178, 1};
    static  uint8_t ucDNSServerAddress[ 4 ] = {10, 60, 1, 2};
#endif

#if( ipconfigUSE_DHCP != 0 )
    extern NetworkAddressingParameters_t xNetworkAddressing;
    NetworkAddressingParameters_t xNd = {RESET_VALUE, RESET_VALUE, RESET_VALUE, RESET_VALUE, RESET_VALUE};
#endif

uint32_t  dhcp_in_use   = RESET_VALUE;
uint32_t  usrPingCount  = RESET_VALUE;
ping_data_t ping_data   = {RESET_VALUE, RESET_VALUE, RESET_VALUE};
static uint32_t usr_print_ability = RESET_VALUE;

// Has to exist even though not assigned
static TlsTransportParams_t transport_params;

// extern variables
extern uint32_t ulGlobalEntryTimeMs;
extern uint8_t receivedMessagesCount;

// global variables
MQTTContext_t xMQTTContext;
MQTTStatus_t xMQTTStatus;

/*******************************************************************************************************************//**
* @brief      This is the User Thread for the EP.
* @param[in]  Thread specific parameters
* @retval     None
**********************************************************************************************************************/
void net_thread_entry(void *pvParameters)
{
    bool sendPings = false;
    bool mqttConEstablished = false, tlsConEstablished = false, gracefulDisconnect = false;

    BaseType_t status = pdFALSE;
    fsp_pack_version_t version = {RESET_VALUE};

    FSP_PARAMETER_NOT_USED (pvParameters);

    /* version get API for FLEX pack information */
    R_FSP_VersionGet (&version);

    /* Example Project information printed on the RTT */
    APP_PRINT (BANNER_INFO, EP_VERSION, version.version_id_b.major, version.version_id_b.minor, version.version_id_b.patch);

    int ierr = FSP_SUCCESS;
    fsp_err_t err = FSP_SUCCESS;

    // initialize and configure littlefs
    err = hal_littlefs_init();
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("** littleFs flash config failed **\r\n");
        APP_ERR_TRAP(err);
    }

    /* Initialize the crypto hardware acceleration. */
    ierr = mbedtls_platform_setup(NULL);
    if (FSP_SUCCESS != ierr)
    {
        APP_ERR_PRINT("** HW SCE Init failed **\r\n");
        APP_ERR_TRAP(ierr);
    }


    /* Prints the Ethernet Configuration prior to the IP Init*/
    APP_PRINT(ETH_PREINIT);
    print_ipconfig();

    /* FreeRTOS IP Initialization: This init initializes the IP stack  */
    status = FreeRTOS_IPInit(ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress);
    if(pdFALSE == status)
    {
        APP_ERR_PRINT("FreeRTOS_IPInit Failed");        
        hal_littlefs_deinit();
        mbedtls_platform_teardown(NULL);
        APP_ERR_TRAP(status);
    }

    APP_PRINT(ETH_POSTINIT);

    Socket_t xSocket = { 0 };
    mbedNetworkContext_t xNetworkContext = { 0 };
    TlsTransportParams_t xTlsTransportParams = { 0 };
    NetworkCredentials_t xNetworkCredentials = { 0 };

    // counters for messages
    uint32_t ulPublishCount = 0U, ulTopicCount = 0U;
    const uint8_t ulMaxPublishCount = 10UL;
    const uint8_t ulMaxReceiveCount = 4UL;
    uint8_t sendTries = 0;

    // Perform device key provisioning to be able to use them later via pkcs#11 labels
    status = provision_alt_key ();
    if (pdPASS != status)
    {
        APP_ERR_PRINT("\r\n Failed in provision_alt_key() function ");
        hal_littlefs_deinit();
        mbedtls_platform_teardown(NULL);
        APP_ERR_TRAP(status);
    }

    // only run while we are connected. After gracefully disconnecting, exit loop and delete task
    while(gracefulDisconnect == false)
    {
        /* Check if Both the Ethernet Link and IP link are UP */
        if(SUCCESS == isNetworkUp())
        {
            /* usr_print_ability is added to avoid multiple Up or Down Messages repeating*/
            if(!(PRINT_UP_MSG_DISABLE & usr_print_ability))
            {
                APP_PRINT("\r\nNetwork is Up");
                usr_print_ability |= PRINT_UP_MSG_DISABLE;
                /* Display the New IP credentials obtained from the DHCP server */
                updateDhcpResponseToUsr();
                print_ipconfig();
            }

            if(!(PRINT_UP_MSG_DISABLE & usr_print_ability))
            {
#if( ipconfigUSE_DHCP != 0 )
                /* Display the New IP credentials obtained from the DHCP server */
                updateDhcpResponseToUsr();
#endif
                /* Updated IP credentials on to the RTT console */
                print_ipconfig();
                /*DNS lookup for the Domain name requested. This is Synchronous Activity */
            }

            // Establish TLS Connection
            if(!tlsConEstablished)
            {
                /* Initialize the mqtt and network context. */
                ( void ) memset( &xNetworkContext, 0U, sizeof( mbedNetworkContext_t ) );
                xNetworkContext.pParams = &transport_params;
                xNetworkContext.pParams->tcpSocket = xSocket;

                TlsTransportStatus_t errorCode;
                errorCode = 0;

                ulGlobalEntryTimeMs = prvGetTimeMs();

                // Try connecting to the Server using TLS with Backoff tries
                errorCode = prvTLSConnectToServerWithBackoffRetries(&xNetworkCredentials, &xNetworkContext);

                // Error handling
                if(errorCode == TLS_TRANSPORT_SUCCESS)
                {
                    APP_PRINT("Connected Successfully\r\n");
                    tlsConEstablished = true;

                }else if(errorCode == TLS_TRANSPORT_INVALID_PARAMETER)
                {
                    APP_PRINT("Invalid Parameter for TLS_CONNECT.\r\n");
                }else if(errorCode == TLS_TRANSPORT_INSUFFICIENT_MEMORY)
                {
                    APP_PRINT("Insufficient Memory for TLS_CONNECT.\r\n");
                }else if(errorCode == TLS_TRANSPORT_INVALID_CREDENTIALS)
                {
                    APP_PRINT("Invalid Credentials for TLS_CONNECT.\r\n");
                }else if(errorCode == TLS_TRANSPORT_HANDSHAKE_FAILED)
                {
                    APP_PRINT("TLS Handshake failed.\r\n");
                }else if(errorCode == TLS_TRANSPORT_INTERNAL_ERROR)
                {
                    APP_PRINT("TLS Internal Error.\r\n");
                }else if(errorCode == TLS_TRANSPORT_CONNECT_FAILURE)
                {
                    APP_PRINT("TLS Connection failure.\r\n");
                }else
                {
                    APP_PRINT("Could not Connect to Socket. Error Code: %d\r\n",errorCode);
                }
            }

            // Connect to the Broker and Subscribe to Topic
            if(!mqttConEstablished && tlsConEstablished)
            {
                mqttConEstablished = prvCreateMQTTConnectionWithBroker(&xMQTTContext, &xNetworkContext);

                prvMQTTSubscribeWithBackoffRetries(&xMQTTContext, SUBSCRIBE_TOPIC, (uint16_t) strlen(SUBSCRIBE_TOPIC));
            }

            // MQTT Connection is Established. Try to send Data until we published ulMaxPublishCount amount of Messages
            if(mqttConEstablished && ulPublishCount < ulMaxPublishCount ) // && receivedMessagesCount < ulMaxReceiveCount
            {
                ulPublishCount++;
                APP_PRINT("Trying to send Data\r\n")

                // Create test data
                char testData[64];
                sprintf(testData, "Test Data %d",++sendTries);

                // Create and set Publish Info
                MQTTPublishInfo_t xPublishInfo;
                xPublishInfo.qos = MQTTQoS0;
                xPublishInfo.retain = false;
                xPublishInfo.dup = false;
                xPublishInfo.pTopicName = TEST_TOPIC_NAME;
                xPublishInfo.topicNameLength = (uint16_t) strlen(TEST_TOPIC_NAME);
                xPublishInfo.pPayload = testData;
                xPublishInfo.payloadLength = (uint32_t) strlen(testData);

                // Publish Data
                prvMQTTPublishToTopic(&xMQTTContext, xPublishInfo);
            }
            
            // Process Loop for handling incoming MQTT Messages. Stops after receiving ulMaxReceiveCount amount of Messages
            if(mqttConEstablished && receivedMessagesCount < ulMaxReceiveCount)
            {
                // Receive Subscription Data
                MQTT_ProcessLoop(&xMQTTContext, PUBLISH_RECV_TIMEOUT_MS);
                configASSERT(xMQTTStatus == MQTTSuccess);
            }

            // If we sent and received the wanted amount of Data, disconnect from MQTT Broker
            if(ulPublishCount >= ulMaxPublishCount && receivedMessagesCount >= ulMaxReceiveCount)
            {
                // MQTT Disconnect
                xMQTTStatus = MQTT_Disconnect(&xMQTTContext);
                if(xMQTTStatus != MQTTSuccess)
                {
                    APP_PRINT("Error while disconnecting. %s \r\n",MQTT_Status_strerror( xMQTTStatus ));
                }else
                {
                    // If Successful increment ulPublishCount once more to avoid sending multiple Disconnect Messages
                    ulPublishCount++;
                    APP_PRINT("Disconnected from MQTT Broker\r\n");
                }

                // Close the TLS Connection to the Server
                TLS_FreeRTOS_Disconnect(&xNetworkContext);

                // stop while loop
                gracefulDisconnect = true;
            }

            if(sendPings)
            {
                if(!(PRINT_NWK_USR_MSG_DISABLE & usr_print_ability))
                          {
                              APP_PRINT("\r\nPinging %s:\r\n\r\n",(char *)remote_ip_address);
                          }


                while (usrPingCount < USR_PING_COUNT)
                {
                    /* Send a ICMP Ping request to the requested IP address
                     * USR_PING_COUNT (100) is used in this Example Project
                     * For Continuous testing the count can be increased to bigger number
                     */
                    status =  vSendPing((char *)remote_ip_address);
                    if(status != pdFALSE)
                    {
                        ping_data.sent++;
                        APP_PRINT("!");
                    }
                    else
                    {
                        ping_data.lost++;
                        APP_PRINT(".");
                    }
                    usrPingCount++;
                    /* Add some delay between Pings */
                    vTaskDelay(10);
                }

                if(!(PRINT_NWK_USR_MSG_DISABLE & usr_print_ability))
                {
                    print_pingResult();
                    usr_print_ability |= PRINT_NWK_USR_MSG_DISABLE;
                }
            }
        }
        else
        {
            if(!(PRINT_DOWN_MSG_DISABLE & usr_print_ability))
            {
                 APP_PRINT("\r\nNetwork is Down");
                 usr_print_ability |= PRINT_DOWN_MSG_DISABLE;
            }
            else
            {
                 APP_PRINT(".");
            }
        }
        vTaskDelay(100);
    }
    APP_PRINT("Exited While Loop... Deleting Task...\r\n");
    vTaskDelete(NULL);
}

/*******************************************************************************************************************//**
 * @brief      Generates 32 bit Random number
 * @param[in]  void
 * @retval     Random Number
 **********************************************************************************************************************/
uint32_t ulongRand(void)
{
    /* example of a 32-bit random number generator.
     * Here rand() returns a 15-bit number. so create 32 bit Random number using 15 bit rand()
     */
    uint32_t ulResult =
        ((((uint32_t) rand()) & 0x7fffuL)       ) |
        ((((uint32_t) rand()) & 0x7fffuL) << 15 ) |
        ((((uint32_t) rand()) & 0x0003uL) << 30 );
    return ulResult;
}

 /*******************************************************************************************************************//**
 * @brief      Generates 32 sequence number
 * @param[in]  Source IP, Source Port, Destination IP and Destination Port.
 * @retval     Sequence Number
 **********************************************************************************************************************/
uint32_t ulApplicationGetNextSequenceNumber(uint32_t ulSourceAddress, uint16_t usSourcePort, uint32_t ulDestinationAddress,
                                            uint16_t usDestinationPort)
{
    /* Here we need to get random number for the sequence number.
     * This is just for testing purpose, so software rand() is okay.
     * This can also be tied to the TRNG.
     */
     return((ulSourceAddress + ulDestinationAddress + usSourcePort + usDestinationPort) && ulongRand());
}

/*******************************************************************************************************************//**
* @brief      Send ICMP Ping request  based on the user input IP Address.
* @param[in]  IP address to Ping
* @retval     Sequence Number
**********************************************************************************************************************/
BaseType_t vSendPing( const char *pcIPAddress)
{
uint32_t ulIPAddress = RESET_VALUE;

    /*
     * The pcIPAddress parameter holds the destination IP address as a string in
     * decimal dot notation (for example, “192.168.0.200”). Convert the string into
     * the required 32-bit format.
     */
    ulIPAddress = FreeRTOS_inet_addr(pcIPAddress);

    /*
     * Send a ping request containing 8 data bytes.  Wait (in the Blocked state) a
     * maximum of 100ms for a network buffer into which the generated ping request
     * can be written and sent.
     */
    return(FreeRTOS_SendPingRequest(ulIPAddress, 8, 100 / portTICK_PERIOD_MS));
}

/*******************************************************************************************************************//**
* @brief      User Hook for the Ping Reply. vApplicationPingReplyHook() is called by the TCP/IP
*             stack when the stack receives a ping reply.
* @param[in]  Ping reply status and Identifier
* @retval     None
**********************************************************************************************************************/
void vApplicationPingReplyHook( ePingReplyStatus_t eStatus, uint16_t usIdentifier )
{
    (void)  usIdentifier;

    switch( eStatus )
    {
        /* A valid ping reply has been received */
        case eSuccess    :
            ping_data.received++;
            break;
            /* A reply was received but it was not valid. */
        case eInvalidData :
        default:
            ping_data.lost++;
            break;
    }
}

/*******************************************************************************************************************//**
* @brief      This is the User Hook for the DHCP Response. xApplicationDHCPHook() is called by DHCP Client Code when DHCP
*             handshake messages are exchanged from the Server.
* @param[in]  Different Phases of DHCP Phases and the Offered IP Address
* @retval     Returns DHCP Answers.
**********************************************************************************************************************/
#if( ipconfigUSE_DHCP != 0 )
eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase,
                                            uint32_t ulIPAddress )
{
eDHCPCallbackAnswer_t eReturn = eDHCPContinue;
  /*
   * This hook is called in a couple of places during the DHCP process, as identified by the eDHCPPhase parameter.
   */
  switch( eDHCPPhase )
  {
    case eDHCPPhasePreDiscover  :
      /*
       *  A DHCP discovery is about to be sent out.  eDHCPContinue is returned to allow the discovery to go out.
       *  If eDHCPUseDefaults had been returned instead then the DHCP process would be stopped and the statically
       *  configured IP address would be used.
       *  If eDHCPStopNoChanges had been returned instead then the DHCP process would be stopped and whatever the
       *  current network configuration was would continue to be used.
       */
        break;

    case eDHCPPhasePreRequest  :
      /* An offer has been received from the DHCP server, and the offered IP address is passed in the ulIPAddress
       * parameter.
       */

        /*
         * The sub-domains don’t match, so continue with the DHCP process so the offered IP address is used.
         */
        /* Update the Structure, the DHCP state Machine is not updating this */
        xNetworkAddressing.ulDefaultIPAddress = ulIPAddress;
        dhcp_in_use = 1;
      break;

    default :
      /*
       * Cannot be reached, but set eReturn to prevent compiler warnings where compilers are disposed to generating one.
       */
        break;
  }

  return eReturn;
}
#endif


/*******************************************************************************************************************//**
 * @brief      Prints the Ping response on to the RTT console
 * @param[in]  void
 * @retval     None
 **********************************************************************************************************************/
void print_pingResult(void)
{
    APP_PRINT("\r\n \r\nPing Statistics for %s :\r\n",(char *)remote_ip_address);
    APP_PRINT("\r\nPackets: Sent  = %02d, Received = %02d, Lost = %02d \r\n",ping_data.sent,ping_data.received,ping_data.lost);
}

/*******************************************************************************************************************//**
 * @brief      Creates and prints the the IP configuration to display on the RTT console
 * @param[in]  void
 * @retval     None
 **********************************************************************************************************************/
void print_ipconfig(void)
{
#if( ipconfigUSE_DHCP != 0 )
    if(dhcp_in_use)
    {
        ucNetMask[3] = (uint8_t)((xNd.ulNetMask & 0xFF000000) >> 24);
        ucNetMask[2] = (uint8_t)((xNd.ulNetMask & 0x00FF0000) >> 16);
        ucNetMask[1] = (uint8_t)((xNd.ulNetMask & 0x0000FF00) >> 8);
        ucNetMask[0] = (uint8_t)(xNd.ulNetMask & 0x000000FF);

        ucGatewayAddress[3] = (uint8_t)((xNd.ulGatewayAddress & 0xFF000000)>> 24);;
        ucGatewayAddress[2] = (uint8_t)((xNd.ulGatewayAddress & 0x00FF0000)>> 16);
        ucGatewayAddress[1] = (uint8_t)((xNd.ulGatewayAddress & 0x0000FF00)>> 8);
        ucGatewayAddress[0] = (uint8_t)(xNd.ulGatewayAddress & 0x000000FF);

        ucDNSServerAddress[3] = (uint8_t)((xNd.ulDNSServerAddress & 0xFF000000)>> 24);
        ucDNSServerAddress[2] = (uint8_t)((xNd.ulDNSServerAddress & 0x00FF0000)>> 16);
        ucDNSServerAddress[1] = (uint8_t)((xNd.ulDNSServerAddress & 0x0000FF00)>> 8);
        ucDNSServerAddress[0] = (uint8_t)(xNd.ulDNSServerAddress & 0x000000FF);

        ucIPAddress[3] = (uint8_t)((xNd.ulDefaultIPAddress & 0xFF000000) >> 24);
        ucIPAddress[2] = (uint8_t)((xNd.ulDefaultIPAddress & 0x00FF0000) >> 16);
        ucIPAddress[1] = (uint8_t)((xNd.ulDefaultIPAddress & 0x0000FF00) >> 8);
        ucIPAddress[0] = (uint8_t)(xNd.ulDefaultIPAddress & 0x000000FF);
    }
#endif
    APP_PRINT("\r\nEthernet adapter for Renesas "KIT_NAME":\r\n")

    APP_PRINT("\tDescription . . . . . . . . . . . : Renesas "KIT_NAME" Ethernet\r\n");
    APP_PRINT("\tPhysical Address. . . . . . . . . : %02x-%02x-%02x-%02x-%02x-%02x\r\n",
            ucMACAddress[0],ucMACAddress[1],ucMACAddress[2],ucMACAddress[3],ucMACAddress[4],ucMACAddress[5]);
    APP_PRINT("\tDHCP Enabled. . . . . . . . . . . : %s\r\n",dhcp_in_use?"Yes":"No")
    APP_PRINT("\tIPv4 Address. . . . . . . . . . . : %d.%d.%d.%d\r\n",ucIPAddress[0],ucIPAddress[1],ucIPAddress[2],ucIPAddress[3]);
    APP_PRINT("\tSubnet Mask . . . . . . . . . . . : %d.%d.%d.%d\r\n",ucNetMask[0],ucNetMask[1],ucNetMask[2],ucNetMask[3]);
    APP_PRINT("\tDefault Gateway . . . . . . . . . : %d.%d.%d.%d\r\n",ucGatewayAddress[0],ucGatewayAddress[1],ucGatewayAddress[2],ucGatewayAddress[3]);
    APP_PRINT("\tDNS Servers . . . . . . . . . . . : %d.%d.%d.%d\r\n",ucDNSServerAddress[0],ucDNSServerAddress[1],ucDNSServerAddress[2],ucDNSServerAddress[3]);
}


/*******************************************************************************************************************//**
 * @brief      DNS Query for the requested Domain name.  Uses the FreeRTOS Client API  FreeRTOS_gethostbyname
 *             to get the IP address for the domain name
 * @param[in]  Domain name
 * @retval     None
 **********************************************************************************************************************/
void dnsQuerryFunc(char *domain)
{
    uint32_t ulIPAddress = RESET_VALUE;
    int8_t cBuffer[ 16 ] = {RESET_VALUE};

    /* Lookup the IP address of the FreeRTOS.org website. */
    ulIPAddress = FreeRTOS_gethostbyname((char*)domain);

    if( ulIPAddress != 0 )
    {
        /* Convert the IP address to a string. */
        FreeRTOS_inet_ntoa( ulIPAddress, ( char * ) cBuffer);

        /* Print out the IP address obtained from the DNS lookup. */
        APP_PRINT ("\r\nDNS Lookup for \"www.freertos.org\" is      : %s  \r\n", cBuffer);
    }
    else
    {
        APP_PRINT ("\r\nDNS Lookup failed for \"www.freertos.org\" \r\n");
    }
}

/*******************************************************************************************************************//**
 * @brief      This Function checks the Network status (Both Ethernet and IP Layer). If the Network is down
 *             the Application will not send any data on the network.
 * @param[in]  None
 * @retval     Network Status
 **********************************************************************************************************************/
uint32_t isNetworkUp(void)
{
    fsp_err_t  eth_link_status = FSP_ERR_NOT_OPEN;
    BaseType_t networkUp = pdFALSE;
    uint32_t network_status = (IP_LINK_UP | ETHERNET_LINK_UP);

    networkUp = FreeRTOS_IsNetworkUp();
    eth_link_status = R_ETHER_LinkProcess(g_ether0.p_ctrl);

    if((FSP_SUCCESS == eth_link_status) && (pdTRUE == networkUp))
    {
        return network_status;
    }
    else
    {
        if(FSP_SUCCESS != eth_link_status)
        {
            network_status |= ETHERNET_LINK_DOWN;
        }
        else if(FSP_SUCCESS == eth_link_status)
        {
            network_status |= ETHERNET_LINK_UP;
        }

        if(pdTRUE != networkUp)
        {
             network_status |= IP_LINK_DOWN;
        }
        else if(pdTRUE == networkUp)
        {
             network_status |= IP_LINK_UP;
        }
        return network_status;
    }
}



/*******************************************************************************************************************//**
 * @brief      Update the DHCP info to the User data structure.
 * @param[in]  None
 * @retval     None
 **********************************************************************************************************************/
#if( ipconfigUSE_DHCP != 0 )
void updateDhcpResponseToUsr(void)
{
    if(dhcp_in_use)
    {
        memcpy(&xNd, &xNetworkAddressing, sizeof(xNd));
    }
}
#endif
/*******************************************************************************************************************//**
 * @brief      DHCP Hook function to populate the user defined Host name for the Kit.
 * @param[in]  None
 * @retval     Hostname
 **********************************************************************************************************************/
#if( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
const char *pcApplicationHostnameHook(void)
{
    return KIT_NAME;
}
#endif

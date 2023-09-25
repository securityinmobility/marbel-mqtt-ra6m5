/*
 * net_thread_entry.h
 *
 *  Created on: 06.09.2023
 *      Author: Robert
 */

#ifndef NET_THREAD_H_
#define NET_THREAD_H_

#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOSIPConfig.h"

#include "common_utils.h"
#include "user_config.h"
#include "mqtt_helper.h"
#include "transport/transport_interface.h"
#include "backoffAlgorithm/backoff_algorithm.h"

#include "core_mqtt.h"
#include "using_mbedtls_pkcs11.h"
#include "littlefs_app.h"
#include "portmacro.h"

#if( ipconfigUSE_DHCP != 0 )
    #include "FreeRTOS_DHCP.h"
#endif

#define SUCCESS 0
#define PRINT_UP_MSG_DISABLE 0x01
#define PRINT_DOWN_MSG_DISABLE 0x02
#define PRINT_NWK_USR_MSG_DISABLE 0x04

#define ETHERNET_LINK_DOWN 0x01
#define ETHERNET_LINK_UP 0x00
#define IP_LINK_DOWN 0x02
#define IP_LINK_UP 0x00

#if( ipconfigUSE_DHCP != 0 )
    void updateDhcpResponseToUsr(void);
#endif

#if( ipconfigUSE_DHCP != 0 )
    eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress );
#endif

#if( ipconfigDHCP_REGISTER_HOSTNAME == 1 )
    /* DHCP has an option for clients to register their hostname.  It doesn't
    have much use, except that a device can be found in a router along with its
    name. If this option is used the callback below must be provided by the
    application writer to return a const string, denoting the device's name. */
    const char *pcApplicationHostnameHook( void );
#endif /* ipconfigDHCP_REGISTER_HOSTNAME */

uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                             uint16_t usSourcePort,
                                             uint32_t ulDestinationAddress,
                                             uint16_t usDestinationPort );
uint32_t ulongRand(void);
uint32_t isNetworkUp(void);
BaseType_t vSendPing( const char *pcIPAddress);
void print_ipconfig(void);
void print_pingResult(void);
void dnsQuerryFunc(char *domain_name);

typedef struct st_ping_data
{
    uint32_t sent;     // Ping Request
    uint32_t received; // Ping Response
    uint32_t lost;     // Ping failure
} ping_data_t;

#endif /* NET_THREAD_H_ */

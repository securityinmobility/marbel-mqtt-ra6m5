/* generated configuration header file - do not edit */
#ifndef CORE_MQTT_CONFIG_H_
#define CORE_MQTT_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MQTT_MAX_CONNACK_RECEIVE_RETRY_COUNT
#define MQTT_MAX_CONNACK_RECEIVE_RETRY_COUNT            (10)
#endif

#ifndef MQTT_STATE_ARRAY_MAX_COUNT
#define MQTT_STATE_ARRAY_MAX_COUNT                      (10U)
#endif

#ifndef MQTT_PINGRESP_TIMEOUT_MS
#define MQTT_PINGRESP_TIMEOUT_MS                        (500U)
#endif

#ifndef MQTT_RECV_POLLING_TIMEOUT_MS
#define MQTT_RECV_POLLING_TIMEOUT_MS                    (10U)
#endif

#ifndef MQTT_SEND_RETRY_TIMEOUT_MS
#define MQTT_SEND_RETRY_TIMEOUT_MS                      (10U)
#endif

#ifdef __cplusplus
}
#endif
#endif /* CORE_MQTT_CONFIG_H_ */

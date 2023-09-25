/* generated common header file - do not edit */
#ifndef COMMON_DATA_H_
#define COMMON_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "aws_dev_mode_key_provisioning.h"
#include "r_sce_key_injection.h"
#include "r_sce_key_injection_api.h"
#include "r_flash_hp.h"
#include "r_flash_api.h"
#include "rm_littlefs_api.h"
#include "rm_littlefs_flash.h"
#include "lfs_util.h"
#include "lfs_util.h"
#include "mbedtls/platform.h"
#include "psa/crypto.h"
#include "psa/crypto_extra.h"
#include "rm_psa_crypto.h"
#include "r_ether_phy.h"
#include "r_ether_phy_api.h"
#include "r_ether.h"
#include "r_ether_api.h"
#include "FreeRTOSIPConfig.h"
#include "sockets_wrapper.h"
#include "using_mbedtls_pkcs11.h"
#include "core_mqtt.h"
#include "r_ioport.h"
#include "bsp_pin_cfg.h"
FSP_HEADER
/* Flash on Flash HP Instance */
extern const flash_instance_t g_flash0;

/** Access the Flash HP instance using these structures when calling API functions directly (::p_api is not used). */
extern flash_hp_instance_ctrl_t g_flash0_ctrl;
extern const flash_cfg_t g_flash0_cfg;

#ifndef NULL
void NULL(flash_callback_args_t *p_args);
#endif
/** LittleFS on Flash Instance. */
extern const rm_littlefs_instance_t g_rm_littlefs0;
extern rm_littlefs_flash_instance_ctrl_t g_rm_littlefs0_ctrl;
extern const rm_littlefs_cfg_t g_rm_littlefs0_cfg;

extern struct lfs g_rm_littlefs0_lfs;
extern const struct lfs_config g_rm_littlefs0_lfs_cfg;

#ifndef ETHER_PHY_LSI_TYPE_KIT_COMPONENT
#define ETHER_PHY_LSI_TYPE_KIT_COMPONENT ETHER_PHY_LSI_TYPE_DEFAULT
#endif

/** ether_phy on ether_phy Instance. */
extern const ether_phy_instance_t g_ether_phy0;

/** Access the Ethernet PHY instance using these structures when calling API functions directly (::p_api is not used). */
extern ether_phy_instance_ctrl_t g_ether_phy0_ctrl;
extern const ether_phy_cfg_t g_ether_phy0_cfg;
#if (BSP_FEATURE_TZ_HAS_TRUSTZONE == 1) && (BSP_TZ_SECURE_BUILD != 1) && (BSP_TZ_NONSECURE_BUILD != 1) && (BSP_FEATURE_ETHER_SUPPORTS_TZ_SECURE == 0)
#define ETHER_BUFFER_PLACE_IN_SECTION BSP_PLACE_IN_SECTION(".ns_buffer.eth")
#else
#define ETHER_BUFFER_PLACE_IN_SECTION
#endif

/** ether on ether Instance. */
extern const ether_instance_t g_ether0;

/** Access the Ethernet instance using these structures when calling API functions directly (::p_api is not used). */
extern ether_instance_ctrl_t g_ether0_ctrl;
extern const ether_cfg_t g_ether0_cfg;

#ifndef vEtherISRCallback
void vEtherISRCallback(ether_callback_args_t *p_args);
#endif
extern ether_instance_t const *gp_freertos_ether;
/* IOPORT Instance */
extern const ioport_instance_t g_ioport;

/* IOPORT control structure. */
extern ioport_instance_ctrl_t g_ioport_ctrl;
void g_common_init(void);
FSP_FOOTER
#endif /* COMMON_DATA_H_ */

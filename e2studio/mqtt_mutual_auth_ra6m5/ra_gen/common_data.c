/* generated common source file - do not edit */
#include "common_data.h"

#ifndef vLoggingPrintf
#include <stdarg.h>
void vLoggingPrintf(const char *pcFormat, ...);

void vLoggingPrintf(const char *pcFormat, ...)
{
    va_list arg;

    va_start (arg, pcFormat);
    vprintf (pcFormat, arg);
    va_end (arg);
}
#endif

#ifndef vLoggingPrint
void vLoggingPrint(const char *pcFormat);

void vLoggingPrint(const char *pcFormat)
{
    printf ("%s", pcFormat);
}
#endif
flash_hp_instance_ctrl_t g_flash0_ctrl;
const flash_cfg_t g_flash0_cfg =
{ .data_flash_bgo = false, .p_callback = NULL, .p_context = NULL,
#if defined(VECTOR_NUMBER_FCU_FRDYI)
    .irq                 = VECTOR_NUMBER_FCU_FRDYI,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_FCU_FIFERR)
    .err_irq             = VECTOR_NUMBER_FCU_FIFERR,
#else
  .err_irq = FSP_INVALID_VECTOR,
#endif
  .err_ipl = (BSP_IRQ_DISABLED),
  .ipl = (BSP_IRQ_DISABLED), };
/* Instance structure to use this module. */
const flash_instance_t g_flash0 =
{ .p_ctrl = &g_flash0_ctrl, .p_cfg = &g_flash0_cfg, .p_api = &g_flash_on_flash_hp };
rm_littlefs_flash_instance_ctrl_t g_rm_littlefs0_ctrl;

#ifdef LFS_NO_MALLOC
static uint8_t g_rm_littlefs0_read[64];
static uint8_t g_rm_littlefs0_prog[64];
static uint8_t g_rm_littlefs0_lookahead[16];
#endif

struct lfs g_rm_littlefs0_lfs;

const struct lfs_config g_rm_littlefs0_lfs_cfg =
{ .context = &g_rm_littlefs0_ctrl, .read = &rm_littlefs_flash_read, .prog = &rm_littlefs_flash_write, .erase =
          &rm_littlefs_flash_erase,
  .sync = &rm_littlefs_flash_sync, .read_size = 1, .prog_size = 4, .block_size = 128, .block_count =
          (BSP_DATA_FLASH_SIZE_BYTES / 256),
  .block_cycles = 1024, .cache_size = 64, .lookahead_size = 16,
#ifdef LFS_NO_MALLOC
    .read_buffer = (void *) g_rm_littlefs0_read,
    .prog_buffer = (void *) g_rm_littlefs0_prog,
    .lookahead_buffer = (void *) g_rm_littlefs0_lookahead,

#endif
        };

const rm_littlefs_flash_cfg_t g_rm_littlefs0_ext_cfg =
{ .p_flash = &g_flash0, };

const rm_littlefs_cfg_t g_rm_littlefs0_cfg =
{ .p_lfs_cfg = &g_rm_littlefs0_lfs_cfg, .p_extend = &g_rm_littlefs0_ext_cfg, };

/* Instance structure to use this module. */
const rm_littlefs_instance_t g_rm_littlefs0 =
{ .p_ctrl = &g_rm_littlefs0_ctrl, .p_cfg = &g_rm_littlefs0_cfg, .p_api = &g_rm_littlefs_on_flash, };

ether_phy_instance_ctrl_t g_ether_phy0_ctrl;

const ether_phy_cfg_t g_ether_phy0_cfg =
{

.channel = 0,
  .phy_lsi_address = 0, .phy_reset_wait_time = 0x00020000, .mii_bit_access_wait_time = 8, .phy_lsi_type =
          ETHER_PHY_LSI_TYPE_KIT_COMPONENT,
  .flow_control = ETHER_PHY_FLOW_CONTROL_DISABLE, .mii_type = ETHER_PHY_MII_TYPE_RMII, .p_context = NULL, .p_extend =
          NULL,

};
/* Instance structure to use this module. */
const ether_phy_instance_t g_ether_phy0 =
{ .p_ctrl = &g_ether_phy0_ctrl, .p_cfg = &g_ether_phy0_cfg, .p_api = &g_ether_phy_on_ether_phy };
ether_instance_ctrl_t g_ether0_ctrl;

uint8_t g_ether0_mac_address[6] =
{ 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };

__attribute__((__aligned__(16))) ether_instance_descriptor_t g_ether0_tx_descriptors[8] ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(16))) ether_instance_descriptor_t g_ether0_rx_descriptors[8] ETHER_BUFFER_PLACE_IN_SECTION;

__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer0[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer1[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer2[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer3[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer4[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer5[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer6[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer7[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer8[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer9[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer10[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer11[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer12[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer13[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer14[1536]ETHER_BUFFER_PLACE_IN_SECTION;
__attribute__((__aligned__(32)))uint8_t g_ether0_ether_buffer15[1536]ETHER_BUFFER_PLACE_IN_SECTION;

uint8_t *pp_g_ether0_ether_buffers[16] =
{ (uint8_t*) &g_ether0_ether_buffer0[0],
  (uint8_t*) &g_ether0_ether_buffer1[0],
  (uint8_t*) &g_ether0_ether_buffer2[0],
  (uint8_t*) &g_ether0_ether_buffer3[0],
  (uint8_t*) &g_ether0_ether_buffer4[0],
  (uint8_t*) &g_ether0_ether_buffer5[0],
  (uint8_t*) &g_ether0_ether_buffer6[0],
  (uint8_t*) &g_ether0_ether_buffer7[0],
  (uint8_t*) &g_ether0_ether_buffer8[0],
  (uint8_t*) &g_ether0_ether_buffer9[0],
  (uint8_t*) &g_ether0_ether_buffer10[0],
  (uint8_t*) &g_ether0_ether_buffer11[0],
  (uint8_t*) &g_ether0_ether_buffer12[0],
  (uint8_t*) &g_ether0_ether_buffer13[0],
  (uint8_t*) &g_ether0_ether_buffer14[0],
  (uint8_t*) &g_ether0_ether_buffer15[0], };

const ether_extended_cfg_t g_ether0_extended_cfg_t =
{ .p_rx_descriptors = g_ether0_rx_descriptors, .p_tx_descriptors = g_ether0_tx_descriptors, };

const ether_cfg_t g_ether0_cfg =
{ .channel = 0, .zerocopy = ETHER_ZEROCOPY_DISABLE, .multicast = ETHER_MULTICAST_ENABLE, .promiscuous =
          ETHER_PROMISCUOUS_DISABLE,
  .flow_control = ETHER_FLOW_CONTROL_DISABLE, .padding = ETHER_PADDING_DISABLE, .padding_offset = 0, .broadcast_filter =
          0,
  .p_mac_address = g_ether0_mac_address,

  .num_tx_descriptors = 8,
  .num_rx_descriptors = 8,

  .pp_ether_buffers = pp_g_ether0_ether_buffers,

  .ether_buffer_size = 1536,

#if defined(VECTOR_NUMBER_EDMAC0_EINT)
                .irq                = VECTOR_NUMBER_EDMAC0_EINT,
#else
  .irq = FSP_INVALID_VECTOR,
#endif

  .interrupt_priority = (12),

  .p_callback = vEtherISRCallback,
  .p_ether_phy_instance = &g_ether_phy0, .p_context = NULL, .p_extend = &g_ether0_extended_cfg_t, };

/* Instance structure to use this module. */
const ether_instance_t g_ether0 =
{ .p_ctrl = &g_ether0_ctrl, .p_cfg = &g_ether0_cfg, .p_api = &g_ether_on_ether };
ether_instance_t const *gp_freertos_ether = &g_ether0;
ioport_instance_ctrl_t g_ioport_ctrl;
const ioport_instance_t g_ioport =
{ .p_api = &g_ioport_on_ioport, .p_ctrl = &g_ioport_ctrl, .p_cfg = &g_bsp_pin_cfg, };

void g_common_init(void)
{
}

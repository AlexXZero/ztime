#include "bluetooth.h"      // for bluetooth_init()

#include <zephyr.h>
#include <power/reboot.h>   // for sys_reboot()

#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
#include "os_mgmt/os_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
#include "img_mgmt/img_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
#include "stat_mgmt/stat_mgmt.h"
#endif
#ifdef CONFIG_MCUMGR_CMD_SHELL_MGMT
#include "shell_mgmt/shell_mgmt.h"
#endif

#include <logging/log.h>
LOG_MODULE_REGISTER(ztime, LOG_LEVEL_INF);


static bool wdt_is_enabled(void)
{
  return NRF_WDT->RUNSTATUS;
}

static void wdt_feed(void)
{
  NRF_WDT->RR[0] = WDT_RR_RR_Reload;
}

void main(void)
{
#ifdef CONFIG_MCUMGR_CMD_OS_MGMT
  os_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_IMG_MGMT
  img_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_STAT_MGMT
  stat_mgmt_register_group();
#endif
#ifdef CONFIG_MCUMGR_CMD_SHELL_MGMT
  shell_mgmt_register_group();
#endif

#ifdef CONFIG_BT
  bluetooth_init();
#endif

  /* The system work queue handles all incoming mcumgr requests.  Let the
   * main thread idle while the mcumgr server runs.
   */
  for (size_t i = 0; i < 120; i++) { // 2 minutes
    if (wdt_is_enabled()) {
      wdt_feed();
    }
    //k_cpu_idle();
    k_msleep(1000); // should be less 7 seconds to avoid WDT reboot
  }

  sys_reboot(SYS_REBOOT_COLD);
}

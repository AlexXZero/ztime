#include "ble/ble.h"

#include <zephyr.h>
#include <stddef.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>

#ifdef CONFIG_MCUMGR_SMP_BT
#include <mgmt/mcumgr/smp_bt.h>
#endif

#include <logging/log.h>
LOG_MODULE_REGISTER(ztime_ble, LOG_LEVEL_INF);


static struct k_work advertise_work;

static const struct bt_data ad[] = {
  BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
  BT_DATA_BYTES(BT_DATA_UUID128_ALL,
          0x84, 0xaa, 0x60, 0x74, 0x52, 0x8a, 0x8b, 0x86,
          0xd3, 0x4c, 0xb7, 0x1d, 0x1d, 0xdc, 0x53, 0x8d),
};

static void advertise(struct k_work *work)
{
  (void)bt_le_adv_stop();

  int ret = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
  if (ret < 0) {
    LOG_ERR("Advertising failed to start (error %d)", ret);
    return;
  }

  LOG_INF("Advertising successfully started");
}

void ble_init(void)
{
  k_work_init(&advertise_work, advertise);

  /* Enable Bluetooth. */
  int ret = bt_enable(NULL);
  if (ret < 0) {
    LOG_ERR("Bluetooth init failed (error %d)", ret);
    return;
  }

  LOG_INF("Bluetooth initialized");
  k_work_submit(&advertise_work);

#ifdef CONFIG_MCUMGR_SMP_BT
  /* Initialize the Bluetooth mcumgr transport. */
  smp_bt_register();
#endif
}

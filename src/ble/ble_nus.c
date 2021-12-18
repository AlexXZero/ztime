#include "ble/ble_nus.h"

#include <stdint.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <bluetooth/services/bas.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ztime_ble_nus, LOG_LEVEL_INF);


static struct bt_uuid_128 bt_uuid_nus_base =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x6E400001, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E));

static struct bt_uuid_128 bt_uuid_nus_rx =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x6E400002, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E));

static struct bt_uuid_128 bt_uuid_nus_tx =
    BT_UUID_INIT_128(BT_UUID_128_ENCODE(0x6E400003, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E));


static ssize_t ble_nus_write_handler(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                                     const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
  // TODO
  return len;
}

BT_GATT_SERVICE_DEFINE(ble_nus_svc,
    BT_GATT_PRIMARY_SERVICE(&bt_uuid_nus_base.uuid),

    /* BLE_NUS tx */
    BT_GATT_CHARACTERISTIC(&bt_uuid_nus_tx.uuid,
                           BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_NONE,
                           NULL, NULL, NULL),

    /* BLE_NUS rx */
    BT_GATT_CHARACTERISTIC(&bt_uuid_nus_rx.uuid,
                           BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                           BT_GATT_PERM_WRITE,
                           NULL, ble_nus_write_handler, NULL),
);



static struct bt_conn *ble_conn = NULL;

static void ble_connected(struct bt_conn *conn, uint8_t err)
{
  if (err != 0) {
    LOG_WRN("Connection failed (err 0x%02x)", err);
    return;
  }

  ble_conn = conn;
}

static void ble_disconnected(struct bt_conn *conn, uint8_t reason)
{
  LOG_INF("Disconnected (reason 0x%02x)", reason);
  ble_conn = NULL;
}

BT_CONN_CB_DEFINE(ble_nus_conn_callbacks) = {
  .connected = ble_connected,
  .disconnected = ble_disconnected,
};


bool ble_nus_available(void)
{
  return (ble_conn != NULL);
}

int ble_nus_read(void *buffer, uint16_t size)
{
  return -ENOTSUP; // TODO
}

int ble_nus_write(const void *data, uint16_t length)
{
  if (!ble_nus_available()) {
    return -EAGAIN;
  }

  int ret = bt_gatt_notify(ble_conn, &ble_nus_svc.attrs[2], data, length);
  if (ret < 0) {
    return ret;
  }

  return length;
}

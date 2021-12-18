#ifdef CONFIG_LOG_BACKEND_BLE_NUS

#include "ble/ble_nus.h"  // for ble_nus_*()

#include <zephyr.h>
#include <stddef.h>

#include <logging/log_backend.h>
#include <logging/log_core.h>
#include <logging/log_msg.h>
#include <logging/log_output.h>
#include <logging/log_output_dict.h>
#include <logging/log_backend_std.h>
#include <sys/__assert.h>
#include <sys/ring_buffer.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ztime_log_backend_ble, LOG_LEVEL_INF);

#define BLE_NUS_PACKET_SIZE (20)


static struct k_thread ble_log_thread_data;
RING_BUF_DECLARE(ble_log_buffer, CONFIG_LOG_BACKEND_BLE_NUS_BUFFER_SIZE);
K_SEM_DEFINE(ble_log_buffer_sem, 0, 1);
K_THREAD_STACK_DEFINE(ble_log_stack_area, CONFIG_LOG_BACKEND_BLE_NUS_THREAD_STACKSIZE);

static void ble_log_thread(void *unused1, void *unused2, void *unused3)
{
  uint8_t data[BLE_NUS_PACKET_SIZE];
  while (1) {
    k_sem_take(&ble_log_buffer_sem, K_FOREVER);
    unsigned int key = irq_lock();
    uint32_t length = ring_buf_get(&ble_log_buffer, data, sizeof(data));
    irq_unlock(key);
    while (length > 0) { // should always be true
      while (!ble_nus_available()) { // wait connection
        while (!ble_nus_available()) {
          k_msleep(20);
        }
        k_msleep(2000); // wait 2 secs for establishment connection
      }
      (void)ble_nus_write(data, length);
      k_msleep(50); // minimum 50 ms delay between bluetooth packets
      unsigned int key = irq_lock();
      length = ring_buf_get(&ble_log_buffer, data, sizeof(data));
      irq_unlock(key);
    }
  }
}

static int ble_out(uint8_t *data, size_t length, void *ctx)
{
  unsigned int key = irq_lock();
  uint32_t result = ring_buf_put(&ble_log_buffer, data, length);
  irq_unlock(key);
  k_sem_give(&ble_log_buffer_sem);
  return result;
}

static uint8_t ble_output_buf[BLE_NUS_PACKET_SIZE];

LOG_OUTPUT_DEFINE(log_output_ble, ble_out, ble_output_buf, sizeof(ble_output_buf));

static void put(const struct log_backend *const backend,
    struct log_msg *msg)
{
  uint32_t flag = IS_ENABLED(CONFIG_LOG_BACKEND_BLE_NUS_SYST_ENABLE) ?
    LOG_OUTPUT_FLAG_FORMAT_SYST : 0;

  log_backend_std_put(&log_output_ble, flag, msg);
}

static void process(const struct log_backend *const backend,
    union log_msg2_generic *msg)
{
  uint32_t flags = log_backend_std_get_flags();

  log_output_msg2_process(&log_output_ble, &msg->log, flags);
}

static void log_backend_ble_init(struct log_backend const *const backend)
{
  k_thread_create(&ble_log_thread_data, ble_log_stack_area,
                  K_THREAD_STACK_SIZEOF(ble_log_stack_area),
                  ble_log_thread, NULL, NULL, NULL,
                  CONFIG_LOG_BACKEND_BLE_NUS_THREAD_PRIORITY, 0, K_NO_WAIT);
  k_thread_name_set(&ble_log_thread_data, "ble_log");
}

static void panic(struct log_backend const *const backend)
{
  log_backend_std_panic(&log_output_ble);
}

static void dropped(const struct log_backend *const backend, uint32_t cnt)
{
  ARG_UNUSED(backend);

  log_backend_std_dropped(&log_output_ble, cnt);
}

static void sync_string(const struct log_backend *const backend,
         struct log_msg_ids src_level, uint32_t timestamp,
         const char *fmt, va_list ap)
{
  uint32_t flag = IS_ENABLED(CONFIG_LOG_BACKEND_BLE_NUS_SYST_ENABLE) ?
    LOG_OUTPUT_FLAG_FORMAT_SYST : 0;

  log_backend_std_sync_string(&log_output_ble, flag, src_level,
            timestamp, fmt, ap);
}

static void sync_hexdump(const struct log_backend *const backend,
       struct log_msg_ids src_level, uint32_t timestamp,
       const char *metadata, const uint8_t *data, uint32_t length)
{
  uint32_t flag = IS_ENABLED(CONFIG_LOG_BACKEND_BLE_NUS_SYST_ENABLE) ?
    LOG_OUTPUT_FLAG_FORMAT_SYST : 0;

  log_backend_std_sync_hexdump(&log_output_ble, flag, src_level,
             timestamp, metadata, data, length);
}

const struct log_backend_api log_backend_ble_api = {
  .process = IS_ENABLED(CONFIG_LOG2) ? process : NULL,
  .put = IS_ENABLED(CONFIG_LOG_MODE_DEFERRED) ? put : NULL,
  .put_sync_string = IS_ENABLED(CONFIG_LOG_MODE_IMMEDIATE) ?
      sync_string : NULL,
  .put_sync_hexdump = IS_ENABLED(CONFIG_LOG_MODE_IMMEDIATE) ?
      sync_hexdump : NULL,
  .panic = panic,
  .init = log_backend_ble_init,
  .dropped = IS_ENABLED(CONFIG_LOG_IMMEDIATE) ? NULL : dropped,
};

LOG_BACKEND_DEFINE(log_backend_ble, log_backend_ble_api, true);

#endif // CONFIG_LOG_BACKEND_BLE_NUS

#ifndef ZTIME_INCLUDE_BLE_NUS_H
#define ZTIME_INCLUDE_BLE_NUS_H

#include <stdint.h>
#include <stdbool.h>

bool ble_nus_available(void);
int ble_nus_read(void *buffer, uint16_t size);
int ble_nus_write(const void *data, uint16_t length);

#endif /* ZTIME_INCLUDE_BLE_NUS_H */

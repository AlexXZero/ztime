#ifndef ZTIME_INCLUDE_BUTTON_INTERNAL_H
#define ZTIME_INCLUDE_BUTTON_INTERNAL_H

#include "button.h"
#include <stdint.h>       // for int64_t
#include <zephyr.h>       // for struct k_timer
#include <drivers/gpio.h> // for struct gpio_dt_spec


typedef enum {
  BUTTON_STATE_IDLE,
  BUTTON_STATE_PRESSED,
#if (CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS > 0)
  BUTTON_STATE_RELEASED,
  BUTTON_STATE_DOUBLE_PRESSED,
#endif
#if (CONFIG_BUTTON_HOLD_DURATION_MS > 0)
  BUTTON_STATE_HELD,
#endif
} button_state_t;

typedef struct {
  button_handler_t click_handler;
  void *click_handler_arg;
#if (CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS > 0)
  button_handler_t double_click_handler;
  void *double_click_handler_arg;
#endif
#if (CONFIG_BUTTON_HOLD_DURATION_MS > 0)
  button_handler_t hold_handler;
  void *hold_handler_arg;
#endif

  const struct gpio_dt_spec in;
  const struct gpio_dt_spec out;
  struct gpio_callback cb;
  button_state_t state;
  struct k_timer timer;
#if (CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS > 0) || (CONFIG_BUTTON_HOLD_DURATION_MS > 0)
  int64_t reftime_ms;
#endif
} button_t;

#define BUTTON_DEFAULT_CFG {                              \
  .in = GPIO_DT_SPEC_GET(DT_NODELABEL(key_in), gpios),    \
  .out = GPIO_DT_SPEC_GET(DT_NODELABEL(key_out), gpios),  \
}

#endif /* ZTIME_INCLUDE_BUTTON_INTERNAL_H */

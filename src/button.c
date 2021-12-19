#ifdef CONFIG_ZTIME_BUTTON

#include "button.h"
#include "button_internal.h"

#include <assert.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(ztime_button, LOG_LEVEL_INF);


static button_t button = BUTTON_DEFAULT_CFG;

void button_set_click_handler(button_handler_t handler, void *arg)
{
  unsigned int key = irq_lock();
  button.click_handler = handler;
  button.click_handler_arg = arg;
  irq_unlock(key);
}

#if (CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS > 0)
void button_set_double_click_handler(button_handler_t handler, void *arg)
{
  unsigned int key = irq_lock();
  button.double_click_handler = handler;
  button.double_click_handler_arg = arg;
  irq_unlock(key);
}
#endif

#if (CONFIG_BUTTON_HOLD_DURATION_MS > 0)
void button_set_hold_handler(button_handler_t handler, void *arg)
{
  unsigned int key = irq_lock();
  button.hold_handler = handler;
  button.hold_handler_arg = arg;
  irq_unlock(key);
}
#endif

static void button_update_sm(struct k_timer *timer)
{
#if (CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS > 0) || (CONFIG_BUTTON_HOLD_DURATION_MS > 0)
  int64_t delta_ms = k_uptime_delta(&button.reftime_ms);
#endif
  bool is_pressed = gpio_pin_get_dt(&button.in);
  button_handler_t handler = NULL;
  void *handler_arg = NULL;

  unsigned int key = irq_lock();

  switch (button.state) {
  case BUTTON_STATE_IDLE:
    if (is_pressed) {
      LOG_DBG("Button is pressed");
      button.state = BUTTON_STATE_PRESSED;
#if (CONFIG_BUTTON_HOLD_DURATION_MS > 0)
      k_timer_start(&button.timer, K_MSEC(CONFIG_BUTTON_HOLD_DURATION_MS), K_NO_WAIT);
#endif
    } else {
      LOG_WRN("Unexpected button state: IDLE");
      button.state = BUTTON_STATE_IDLE;
    }
    break;

  case BUTTON_STATE_PRESSED:
    if (!is_pressed) {
      LOG_DBG("Button is released");
#if (CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS > 0)
      button.state = BUTTON_STATE_RELEASED;
      k_timer_start(&button.timer, K_MSEC(CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS), K_NO_WAIT);
#else
      button.state = BUTTON_STATE_IDLE;
      handler = button.click_handler;
      handler_arg = button.click_handler_arg;
#endif
#if (CONFIG_BUTTON_HOLD_DURATION_MS > 0)
    } else if (is_pressed && (delta_ms >= CONFIG_BUTTON_HOLD_DURATION_MS)) {
      LOG_DBG("Button is held");
      button.state = BUTTON_STATE_HELD;
      handler = button.hold_handler;
      handler_arg = button.hold_handler_arg;
#endif
    } else {
      LOG_WRN("Unexpected button state: PRESSED");
      button.state = BUTTON_STATE_IDLE;
    }
    break;

#if (CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS > 0)
  case BUTTON_STATE_RELEASED:
    if (!is_pressed && (delta_ms >= CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS)) {
      LOG_DBG("Button click is proved");
      button.state = BUTTON_STATE_IDLE;
      handler = button.click_handler;
      handler_arg = button.click_handler_arg;
    } else if (is_pressed && (delta_ms < CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS)) {
      LOG_DBG("Button is double pressed");
      button.state = BUTTON_STATE_DOUBLE_PRESSED;
      handler = button.double_click_handler;
      handler_arg = button.double_click_handler_arg;
      k_timer_stop(&button.timer);
    } else {
      LOG_WRN("Unexpected button state: RELEASED");
      button.state = BUTTON_STATE_IDLE;
    }
    break;

  case BUTTON_STATE_DOUBLE_PRESSED:
    if (!is_pressed) {
      LOG_DBG("Button is released*");
      button.state = BUTTON_STATE_IDLE;
    } else {
      LOG_WRN("Unexpected button state: DOUBLE_PRESSED");
      button.state = BUTTON_STATE_IDLE;
    }
    break;
#endif

#if (CONFIG_BUTTON_HOLD_DURATION_MS > 0)
  case BUTTON_STATE_HELD:
    if (!is_pressed) {
      LOG_DBG("Button is released*");
      button.state = BUTTON_STATE_IDLE;
    } else {
      LOG_WRN("Unexpected button state: HELD");
      button.state = BUTTON_STATE_IDLE;
    }
    break;
#endif

  default:
    LOG_ERR("Unexpected button state: %d", button.state);
    button.state = BUTTON_STATE_IDLE;
    break;
  }

  irq_unlock(key);

  if (handler != NULL) {
    (*handler)(handler_arg);
  }
}

static void button_irq_handler(const struct device *gpiobtn, struct gpio_callback *cb, uint32_t pins)
{
#if (CONFIG_BUTTON_DEBOUNCE_DELAY_MS > 0)
  // Here we use a timer to be able to read the stable state of the button after debounce delay
  k_timer_start(&button.timer, K_MSEC(CONFIG_BUTTON_DEBOUNCE_DELAY_MS), K_NO_WAIT);
#else
  button_update_sm(&button.timer);
#endif
}

static int button_init(const struct device *dev)
{
  k_timer_init(&button.timer, button_update_sm, NULL);

  (void)gpio_pin_configure_dt(&button.in, GPIO_INPUT);
  (void)gpio_pin_configure_dt(&button.out, GPIO_OUTPUT_ACTIVE);

  (void)gpio_pin_interrupt_configure_dt(&button.in, GPIO_INT_EDGE_BOTH);

  gpio_init_callback(&button.cb, &button_irq_handler, BIT(button.in.pin));
  (void)gpio_add_callback(button.in.port, &button.cb);

  return 0;
}

SYS_INIT(button_init, POST_KERNEL, CONFIG_BUTTON_INIT_PRIORITY);

#endif // CONFIG_ZTIME_BUTTON

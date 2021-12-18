#ifndef ZTIME_INCLUDE_BUTTON_H
#define ZTIME_INCLUDE_BUTTON_H

typedef void (*button_handler_t)(void *);

void button_set_click_handler(button_handler_t handler, void *arg);

#if (CONFIG_BUTTON_DOUBLE_CLICK_DURATION_MS > 0)
void button_set_double_click_handler(button_handler_t handler, void *arg);
#endif

#if (CONFIG_BUTTON_HOLD_DURATION_MS > 0)
void button_set_hold_handler(button_handler_t handler, void *arg);
#endif

#endif /* ZTIME_INCLUDE_BUTTON_H */

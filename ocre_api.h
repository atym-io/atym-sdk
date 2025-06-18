#ifndef OCRE_API_H
#define OCRE_API_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int32_t type;
    int32_t id;
    int32_t port;
    int32_t state;
} event_data_t;

typedef enum
{
    OCRE_RESOURCE_TYPE_TIMER,
    OCRE_RESOURCE_TYPE_GPIO,
    OCRE_RESOURCE_TYPE_SENSOR,
    OCRE_RESOURCE_TYPE_COUNT
} ocre_resource_type_t;

// Timer API
int ocre_timer_create(int id);
int ocre_timer_delete(int id);
int ocre_timer_start(int id, int interval, int is_periodic);
int ocre_timer_stop(int id);
int ocre_timer_get_remaining(int id);

// GPIO API
typedef enum
{
    OCRE_GPIO_DIR_INPUT,
    OCRE_GPIO_DIR_OUTPUT
} ocre_gpio_direction_t;

typedef enum
{
    OCRE_GPIO_PIN_RESET = 0,
    OCRE_GPIO_PIN_SET = 1
} ocre_gpio_pin_state_t;

int ocre_gpio_init(void);
int ocre_gpio_configure(int port, int pin, int direction);
int ocre_gpio_pin_set(int port, int pin, ocre_gpio_pin_state_t state);
ocre_gpio_pin_state_t ocre_gpio_pin_get(int port, int pin);
int ocre_gpio_pin_toggle(int port, int pin);
int ocre_gpio_register_callback(int port, int pin);
int ocre_gpio_unregister_callback(int port, int pin);

// Event API

typedef void (*timer_callback_func_t)(void);
typedef void (*gpio_callback_func_t)(void);

int ocre_get_event(uint32_t type_offset, uint32_t id_offset, uint32_t port_offset,
                   uint32_t state_offset);
void ocre_poll_events(void);
int ocre_unregister_gpio_callback(int pin, int port);
int ocre_unregister_timer_callback(int timer_id);
int ocre_register_gpio_callback(int pin, int port, gpio_callback_func_t callback);
int ocre_register_timer_callback(int timer_id, timer_callback_func_t callback);

// Utility API
int ocre_sleep(int milliseconds);
#define ocre_pause() ocre_sleep(9999999)

// Sensor API
typedef int ocre_sensor_handle_t;
int ocre_sensors_init(void);
int ocre_sensors_discover(void);
int ocre_sensors_open(ocre_sensor_handle_t handle);
int ocre_sensors_get_handle(int sensor_id);
int ocre_sensors_get_channel_count(int sensor_id);
int ocre_sensors_get_channel_type(int sensor_id, int channel_index);
int ocre_sensors_read(int sensor_id, int channel_type);
int ocre_sensors_get_handle_by_name(const char *name, ocre_sensor_handle_t *handle);
int ocre_sensors_open_by_name(const char *name, ocre_sensor_handle_t *handle);

int ocre_register_dispatcher(ocre_resource_type_t type, const char *function_name);
#endif

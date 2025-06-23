/*
 * Copyright (C) 2025 Atym Incorporated. All rights reserved.
 */
#ifndef OCRE_SDK_H
#define OCRE_SDK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Forward declarations for WASM types (MOVE THIS HERE)
struct WASMModuleInstance;
typedef struct WASMModuleInstance *wasm_module_inst_t;

#ifdef __cplusplus
extern "C"
{
#endif

// For exported callback functions (optional - only needed for WASM callbacks)
#define OCRE_EXPORT(name) __attribute__((export_name(name)))

// OCRE SDK Version Information
#define OCRE_SDK_VERSION_MAJOR  1
#define OCRE_SDK_VERSION_MINOR  0
#define OCRE_SDK_VERSION_PATCH  0
#define OCRE_SDK_VERSION        "1.0.0"

// Common Return Codes
#define OCRE_SUCCESS            0
#define OCRE_ERROR_INVALID      -1
#define OCRE_ERROR_TIMEOUT      -2
#define OCRE_ERROR_NOT_FOUND    -3
#define OCRE_ERROR_BUSY         -4
#define OCRE_ERROR_NO_MEMORY    -5

// Configuration
#define OCRE_MAX_TIMERS         16
#define OCRE_MAX_SENSORS        32
#define OCRE_MAX_CALLBACKS      64
#define OCRE_MAX_TOPIC_LEN      128
#define OCRE_MAX_PAYLOAD_LEN    1024
#define CONFIG_MAX_SENSOR_NAME_LENGTH 125

#define OCRE_API_POSIX_BUF_SIZE 65

// GPIO Configuration
#ifndef CONFIG_OCRE_GPIO_MAX_PINS
#define CONFIG_OCRE_GPIO_MAX_PINS 32
#endif

#ifndef CONFIG_OCRE_GPIO_MAX_PORTS
#define CONFIG_OCRE_GPIO_MAX_PORTS 8
#endif

#ifndef CONFIG_OCRE_GPIO_PINS_PER_PORT
#define CONFIG_OCRE_GPIO_PINS_PER_PORT 16
#endif

    // Internal state tracking
    typedef struct
    {
        bool initialized;
        uint32_t active_timers;
        uint32_t active_sensors;
    } ocre_sdk_state_t;

    /**
     * Structure for event data
     */
    typedef struct
    {
        int32_t type;  /**< Resource type (e.g., OCRE_RESOURCE_TYPE_*) */
        int32_t id;    /**< Resource ID */
        int32_t port;  /**< Port number (for GPIO) */
        int32_t state; /**< State (e.g., pin state for GPIO) */
    } event_data_t;

    // =============================================================================
    // Resource Types
    // =============================================================================

    /**
     * Enum representing different resource types
     */
    typedef enum
    {
        OCRE_RESOURCE_TYPE_TIMER,
        OCRE_RESOURCE_TYPE_GPIO,
        OCRE_RESOURCE_TYPE_SENSOR,
        OCRE_RESOURCE_TYPE_COUNT
    } ocre_resource_type_t;

    // =============================================================================
    // Timer API
    // =============================================================================

    /**
     * Create a timer with specified ID
     * @param id Timer identifier (must be between 1 and MAX_TIMERS)
     * @return 0 on success, -1 on error
     */
    int ocre_timer_create(int id);

    /**
     * Delete a timer
     * @param id Timer identifier
     * @return 0 on success, -1 on error
     */
    int ocre_timer_delete(int id);

    /**
     * Start a timer
     * @param id Timer identifier
     * @param interval Timer interval in milliseconds
     * @param is_periodic True for periodic timer, false for one-shot
     * @return 0 on success, -1 on error
     */
    int ocre_timer_start(int id, int interval, int is_periodic);

    /**
     * Stop a timer
     * @param id Timer identifier
     * @return 0 on success, -1 on error
     */
    int ocre_timer_stop(int id);

    /**
     * Get remaining time for a timer
     * @param id Timer identifier
     * @return Remaining time in milliseconds, or -1 on error
     */
    int ocre_timer_get_remaining(int id);

    // =============================================================================
    // GPIO API
    // =============================================================================

    /**
     * GPIO pin state
     */
    typedef enum
    {
        OCRE_GPIO_PIN_RESET = 0,
        OCRE_GPIO_PIN_SET = 1
    } ocre_gpio_pin_state_t;

    /**
     * GPIO pin direction
     */
    typedef enum
    {
        OCRE_GPIO_DIR_INPUT,
        OCRE_GPIO_DIR_OUTPUT
    } ocre_gpio_direction_t;

    /**
     * Get GPIO pin state
     * @param port GPIO port number
     * @param pin GPIO pin number
     * @return Pin state or negative error code
     */
    ocre_gpio_pin_state_t ocre_gpio_pin_get(int port, int pin);

    /**
     * Initialize GPIO subsystem
     * @return 0 on success, negative error code on failure
     */
    int ocre_gpio_init(void);

    /**
     * Configure a GPIO pin
     * @param port GPIO port number
     * @param pin GPIO pin number
     * @param direction Pin direction (input/output)
     * @return 0 on success, negative error code on failure
     */
    int ocre_gpio_configure(int port, int pin, int direction);

    /**
     * Set GPIO pin state
     * @param port GPIO port number
     * @param pin GPIO pin number
     * @param state Desired pin state
     * @return 0 on success, negative error code on failure
     */
    int ocre_gpio_pin_set(int port, int pin, ocre_gpio_pin_state_t state);

    /**
     * Toggle GPIO pin state
     * @param port GPIO port number
     * @param pin GPIO pin number
     * @return 0 on success, negative error code on failure
     */
    int ocre_gpio_pin_toggle(int port, int pin);

    /**
     * Register callback for GPIO pin state changes
     * @param port GPIO port number
     * @param pin GPIO pin number
     * @return 0 on success, negative error code on failure
     */
    int ocre_gpio_register_callback(int port, int pin);

    /**
     * Unregister GPIO pin callback
     * @param port GPIO port number
     * @param pin GPIO pin number
     * @return 0 on success, negative error code on failure
     */
    int ocre_gpio_unregister_callback(int port, int pin);

    // =============================================================================
    // Event API
    // =============================================================================

    /**
     * Timer callback function type
     */
    typedef void (*timer_callback_func_t)(void);

    /**
     * GPIO callback function type
     */
    typedef void (*gpio_callback_func_t)(void);

    /**
     * Get event data for a specific resource
     * @param type_offset Offset for resource type
     * @param id_offset Offset for resource ID
     * @param port_offset Offset for port number
     * @param state_offset Offset for state
     * @return OCRE_SUCCESS on success, negative error code on failure
     */
    int ocre_get_event(uint32_t type_offset, uint32_t id_offset, uint32_t port_offset,
                       uint32_t state_offset);

    /**
     * Poll for events
     */
    void ocre_poll_events(void);

    /**
     * Unregister GPIO callback
     * @param pin GPIO pin number
     * @param port GPIO port number
     * @return 0 on success, negative error code on failure
     */
    int ocre_unregister_gpio_callback(int pin, int port);

    /**
     * Unregister timer callback
     * @param timer_id Timer identifier
     * @return 0 on success, negative error code on failure
     */
    int ocre_unregister_timer_callback(int timer_id);

    /**
     * Register GPIO callback
     * @param pin GPIO pin number
     * @param port GPIO port number
     * @param callback Callback function to register
     * @return 0 on success, negative error code on failure
     */
    int ocre_register_gpio_callback(int pin, int port, gpio_callback_func_t callback);

    /**
     * Register timer callback
     * @param timer_id Timer identifier
     * @param callback Callback function to register
     * @return 0 on success, negative error code on failure
     */
    int ocre_register_timer_callback(int timer_id, timer_callback_func_t callback);

    // =============================================================================
    // Utility API
    // =============================================================================

    /**
     * Sleep for specified duration
     * @param milliseconds Sleep duration in milliseconds
     * @return OCRE_SUCCESS on success, error code on failure
     */
    int ocre_sleep(int milliseconds);

/**
 * Pause execution indefinitely (implementation-specific)
 * @return OCRE_SUCCESS on success, error code on failure
 */
#define ocre_pause() ocre_sleep(9999999)
  
    // =============================================================================
    // Sensor API
    // =============================================================================

    /**
     * Sensor handle type
     */
    typedef int ocre_sensor_handle_t;

    /**
     * Initialize the sensor system
     * @return 0 on success
     */
    int ocre_sensors_init(void);

    /**
     * Discover available sensors
     * @return Number of discovered sensors, negative error code on failure
     */
    int ocre_sensors_discover(void);

    /**
     * Open a sensor for use
     * @param handle Handle of the sensor to open
     * @return 0 on success, negative error code on failure
     */
    int ocre_sensors_open(ocre_sensor_handle_t handle);

    /**
     * Get the handle of a sensor
     * @param sensor_id ID of the sensor
     * @return Sensor handle on success, negative error code on failure
     */
    int ocre_sensors_get_handle(int sensor_id);

    /**
     * Get the number of channels available in a sensor
     * @param sensor_id ID of the sensor
     * @return Number of channels on success, negative error code on failure
     */
    int ocre_sensors_get_channel_count(int sensor_id);

    /**
     * Get the type of a specific sensor channel
     * @param sensor_id ID of the sensor
     * @param channel_index Index of the channel
     * @return Channel type on success, negative error code on failure
     */
    int ocre_sensors_get_channel_type(int sensor_id, int channel_index);

    /**
     * Read data from a sensor channel
     * @param sensor_id ID of the sensor
     * @param channel_type Type of the channel to read
     * @return Sensor value in integer format, negative error code on failure
     */
    int ocre_sensors_read(int sensor_id, int channel_type);

    /**
     * Get the handle of a sensor by name
     * @param name Name of the sensor
     * @param handle Pointer to store the sensor handle
     * @return OCRE_SUCCESS on success, negative error code on failure
     */
    int ocre_sensors_get_handle_by_name(const char *name, ocre_sensor_handle_t *handle);

    /**
     * Open a sensor by name
     * @param name Name of the sensor
     * @param handle Pointer to store the sensor handle
     * @return OCRE_SUCCESS on success, negative error code on failure
     */
    int ocre_sensors_open_by_name(const char *name, ocre_sensor_handle_t *handle);

    /**
     * Register a dispatcher for a resource type
     * @param type Resource type to register the dispatcher for
     * @param function_name Name of the callback function
     * @return OCRE_SUCCESS on success, negative error code on failure
     */
    int ocre_register_dispatcher(ocre_resource_type_t type, const char *function_name);

    /**
     * Structure for system information
     */
    struct _ocre_posix_utsname
    {
        char sysname[OCRE_API_POSIX_BUF_SIZE];    /**< System name */
        char nodename[OCRE_API_POSIX_BUF_SIZE];   /**< Node name */
        char release[OCRE_API_POSIX_BUF_SIZE];    /**< Release version */
        char version[OCRE_API_POSIX_BUF_SIZE];    /**< Version string */
        char machine[OCRE_API_POSIX_BUF_SIZE];    /**< Machine type */
        char domainname[OCRE_API_POSIX_BUF_SIZE]; /**< Domain name */
    };

    /**
     * Get system information
     * @param name Buffer to receive system information
     * @return 0 on success, -1 on failure
     */
    int uname(struct _ocre_posix_utsname *name);

#ifdef __cplusplus
}
#endif
#endif
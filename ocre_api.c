/*
 * Copyright (C) 2025 Atym Incorporated. All rights reserved.
 */
#include "ocre_api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CALLBACKS 16
#define BUTTON_PORT 2

static void (*timer_callbacks[MAX_CALLBACKS])(void) = {0};
static void (*gpio_callbacks[MAX_CALLBACKS])(void) = {0};
static int gpio_callback_pins[MAX_CALLBACKS] = {-1};
static int gpio_callback_ports[MAX_CALLBACKS] = {-1};

// Initialize callback pins to invalid values
static void init_callback_system(void)
{
    static bool initialized = false;
    if (!initialized)
    {
        for (int i = 0; i < MAX_CALLBACKS; i++)
        {
            gpio_callback_pins[i] = -1;
            gpio_callback_ports[i] = -1; // Initialize ports to invalid
        }
        initialized = true;
    }
}

// =============================================================================
// INTERNAL CALLBACK DISPATCHERS
// =============================================================================

__attribute__((export_name("timer_callback"))) void timer_callback(int timer_id)
{
    init_callback_system();

    if (timer_id >= 0 && timer_id < MAX_CALLBACKS && timer_callbacks[timer_id])
    {
        printf("Executing timer callback for ID: %d\n", timer_id);
        timer_callbacks[timer_id]();
    }
    else
    {
        printf("No timer callback registered for ID: %d\n", timer_id);
    }
}

__attribute__((export_name("gpio_callback"))) void gpio_callback(int pin, int state, int port)
{
    init_callback_system();

    printf("GPIO event triggered: pin=%d, port=%d, state=%d\n", pin, port, state);

    for (int i = 0; i < MAX_CALLBACKS; i++)
    {
        if (gpio_callback_pins[i] == pin && gpio_callback_ports[i] == port && gpio_callbacks[i])
        {
            printf("Executing GPIO callback for pin: %d, port: %d\n", pin, port);
            gpio_callbacks[i]();
            return;
        }
    }

    printf("No GPIO callback registered for pin: %d, port: %d\n", pin, port);
}

__attribute__((export_name("poll_events"))) void poll_events(void)
{
    ocre_process_events();
}

// =============================================================================
// PUBLIC API FUNCTIONS
// =============================================================================

int ocre_register_timer_callback(int timer_id, timer_callback_func_t callback)
{
    // Register dispatchers
    if (ocre_register_dispatcher(OCRE_RESOURCE_TYPE_TIMER, "timer_callback") != 0)
    {
        printf("Failed to register timer dispatcher\n");
        return -1;
    }

    init_callback_system();

    if (timer_id < 0 || timer_id >= MAX_CALLBACKS)
    {
        printf("Error: Timer ID %d out of range (0-%d)\n", timer_id, MAX_CALLBACKS - 1);
        return -1;
    }

    if (callback == NULL)
    {
        printf("Error: Timer callback is NULL for ID %d\n", timer_id);
        return -1;
    }

    timer_callbacks[timer_id] = callback;
    printf("Timer callback registered for ID: %d\n", timer_id);
    return 0;
}

int ocre_register_gpio_callback(int pin, int port, gpio_callback_func_t callback)
{
    // Register dispatchers
    if (ocre_register_dispatcher(OCRE_RESOURCE_TYPE_GPIO, "gpio_callback") != 0)
    {
        printf("Failed to register GPIO dispatcher\n");
        return -1;
    }

    init_callback_system();

    if (callback == NULL)
    {
        printf("Error: GPIO callback is NULL for pin %d, port %d\n", pin, port);
        return -1;
    }

    int slot = -1;
    for (int i = 0; i < MAX_CALLBACKS; i++)
    {
        if (gpio_callback_pins[i] == pin && gpio_callback_ports[i] == port)
        {
            slot = i; // Update existing
            break;
        }
        if (slot == -1 && gpio_callback_pins[i] == -1)
        {
            slot = i; // Found empty slot
        }
    }

    if (slot == -1)
    {
        printf("Error: No available slots for GPIO callbacks\n");
        return -1;
    }

    gpio_callback_pins[slot] = pin;
    gpio_callback_ports[slot] = port;
    gpio_callbacks[slot] = callback;
    printf("GPIO callback registered for pin: %d, port: %d (slot %d)\n", pin, port, slot);
    return 0;
}

int ocre_unregister_timer_callback(int timer_id)
{
    init_callback_system();

    if (timer_id < 0 || timer_id >= MAX_CALLBACKS)
    {
        return -1;
    }

    timer_callbacks[timer_id] = NULL;
    printf("Timer callback unregistered for ID: %d\n", timer_id);
    return 0;
}

int ocre_unregister_gpio_callback(int pin, int port)
{
    init_callback_system();

    for (int i = 0; i < MAX_CALLBACKS; i++)
    {
        if (gpio_callback_pins[i] == pin && gpio_callback_ports[i] == port)
        {
            gpio_callback_pins[i] = -1;
            gpio_callback_ports[i] = -1;
            gpio_callbacks[i] = NULL;
            printf("GPIO callback unregistered for pin: %d, port: %d\n", pin, port);
            return 0;
        }
    }

    return -1; // Pin/port not found
}

void ocre_process_events(void)
{
    event_data_t event_data;
    int event_count = 0;
    const int max_events_per_loop = 5;

    // Get the base address of event_data as an offset in WASM memory
    uint32_t base_offset = (uint32_t)&event_data;
    uint32_t type_offset = base_offset + offsetof(event_data_t, type);
    uint32_t id_offset = base_offset + offsetof(event_data_t, id);
    uint32_t port_offset = base_offset + offsetof(event_data_t, port);
    uint32_t state_offset = base_offset + offsetof(event_data_t, state);

    while (event_count < max_events_per_loop)
    {
        int ret = ocre_get_event(type_offset, id_offset, port_offset, state_offset);
        if (ret != 0)
        {
            break;
        }

        // Access event data
        int32_t type = event_data.type;
        int32_t id = event_data.id;
        int32_t port = event_data.port;
        int32_t state = event_data.state;

        // Validate event data
        if (type < 0 || type >= OCRE_RESOURCE_TYPE_COUNT || id < 0 || port < 0 ||
            (type == OCRE_RESOURCE_TYPE_GPIO && state != OCRE_GPIO_PIN_SET &&
             state != OCRE_GPIO_PIN_RESET))
        {
            printf("Invalid event: type=%d, id=%d, port=%d, state=%d\n", type, id, port, state);
            continue;
        }

        printf("Retrieved event: type=%d, id=%d, port=%d, state=%d\n", type, id, port, state);

        // Dispatch events
        if (type == OCRE_RESOURCE_TYPE_TIMER && port == 0)
        {
            timer_callback(id);
        }
        else if (type == OCRE_RESOURCE_TYPE_GPIO)
        {
            gpio_callback(id, state, port);
        }
        else
        {
            printf("Unknown event: type=%d, id=%d, port=%d, state=%d\n", type, id, port, state);
        }
        event_count++;
    }

    if (event_count == 0)
    {
        ocre_sleep(10);
    }
}

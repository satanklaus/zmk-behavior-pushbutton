/*
 * Copyright (c) XXXX The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_pushbutton

// Dependencies
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zmk/behavior.h>
#include <drivers/behavior.h>


LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_pushbutton_config {
	struct gpio_dt_spec gpio;
	uint32_t duration_ms;
};

// Instance-specific Data struct (Optional)
struct behavior_pushbutton_data {
	const struct device *dev;
	struct k_work_delayable work;
};

static void pushbutton_work_handler(struct k_work *work)
{
    struct k_work_delayable *dwork =
        k_work_delayable_from_work(work);

    struct behavior_pushbutton_data *data =
        CONTAINER_OF(dwork,
                     struct behavior_pushbutton_data,
                     work);

    const struct behavior_pushbutton_config *config =
        data->dev->config;

    gpio_pin_set_dt(&config->gpio, 0);

    LOG_DBG("GPIO OFF");
} 

// Initialization Function (Optional)
static int pushbutton_init(const struct device *dev) {
    const struct behavior_pushbutton_config *config =
        dev->config;

    struct behavior_pushbutton_data *data =
        dev->data;

    if (!device_is_ready(config->gpio.port)) {
        LOG_ERR("GPIO device is not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(
        &config->gpio,
        GPIO_OUTPUT_INACTIVE
    );

    if (ret) {
        LOG_ERR("Failed to configure GPIO: %d", ret);
        return ret;
    }

    data->dev = dev;

    k_work_init_delayable(
        &data->work,
        pushbutton_work_handler
    );

    return 0;
};

static int on_pushbutton_binding_pressed(struct zmk_behavior_binding *binding,
                                                 struct zmk_behavior_binding_event event) {
    const struct device *dev =
        zmk_behavior_get_binding(binding->behavior_dev);

    const struct behavior_pushbutton_config *config =
        dev->config;

    struct behavior_pushbutton_data *data =
        dev->data;

    gpio_pin_set_dt(&config->gpio, 1);

    LOG_DBG("GPIO ON");

    k_work_reschedule(
        &data->work,
        K_MSEC(config->duration_ms)
    );
    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_pushbutton_binding_released(struct zmk_behavior_binding *binding,
                                                  struct zmk_behavior_binding_event event) {
    return ZMK_BEHAVIOR_OPAQUE;
}

// API struct
static const struct behavior_driver_api pushbutton_driver_api = {
    .binding_pressed = on_pushbutton_binding_pressed
};

#define PUSHBUTTON_INST(n)                                                                                       \
    static struct behavior_pushbutton_data pushbutton_data_##n;                                                                  \
                                                                                                                         \
    static struct behavior_pushbutton_config pushbutton_config_##n = {                                                               \
        .gpio = GPIO_DT_SPEC_INST_GET(n, gpios),                                                                                           \
        .duration_ms = DT_INST_PROP_OR(n, press_duration_ms, 200),                                                                                      \
    };                                                                                                                   \
                                                                                                                         \
    BEHAVIOR_DT_INST_DEFINE(n,                                  /* Instance Number (Automatically populated by macro) */ \
                            pushbutton_init,            /* Initialization Function */                            \
                            NULL,                               /* Power Management Device Pointer */                    \
                            &pushbutton_data_##n,       /* Behavior Data Pointer */                              \
                            &pushbutton_config_##n,     /* Behavior Configuration Pointer */                     \
                            POST_KERNEL,                        /* Initialization Level */                               \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT /* Device Priority */                                    \
                            &pushbutton_driver_api);    /* API struct */                                         \

DT_INST_FOREACH_STATUS_OKAY(PUSHBUTTON_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */

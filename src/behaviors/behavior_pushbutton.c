/*
 * Copyright (c) 2025 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_pushbutton

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_pushbutton_config {
    struct gpio_dt_spec gpio;
    uint32_t duration_ms;
};

struct behavior_pushbutton_data {
    const struct device *dev;
    struct k_work_delayable release_work;
    int64_t pressed_at;
};

static void pushbutton_release(struct k_work *work) {
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    struct behavior_pushbutton_data *data =
        CONTAINER_OF(dwork, struct behavior_pushbutton_data, release_work);
    const struct behavior_pushbutton_config *config = data->dev->config;

    int ret = gpio_pin_set_dt(&config->gpio, 0);
    if (ret < 0) {
        LOG_ERR("Failed to deactivate pin %d: %d", config->gpio.pin, ret);
        return;
    }

    LOG_DBG("pin %d inactive after %lld ms", config->gpio.pin,
            k_uptime_get() - data->pressed_at);
}

static int behavior_pushbutton_init(const struct device *dev) {
    const struct behavior_pushbutton_config *config = dev->config;
    struct behavior_pushbutton_data *data = dev->data;

    if (!gpio_is_ready_dt(&config->gpio)) {
        LOG_ERR("GPIO device %s is not ready", config->gpio.port->name);
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&config->gpio, GPIO_OUTPUT_INACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure pin %d: %d", config->gpio.pin, ret);
        return ret;
    }

    data->dev = dev;
    k_work_init_delayable(&data->release_work, pushbutton_release);

    return 0;
}

static int on_pushbutton_binding_pressed(struct zmk_behavior_binding *binding,
                                         struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_pushbutton_config *config = dev->config;
    struct behavior_pushbutton_data *data = dev->data;

    int ret = gpio_pin_set_dt(&config->gpio, 1);
    if (ret < 0) {
        LOG_ERR("Failed to activate pin %d: %d", config->gpio.pin, ret);
        return ret;
    }

    // A press while the pin is still active restarts the pulse.
    data->pressed_at = k_uptime_get();
    k_work_reschedule(&data->release_work, K_MSEC(config->duration_ms));

    LOG_DBG("pin %d active for %d ms", config->gpio.pin, config->duration_ms);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_pushbutton_binding_released(struct zmk_behavior_binding *binding,
                                          struct zmk_behavior_binding_event event) {
    // The pulse length is fixed; releasing the key early does not cut it short.
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_pushbutton_driver_api = {
    .binding_pressed = on_pushbutton_binding_pressed,
    .binding_released = on_pushbutton_binding_released,
    .locality = BEHAVIOR_LOCALITY_CENTRAL,
};

#define PUSHBUTTON_INST(n)                                                                         \
    static struct behavior_pushbutton_data behavior_pushbutton_data_##n;                           \
    static const struct behavior_pushbutton_config behavior_pushbutton_config_##n = {              \
        .gpio = GPIO_DT_SPEC_INST_GET(n, gpios),                                                   \
        .duration_ms = DT_INST_PROP(n, press_duration_ms),                                         \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_pushbutton_init, NULL, &behavior_pushbutton_data_##n,      \
                            &behavior_pushbutton_config_##n, POST_KERNEL,                          \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_pushbutton_driver_api);

DT_INST_FOREACH_STATUS_OKAY(PUSHBUTTON_INST)

#endif /* DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT) */

#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>
#include <drivers/led_sensor.h>


/* The devicetree node identifier for the "led0" alias. */
#if DT_NODE_EXISTS(DT_ALIAS(app_led))
#define LED_NODE DT_ALIAS(app_led)
#else
#define LED_NODE DT_ALIAS(led0)
#endif

// using and nrf9151-DK board, the led0 alias is not defined in the devicetree.
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

/* Custom LED sensor device */
#if DT_HAS_COMPAT_STATUS_OKAY(custom_led_sensor)
static const struct device *const led_sensor = DEVICE_DT_GET_ANY(custom_led_sensor);

/* Shell command handlers */
static int cmd_sensor_fetch(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    if (!device_is_ready(led_sensor)) {
        shell_error(sh, "Sensor device not ready");
        return -ENODEV;
    }

    int ret = sensor_sample_fetch(led_sensor);
    if (ret < 0) {
        shell_error(sh, "Failed to fetch sample (%d)", ret);
        return ret;
    }

    shell_print(sh, "Sample fetched successfully (LED turned ON)");
    return 0;
}

static int cmd_sensor_read(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    if (!device_is_ready(led_sensor)) {
        shell_error(sh, "Sensor device not ready");
        return -ENODEV;
    }

    struct sensor_value val;
    int ret = sensor_channel_get(led_sensor, SENSOR_CHAN_ALL, &val);
    if (ret < 0) {
        shell_error(sh, "Failed to read channel (%d)", ret);
        return ret;
    }

    shell_print(sh, "Sensor read: val1=%d, val2=%d (LED turned OFF)", val.val1, val.val2);
    return 0;
}

static int cmd_sensor_info(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    bool ready = device_is_ready(led_sensor);
    shell_print(sh, "Device name: %s", led_sensor->name);
    shell_print(sh, "Device ready: %s", ready ? "yes" : "no");
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensor,
    SHELL_CMD(fetch, NULL, "Fetch sensor sample (turns LED ON)", cmd_sensor_fetch),
    SHELL_CMD(read, NULL, "Read sensor channel (turns LED OFF)", cmd_sensor_read),
    SHELL_CMD(info, NULL, "Print device name and ready state", cmd_sensor_info),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(sensor, &sub_sensor, "Sensor commands", NULL);

#endif

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    bool led_state = true;

#if DT_HAS_COMPAT_STATUS_OKAY(custom_led_sensor)
    if (led_sensor && device_is_ready(led_sensor)) {
        LOG_INF("LED Sensor device is ready");
        /* Test the driver API */
        sensor_sample_fetch(led_sensor);
        k_msleep(500);
        sensor_channel_get(led_sensor, SENSOR_CHAN_ALL, NULL);

        /* Test custom extension API */
        led_sensor_set_invert(led_sensor, false);
    }
#endif

    if (!gpio_is_ready_dt(&led)) return 0;

    if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) return 0;

    while (1) {
#if DT_HAS_COMPAT_STATUS_OKAY(custom_led_sensor)
        if (led_sensor && device_is_ready(led_sensor)) {
            sensor_sample_fetch(led_sensor);
            led_state = true;
            LOG_INF("LED state (via sensor fetch): ON");
#if defined(CONFIG_APP_HEARTBEAT_PERIOD_MS)
            k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
#else
            k_msleep(CONFIG_BLINK_SLEEP_TIME_MS);
#endif
            sensor_channel_get(led_sensor, SENSOR_CHAN_ALL, NULL);
            led_state = false;
            LOG_INF("LED state (via sensor get): OFF");
#if defined(CONFIG_APP_HEARTBEAT_PERIOD_MS)
            k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
#else
            k_msleep(CONFIG_BLINK_SLEEP_TIME_MS);
#endif
            continue;
        }
#endif

        if (gpio_pin_toggle_dt(&led) < 0) return 0;

        led_state = !led_state;
        LOG_INF("LED state: %s", led_state ? "ON" : "OFF");
#if defined(CONFIG_APP_HEARTBEAT_PERIOD_MS)
        k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
#else
        k_msleep(CONFIG_BLINK_SLEEP_TIME_MS);
#endif
    }
    return 0;
}

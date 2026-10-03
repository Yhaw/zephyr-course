#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#define DT_DRV_COMPAT custom_led_sensor

LOG_MODULE_REGISTER(led_sensor, CONFIG_SENSOR_LOG_LEVEL);

struct led_sensor_config {
	struct gpio_dt_spec led;
};

struct led_sensor_data {
	int led_state;
};

static int led_sensor_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	const struct led_sensor_config *cfg = dev->config;
	struct led_sensor_data *data = dev->data;

	/* Turning on the LED */
	gpio_pin_set_dt(&cfg->led, 1);
	data->led_state = 1;

	return 0;
}

static int led_sensor_channel_get(const struct device *dev,
				  enum sensor_channel chan,
				  struct sensor_value *val)
{
	const struct led_sensor_config *cfg = dev->config;
	struct led_sensor_data *data = dev->data;

	/* Turning off the LED */
	gpio_pin_set_dt(&cfg->led, 0);
	data->led_state = 0;

	if (val != NULL) {
		val->val1 = 0;
		val->val2 = 0;
	}

	return 0;
}

static DEVICE_API(sensor, led_sensor_api) = {
	.sample_fetch = led_sensor_sample_fetch,
	.channel_get = led_sensor_channel_get,
};

static int led_sensor_init(const struct device *dev)
{
	const struct led_sensor_config *cfg = dev->config;

	if (!gpio_is_ready_dt(&cfg->led)) {
		LOG_ERR("LED GPIO not ready");
		return -ENODEV;
	}

	if (gpio_pin_configure_dt(&cfg->led, GPIO_OUTPUT_INACTIVE) < 0) {
		LOG_ERR("Failed to configure LED GPIO");
		return -EIO;
	}

	return 0;
}

#define LED_SENSOR_INIT(inst)                                                   \
	static struct led_sensor_data led_sensor_data_##inst;                  \
	static const struct led_sensor_config led_sensor_config_##inst = {     \
		.led = GPIO_DT_SPEC_INST_GET(inst, led_gpios),                 \
	};                                                                      \
	DEVICE_DT_INST_DEFINE(inst,                                            \
			      led_sensor_init,                                 \
			      NULL,                                            \
			      &led_sensor_data_##inst,                         \
			      &led_sensor_config_##inst,                        \
			      POST_KERNEL,                                     \
			      CONFIG_SENSOR_INIT_PRIORITY,                     \
			      &led_sensor_api);

DT_INST_FOREACH_STATUS_OKAY(LED_SENSOR_INIT)

#ifndef DRIVERS_LED_SENSOR_H_
#define DRIVERS_LED_SENSOR_H_

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Custom extension API to update runtime state parameter */
int led_sensor_set_invert(const struct device *dev, bool invert);

#ifdef __cplusplus
}
#endif

#endif /* DRIVERS_LED_SENSOR_H_ */

#include <bmi160.h>

extern uint8_t pbuf[1024];
extern uint8_t *pos;
static struct device *bmi160;

/*
 * The values in the following map are the expected values that the
 * accelerometer needs to converge to if the device lies flat on the table. The
 * device has to stay still for about 500ms = 250ms(accel) + 250ms(gyro).
 */
struct sensor_value acc_calib[] = {
	{SENSOR_VALUE_TYPE_INT_PLUS_MICRO, { {0, 0} } },      /* X */
	{SENSOR_VALUE_TYPE_INT_PLUS_MICRO, { {0, 0} } },      /* Y */
	{SENSOR_VALUE_TYPE_INT_PLUS_MICRO, { {9, 806650} } }, /* Z */
};

static int auto_calibration(struct device* bmi160)
{
	/* calibrate accelerometer */
	if (sensor_attr_set(bmi160, SENSOR_CHAN_ACCEL_ANY,
			      SENSOR_ATTR_CALIB_TARGET, acc_calib) < 0) {
		return -EIO;
	}

	/*
	 * Calibrate gyro. No calibration value needs to be passed to BMI160 as
	 * the target on all axis is set internally to 0. This is used just to
	 * trigger a gyro calibration.
	 */
	if (sensor_attr_set(bmi160, SENSOR_CHAN_GYRO_ANY,
			      SENSOR_ATTR_CALIB_TARGET, NULL) < 0) {
		return -EIO;
	}

	return 0;
}


/**
 * @brief Helper function for printing a sensor value to a buffer
 *
 * @param buf A pointer to the buffer to which the printing is done.
 * @param len Size of buffer in bytes.
 * @param val A pointer to a sensor_value struct holding the value
 *            to be printed.
 *
 * @return The number of characters printed to the buffer.
 */
static inline int sensor_value_snprintf(char *buf, size_t len, const struct sensor_value *val)
{
	int32_t val1, val2;

	switch (val->type) {
	case SENSOR_VALUE_TYPE_INT:
		return snprintf(buf, len, "%d", val->val1);
	case SENSOR_VALUE_TYPE_INT_PLUS_MICRO:
		if (val->val2 == 0) {
			return snprintf(buf, len, "%d", val->val1);
		}

		/* normalize value */
		if (val->val1 < 0 && val->val2 > 0) {
			val1 = val->val1 + 1;
			val2 = val->val2 - 1000000;
		} else {
			val1 = val->val1;
			val2 = val->val2;
		}

		/* print value to buffer */
		if (val1 > 0 || (val1 == 0 && val2 > 0)) {
			return snprintf(buf, len, "%d.%06d", val1, val2);
		} else if (val1 == 0 && val2 < 0) {
			return snprintf(buf, len, "-0.%06d", -val2);
		} else {
			return snprintf(buf, len, "%d.%06d", val1, -val2);
		}
	case SENSOR_VALUE_TYPE_DOUBLE:
		return snprintf(buf, len, "%f", val->dval);
	default:
		return 0;
	}
}

void get_gyro_data(struct sensor_value* sensor_data_ps){
	if (sensor_channel_get(bmi160, SENSOR_CHAN_GYRO_ANY, sensor_data_ps) < 0) {
		printf("Cannot read bmi160 gyro channels.\n");
		return;
	}
}

void get_temp_data(struct sensor_value* sensor_data_ps){
	if (sensor_channel_get(bmi160, SENSOR_CHAN_TEMP, sensor_data_ps) < 0) {
		printf("Temperature channel read error.\n");
		return;
	}
}

void get_accel_data(struct sensor_value* sensor_data_ps){
	if (sensor_channel_get(bmi160, SENSOR_CHAN_ACCEL_ANY, sensor_data_ps) < 0) {
		printf("Cannot read bmi160 accel channels.\n");
		return;
	}
}

int sample_update () {
	return sensor_sample_fetch(bmi160);
}

void bmi160_sensor_init()
{
	printf("IMU: Binding...\n");
	bmi160 = device_get_binding("bmi160");
	if (!bmi160) {
		printf("Gyro: Device not found.\n");
		return;
	}

	/* auto calibrate accelerometer and gyro */
	if (auto_calibration(bmi160) < 0) {
		printf("HW calibration failed.\n");
		return;
	} else {
		printf("HW calibration passed.\n");
	}
}

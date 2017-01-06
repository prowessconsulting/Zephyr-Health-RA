/** @file
 *  @brief ESS Service sample
 */

/*
 * Copyright (c) 2016 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ess.h"
#include "uuid.h"


static struct bt_gatt_ccc_cfg  blvl_ccc_cfg[CONFIG_BLUETOOTH_MAX_PAIRED] = {};
static uint8_t simulate_blvl;

static char ess_humidity = 2;
static char ess_pressure = 3;

static int64_t ess_accel[3];
static int64_t ess_gyro[3];
static int16_t ess_temp;

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
static inline double sensor_value_normalize(const struct sensor_value *val)
{
	int32_t val1, val2;

	switch (val->type) {
	case SENSOR_VALUE_TYPE_INT:
		return (double) val->val1;
	case SENSOR_VALUE_TYPE_INT_PLUS_MICRO:
		if (val->val2 == 0) {
			return (double) val->val1;
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
			return val1 + val2 / 1000000;
		} else if (val1 == 0 && val2 < 0) {
			return val2 / -1000000;
		} else {
			return val1 + val2 / -1000000;
		}
	case SENSOR_VALUE_TYPE_DOUBLE:
		return val->dval;
	default:
		return 0;
	}
}



static void blvl_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	simulate_blvl = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static ssize_t read_temperature(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &ess_temp, sizeof(ess_temp));
}

static ssize_t read_humidity(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &ess_humidity, sizeof(ess_humidity));
}

static ssize_t read_pressure(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &ess_pressure, sizeof(ess_pressure));
}

static ssize_t read_gyrometer(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &ess_gyro, sizeof(ess_gyro));
}

static ssize_t read_accelerometer(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &ess_accel, sizeof(ess_accel));
}

/* Automation IO Service Declaration */
// https://nexus.zephyrproject.org/content/sites/site/org.zephyrproject.zephyr/dev/api/html/de/d63/gatt_8h.html
static struct bt_gatt_attr attrs[] = {
	BT_GATT_PRIMARY_SERVICE(BT_UUID_ESS),

	BT_GATT_CHARACTERISTIC(BT_UUID_ESS_TEMPERATURE, BT_GATT_CHRC_READ),
	BT_GATT_DESCRIPTOR(BT_UUID_ESS_TEMPERATURE, BT_GATT_PERM_READ, read_temperature, NULL, NULL),

	BT_GATT_CHARACTERISTIC(BT_UUID_ESS_HUMIDITY, BT_GATT_CHRC_READ),
	BT_GATT_DESCRIPTOR(BT_UUID_ESS_HUMIDITY, BT_GATT_PERM_READ, read_humidity, NULL, NULL),

	BT_GATT_CHARACTERISTIC(BT_UUID_ESS_PRESSURE, BT_GATT_CHRC_READ),
	BT_GATT_DESCRIPTOR(BT_UUID_ESS_PRESSURE, BT_GATT_PERM_READ, read_pressure, NULL, NULL),

	BT_GATT_CHARACTERISTIC(BT_UUID_ESS_GYRO, BT_GATT_CHRC_READ),
	BT_GATT_DESCRIPTOR(BT_UUID_ESS_GYRO, BT_GATT_PERM_READ, read_gyrometer, NULL, NULL),

	BT_GATT_CHARACTERISTIC(BT_UUID_ESS_ACCELEROMETER, BT_GATT_CHRC_READ),
	BT_GATT_DESCRIPTOR(BT_UUID_ESS_ACCELEROMETER, BT_GATT_PERM_READ, read_accelerometer, NULL, NULL),
};

void ess_init(void)
{
	bt_gatt_register(attrs, ARRAY_SIZE(attrs));
}

void ess_gyro_notify(struct sensor_value* value, uint32_t axis)
{
	ess_gyro[axis] = ((uint64_t) value->val1) << 32 | value->val2;
};

void ess_accel_notify(struct sensor_value* value, uint32_t axis)
{
	ess_accel[axis] = ((uint64_t) value->val1) << 32 | value->val2;
};

void ess_temp_notify(struct sensor_value* value)
{
	ess_temp = (value->val1 * 100) + (value->val2 / 10000);
};

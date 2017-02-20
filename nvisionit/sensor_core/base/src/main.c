/* main_arc.c - main source file for ARC app */

/*
 * Copyright (c) 2016 Intel Corporation.
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

#include <zephyr.h>
#include <stdio.h>
#include <device.h>
#include <sensor.h>
#include <max30100_pulse_oximeter.h>

#include <ipm.h>
#include <ipm/ipm_quark_se.h>

#include <init.h>
#include <string.h>

#include <misc/byteorder.h>
#include <misc/printk.h>
#include <misc/util.h>

#include <ipm_ids.h>
#include <bmi160.h>
#include <heartrate.h>

#define INTERVAL_HRS 500

QUARK_SE_IPM_DEFINE(health_sensor_ipm, 0, QUARK_SE_IPM_OUTBOUND);

struct device *i2c_dev;
struct device *health_ipm;

struct sensor_value sensor_value_ast[3];

struct health_data
{
	uint8_t heartrate;
	uint8_t spo2;
	int16_t temperature;
	int16_t gyro_x;
	int16_t gyro_y;
	int16_t gyro_z;
	int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
};

static double sensor_value_normalize(struct sensor_value* val)
{
	double value;

	switch (val->type) {
	case SENSOR_VALUE_TYPE_INT:
	    value = (double)(val->val1);
	    break;
	case SENSOR_VALUE_TYPE_INT_PLUS_MICRO:
		value = (double)(val->val1);
		value += ((double)(val->val2)) * 0.000001;
	    break;
	case SENSOR_VALUE_TYPE_DOUBLE:
	    value = val->dval;
	    break;
	default:
		value = 0;
	    break;
	}
	return value;
}

void main(void)
{
    int ret;

    // Get the devices
    i2c_dev = device_get_binding("I2C_0");
    if (!i2c_dev)
    {
	printk("Error getting I2C device.\n");
    }

    /* Initialize the IPM */
    health_ipm = device_get_binding("health_sensor_ipm");
    if (!health_ipm)
    {
		printk("IPM: Device not found.\n");
    }

    bmi160_sensor_init();
	
	printk("Initializing the MAX30100\n");
	printk("Initializing the MAX30100 Pulse Oximeter\n");
	max30100_init(i2c_dev);
	max30100_pulse_oximeter_init(i2c_dev);
   
	printk("Polling the MAX30100\n");

	struct health_data data;
	int16_t axis_x = 0; 
	int16_t axis_y = 0;
	int16_t axis_z = 0;

    uint64_t lastSampleTime = 0;
	while(1){
	    time = k_uptime_get();
	    max30100_pulse_oximeter_update(i2c_dev);

		if((time - lastSampleTime) > 500)
		{	
			data.heartrate = max30100_pulse_oximeter_heartrate;
			data.spo2 = max30100_pulse_oximeter_spo2;
			data.temperature = max30100_pulse_oximeter_temperature;
			
			get_accel_data(sensor_value_ast);	

			axis_x = (int16_t)(sensor_value_normalize(&sensor_value_ast[0]) * 1000.0);
			axis_y = (int16_t)(sensor_value_normalize(&sensor_value_ast[1]) * 1000.0);
			axis_z = (int16_t)(sensor_value_normalize(&sensor_value_ast[2]) * 1000.0);

			// printk("Accel: %d %d %d\n", axis_x, axis_y, axis_z);

			data.accel_x = (axis_x + data.accel_x) / 2;
			data.accel_y = (axis_y + data.accel_y) / 2;
			data.accel_z = (axis_z + data.accel_z) / 2;

			get_gyro_data(sensor_value_ast);		

			axis_x = (int16_t)(sensor_value_normalize(&sensor_value_ast[0]) * 1000.0);
			axis_y = (int16_t)(sensor_value_normalize(&sensor_value_ast[1]) * 1000.0);
			axis_z = (int16_t)(sensor_value_normalize(&sensor_value_ast[2]) * 1000.0);

			// printk("Gyro : %d %d %d\n", axis_x, axis_y, axis_z);

			data.gyro_x = (axis_x + data.gyro_x) / 2;
			data.gyro_y = (axis_y + data.gyro_y) / 2;
			data.gyro_z = (axis_z + data.gyro_z) / 2;

		    lastSampleTime = k_uptime_get();
            sample_update();

			ret = ipm_send(health_ipm, 1, IPM_ID_BMI_ALL, &data, sizeof(data));
			if (ret)
			{
				printk("Failed to send Health message, error (%d)\n", ret);
			}		

		}
	}	
}
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
#include <adc.h>

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

/* measure every 2ms */
#define INTERVAL_HRS 	2
#define INTERVAL 	320
#define INTERVAL_TEMP 1000


QUARK_SE_IPM_DEFINE(hrs_sensor_ipm, 0, QUARK_SE_IPM_OUTBOUND);
QUARK_SE_IPM_DEFINE(temp_sensor_ipm, 1, QUARK_SE_IPM_OUTBOUND);
QUARK_SE_IPM_DEFINE(accel_sensor_ipm, 2, QUARK_SE_IPM_OUTBOUND);
QUARK_SE_IPM_DEFINE(gyro_sensor_ipm, 3, QUARK_SE_IPM_OUTBOUND);


// START Heartrate sensor variables
#define ADC_DEVICE_NAME "ADC_0"
#define ADC_CHANNEL 	12
#define ADC_BUFFER_SIZE 4

struct device *adc;

static uint8_t seq_buffer[ADC_BUFFER_SIZE];

static struct adc_seq_entry sample = {
	.sampling_delay = 12,
	.channel_id = ADC_CHANNEL,
	.buffer = seq_buffer,
	.buffer_length = ADC_BUFFER_SIZE,
};

static struct adc_seq_table table = {
	.entries = &sample,
	.num_entries = 1,
};

uint8_t heartrate_u8;
// END Heartrate sensor variables

extern void poll_hrs(struct k_timer *timer_id){
	if (adc_read(adc, &table) == 0) {

		uint32_t signal = *((uint32_t*)seq_buffer); // This is faster than bit shifting, no math, just dereference.
		signal = signal & 0xFFF;
		uint32_t value = measure_heartrate(signal);
		if (value > 0) {
			heartrate_u8 = value;
		}
	}
}

K_TIMER_DEFINE(hrs_timer, poll_hrs, NULL);


void main(void)
{
	struct device *hrs_ipm, *temp_ipm, *accel_ipm, *gyro_ipm;
	int ret;

	/* Initialize the IPM */
	hrs_ipm = device_get_binding("hrs_sensor_ipm");
	if (!hrs_ipm) {
		printk("IPM: Device not found.\n");
	}

	temp_ipm = device_get_binding("temp_sensor_ipm");
	if (!temp_ipm) {
		printk("IPM: Device not found.\n");
	}

	accel_ipm = device_get_binding("accel_sensor_ipm");
	if (!accel_ipm) {
		printk("IPM: Device not found.\n");
	}

	gyro_ipm = device_get_binding("gyro_sensor_ipm");
	if (!gyro_ipm) {
		printk("IPM: Device not found.\n");
	}

	adc = device_get_binding(ADC_DEVICE_NAME);
	if (!adc) {
		printk("ADC Controller: Device not found.\n");
		return;
	}
	adc_enable(adc);

	bmi160_sensor_init();

	k_timer_start(&hrs_timer, K_MSEC(INTERVAL_HRS), K_MSEC(INTERVAL_HRS));

	struct sensor_value accel_sensor_value_ast[3];
	struct sensor_value gyro_sensor_value_ast[3];
	struct sensor_value temp_sensor_value_st;

	while (hrs_ipm) {
		sample_update();

		get_temp_data(&temp_sensor_value_st);
		get_accel_data(accel_sensor_value_ast);
		get_gyro_data(gyro_sensor_value_ast);

		// /* send data over ipm to x86 side */
		ret = ipm_send(hrs_ipm, 1, IPM_ID_BMI_ALL, &heartrate_u8, sizeof(heartrate_u8));
		if (ret) {
			printk("Failed to send IPM_ID_BMI_ALL message, error (%d)\n", ret);
		}
		k_sleep(INTERVAL / 8);

		ret = ipm_send(temp_ipm, 1, IPM_ID_BMI_ALL, &temp_sensor_value_st, sizeof(temp_sensor_value_st));
		if (ret) {
			printk("Failed to send IPM_ID_BMI_ALL message, error (%d)\n", ret);
		}
		k_sleep(INTERVAL / 8);

		ret = ipm_send(accel_ipm, 1, IPM_ID_ACCEL_X, &accel_sensor_value_ast[0], sizeof(struct sensor_value));
		if (ret) {
			printk("Failed to send IPM_ID_ACCEL_X message, error (%d)\n", ret);
		}
		k_sleep(INTERVAL / 8);

		ret = ipm_send(accel_ipm, 1, IPM_ID_ACCEL_Y, &accel_sensor_value_ast[1], sizeof(struct sensor_value));
		if (ret) {
			printk("Failed to send IPM_ID_ACCEL_Y message, error (%d)\n", ret);
		}
		k_sleep(INTERVAL / 8);

		ret = ipm_send(accel_ipm, 1, IPM_ID_ACCEL_Z, &accel_sensor_value_ast[2], sizeof(struct sensor_value));
		if (ret) {
			printk("Failed to send IPM_ID_ACCEL_Z message, error (%d)\n", ret);
		}
		k_sleep(INTERVAL / 8);

		ret = ipm_send(gyro_ipm, 1, IPM_ID_GYRO_X, &gyro_sensor_value_ast[0], sizeof(struct sensor_value));
		if (ret) {
			printk("Failed to send IPM_ID_GYRO_X message, error (%d)\n", ret);
		}
		k_sleep(INTERVAL / 8);

		ret = ipm_send(gyro_ipm, 1, IPM_ID_GYRO_Y, &gyro_sensor_value_ast[1], sizeof(struct sensor_value));
		if (ret) {
			printk("Failed to send IPM_ID_GYRO_Y message, error (%d)\n", ret);
		}
		k_sleep(INTERVAL / 8);

		ret = ipm_send(gyro_ipm, 1, IPM_ID_GYRO_Z, &gyro_sensor_value_ast[2], sizeof(struct sensor_value));
		if (ret) {
			printk("Failed to send IPM_ID_GYRO_Z message, error (%d)\n", ret);
		}

	}

	adc_disable(adc);

}

#ifndef BMI160_H
#define BMI160_H

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

#include <zephyr.h>
#include <stdio.h>
#include <device.h>
#include <sensor.h>
#include <misc/util.h>

#define MAX_TEST_TIME	15000
#define SLEEPTIME	300


void bmi160_sensor_init(void);
int sample_update();
void get_gyro_data(struct sensor_value* sensor_data_ps);
void get_accel_data(struct sensor_value* sensor_data_ps);
void get_temp_data(struct sensor_value* sensor_data_ps);

#endif

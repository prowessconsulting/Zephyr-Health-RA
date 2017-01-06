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
 #include <stdint.h>
 #include <stddef.h>
 #include <string.h>
 #include <errno.h>
 #include <misc/printk.h>
 #include <misc/byteorder.h>
 #include <zephyr.h>

 #include <bluetooth/bluetooth.h>
 #include <bluetooth/hci.h>
 #include <bluetooth/conn.h>
 #include <bluetooth/uuid.h>
 #include <bluetooth/gatt.h>

 #include <sensor.h>

void ess_init(void);
void ess_gyro_notify(struct sensor_value* value, uint32_t axis);
void ess_accel_notify(struct sensor_value* value, uint32_t axis);
void ess_temp_notify(struct sensor_value* value);

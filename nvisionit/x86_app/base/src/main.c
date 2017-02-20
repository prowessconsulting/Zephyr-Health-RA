/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
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

#include <sensor.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include <ipm.h>
#include <ipm/ipm_quark_se.h>

#include <ipm_ids.h>

#define DEVICE_NAME		"nV Zephyr Heartrate Sensor"
#define DEVICE_NAME_LEN		(sizeof(DEVICE_NAME) - 1)

#define HEARTRATE_AVERAGE_COUNT 10

QUARK_SE_IPM_DEFINE(health_sensor_ipm, 0, QUARK_SE_IPM_INBOUND);

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


// START: GATT stuff
// GATT includes
#include <gatt/gap.h>
#include <gatt/hrs.h>
#include <gatt/pos.h>
#include <gatt/bas.h>
#include <gatt/ess.h>

// Define what the device 'look' like
#define GAP_APPEARANCE	0x0341

// Register / initialize GATT sensors here
static void start_gatt()
{
	gap_init(DEVICE_NAME, GAP_APPEARANCE);
	hrs_init(0x01);
	pos_init(0x01);
	bas_init();
	ess_init();
	dis_init(CONFIG_SOC, "Manufacturer");
}
// END: GATT stuff

uint8_t heartrate_average[HEARTRATE_AVERAGE_COUNT];
int heartrate_average_pos = 0;

// START: IPM stuff
// add 'watch' for sensor messages
void health_ipm_callback(void *context, uint32_t id, volatile void *data_ptr)
{
    struct health_data *data;
	data = (struct health_data*)data_ptr;
    
	if(heartrate_average_pos >= 10)
	{
		heartrate_average_pos = 0;
	}
	heartrate_average[heartrate_average_pos++] = data->heartrate;

	if(heartrate_average[9] > 0)
	{
		int hrs_sum = 0;
		for(int j = 0; j < 10; j++)
		{
			hrs_sum += heartrate_average[j];
		}
		hrs_sum /= 10;
	    hrs_notify((uint8_t)hrs_sum);
	}
	pos_notify(data->spo2, data->heartrate);

	ess_temp_notify(data->temperature);
	ess_gyro_notify(data->gyro_x, data->gyro_y, data->gyro_z);
	ess_accel_notify(data->accel_x, data->accel_y, data->accel_z);
}

// END: IPM stuff


// STOP HERE
// Don't touch the rest of the x86_app main.c
// it should work by only adding the gatt stuff and ipm stuff above

struct bt_conn *default_conn;

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),

	// Declare the services that will be advertised here
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, 0x0d, 0x18, 0x0f, 0x18, 0x05, 0x18),
};

static const struct bt_data sd[] = {
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err %u)\n", err);
	} else {
		default_conn = bt_conn_ref(conn);
		printk("Connected\n");
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)\n", reason);

	if (default_conn) {
		bt_conn_unref(default_conn);
		default_conn = NULL;
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected,
};

static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		printk("Advertising failed to start (err %d)\n", err);
		return;
	}

	start_gatt();

	printk("Advertising successfully started\n");
}

static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	printk("Pairing cancelled: %s\n", addr);
}

static struct bt_conn_auth_cb auth_cb_display = {
	.cancel = auth_cancel,
};

void main(void)
{
	int err;
	struct device *health_ipm;

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}
	
	for(heartrate_average_pos = 0; heartrate_average_pos < 10; heartrate_average_pos++){
		heartrate_average[heartrate_average_pos] = 0; 
	}

	bt_conn_cb_register(&conn_callbacks);
	bt_conn_auth_cb_register(&auth_cb_display);


	printk("START: device_get_binding\n");
	health_ipm = device_get_binding("health_sensor_ipm");
	if (!health_ipm)
	{
		printk("IPM: Device not found.\n");
		return;
	}

	printk("START: ipm_register_callback\n");
	ipm_register_callback(health_ipm, health_ipm_callback, NULL);

	printk("START: ipm_set_enabled\n");
	ipm_set_enabled(health_ipm, 1);

	task_sleep(TICKS_UNLIMITED);
}

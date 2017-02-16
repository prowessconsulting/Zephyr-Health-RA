/** @file
 *  @brief POS Service sample
 */

#include "pos.h"
#include "uuid.h"

static struct bt_gatt_ccc_cfg blvl_ccc_cfg[CONFIG_BLUETOOTH_MAX_PAIRED] = {};
static uint8_t simulate_blvl;

static uint8_t pos_blsc;

static uint16_t pos_spo2 = 2;
static uint16_t pos_hr = 3;

static void blvl_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
    simulate_blvl = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static ssize_t read_blsc(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                         void *buf, uint16_t len, uint16_t offset)
{
    return bt_gatt_attr_read(conn, attr, buf, len, offset, &pos_blsc,
                             sizeof(pos_blsc));
}

/* Service Declaration */
static struct bt_gatt_attr attrs[] = {
    BT_GATT_PRIMARY_SERVICE(BT_UUID_POS),
    BT_GATT_CHARACTERISTIC(BT_UUID_POS_PLX_CONTINUOUS, BT_GATT_CHRC_NOTIFY),
    BT_GATT_DESCRIPTOR(BT_UUID_POS_PLX_CONTINUOUS, BT_GATT_PERM_READ, NULL,
                       NULL, NULL),
    BT_GATT_CCC(blvl_ccc_cfg, blvl_ccc_cfg_changed),

};

void pos_notify(int16_t spo2, int16_t pulserate)
{    
	static uint8_t pos[5];

	pos[0] = 0x0; /* uint8, no sensor contact */
	pos[1] = 0;
	pos[2] = spo2;
	pos[3] = 0;
	pos[4] = pulserate;

	bt_gatt_notify(NULL, &attrs[2], &pos, sizeof(pos));
}

void pos_init(uint8_t blsc)
{
    pos_blsc = blsc;
    bt_gatt_register(attrs, ARRAY_SIZE(attrs));
}
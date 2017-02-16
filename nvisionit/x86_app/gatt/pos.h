/** @file
 *  @brief POS Service sample
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

void pos_init(uint8_t blsc);
void pos_notify(int16_t spo2, int16_t pulserate);

#include <max30100.h>

void max30100_spo2_calculator_update(float led_ir_ac_value, float led_red_ac_value, bool beat_detected);
void max30100_spo2_calculator_reset();
uint8_t max30100_spo2_calculator_get_spo2();

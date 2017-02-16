#include <max30100.h>
#include <max30100_beat_detector.h>
#include <max30100_spo2_calculator.h>

#define SAMPLING_FREQUENCY                  100
#define SAMPLE_TIME                         (1000 / SAMPLING_FREQUENCY)
#define CURRENT_ADJUSTMENT_PERIOD_MS        500
#define IR_LED_CURRENT                      MAX30100_LED_CURR_50MA
#define RED_LED_CURRENT_START               MAX30100_LED_CURR_27_1MA
#define DC_REMOVER_ALPHA                    0.95
#define TEMPERATURE_SAMPLING_PERIOD_MS      10000

typedef enum PulseOximeterState {
    PULSEOXIMETER_STATE_INIT,
    PULSEOXIMETER_STATE_IDLE,
    PULSEOXIMETER_STATE_DETECTING
} PulseOximeterState;

void max30100_pulse_oximeter_init(struct device *i2c_dev);
void max30100_pulse_oximeter_update(struct device *i2c_dev);

uint8_t max30100_pulse_oximeter_heartrate;
uint8_t max30100_pulse_oximeter_spo2;
int16_t max30100_pulse_oximeter_temperature;
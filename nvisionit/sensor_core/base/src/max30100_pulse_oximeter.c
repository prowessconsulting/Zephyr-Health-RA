#include <max30100_pulse_oximeter.h>

#define DC_REMOVAL_VAL(x, w, alpha) (x + alpha * w)
#define DC_REMOVAL_PREV(x, w, alpha) (DC_REMOVAL_VAL(x, w, alpha) - w)

PulseOximeterState max30100_pulse_oximeter_state = PULSEOXIMETER_STATE_IDLE;
float led_ir_ac_dcw = 0;
float led_red_ac_dcw = 0;
uint8_t redLedPower;

uint32_t tsFirstBeatDetected;
uint32_t tsLastBeatDetected;
uint32_t tsLastSample;
uint32_t tsLastBiasCheck;
uint32_t tsLastCurrentAdjustment;
uint32_t tsLastTemperaturePoll;

float beat_detection_filter[2];

// http://sam-koblenski.blogspot.de/2015/11/everyday-dsp-for-programmers-dc-and.html
float _dc_removal(float x, float *w, float alpha)
{
    float w_n = x + alpha * (*w);
    float y = w_n - (*w);
    *w = w_n;
    return y;
}

// http://www.schwietering.com/jayduino/filtuino/
// Low pass butterworth filter order=1 alpha1=0.1
// Fs=100Hz, Fc=6Hz
float _low_pass_filter(float x, float *v)
{
    v[0] = v[1];
    v[1] = (2.452372752527856026e-1 * x) + (0.50952544949442879485 * v[0]);
    return (v[0] + v[1]);
}

void max30100_pulse_oximeter_init(struct device *i2c_dev)
{
    max30100_pulse_oximeter_heartrate = 0;

    max30100_set_mode(i2c_dev, MAX30100_MODE_SPO2_HR);
    max30100_set_leds_current(i2c_dev, IR_LED_CURRENT, RED_LED_CURRENT_START);

    beat_detection_filter[0] = 0;
    beat_detection_filter[1] = 0;

    max30100_pulse_oximeter_state = PULSEOXIMETER_STATE_IDLE;

    max30100_start_temperature(i2c_dev);

    printk("MAX30100: Waiting for the device to become ready\n");
    while (!max30100_is_temperature_ready(i2c_dev))
    {
        k_sleep(50);
    }
    printk("MAX30100: Ready!\n");
}

uint8_t buffer[4];
void max30100_pulse_oximeter_update(struct device *i2c_dev)
{
    if ((time - tsLastSample) > SAMPLE_TIME)
    {
        //printk("MAX30100: Updating data from FIFO - 0x%x\n", i2c_dev);
        max30100_update(i2c_dev, buffer);
        //printk("MAX30100: Received data from FIFO\n");

        // Warning: the values are always left-aligned
        uint16_t raw_ir_value = (buffer[0] << 8) | buffer[1];
        uint16_t raw_red_value = (buffer[2] << 8) | buffer[3];

        //printk("MAX30100: Filtering LED data\n");
        float led_ir_ac_value = _dc_removal(raw_ir_value, &led_ir_ac_dcw, DC_REMOVER_ALPHA);
        float led_red_ac_value = _dc_removal(raw_red_value, &led_red_ac_dcw, DC_REMOVER_ALPHA);

        //printk("MAX30100: Applying Low Pass filter for the heart beat class\n");
        float led_ir_heartrate_filtered_sample = _low_pass_filter(-led_ir_ac_value, beat_detection_filter);

        //printk("MAX30100: Sending sample to the heartbeat sample function\n");
        bool beat_detected = max30100_beat_detector_sample(led_ir_heartrate_filtered_sample);
        float hrs = max30100_beat_detector_get_rate();

        if (hrs > 0)
        {
            max30100_pulse_oximeter_state = PULSEOXIMETER_STATE_DETECTING;
            max30100_spo2_calculator_update(led_ir_ac_value, led_red_ac_value, beat_detected);

            if (beat_detected)
            {
                max30100_pulse_oximeter_spo2 = max30100_spo2_calculator_get_spo2();
                max30100_pulse_oximeter_heartrate = (uint8_t)hrs;
                max30100_pulse_oximeter_temperature = (int16_t)(max30100_get_temperature(i2c_dev) * 100);

                printk("MAX30100: Temperature: %d / 100 C\n", max30100_pulse_oximeter_temperature);
                printk("MAX30100: Heartrate: %d bpm\n", max30100_pulse_oximeter_heartrate);
                printk("MAX30100: SpO2: %d %%\n", max30100_pulse_oximeter_spo2);
            }
        }
        else if (max30100_pulse_oximeter_state == PULSEOXIMETER_STATE_DETECTING)
        {
            max30100_pulse_oximeter_state = PULSEOXIMETER_STATE_IDLE;
            max30100_spo2_calculator_reset();
        }

        tsLastSample = time;
    }
    // Check current bias

    // Follower that adjusts the red led current in order to have comparable DC baselines between
    // red and IR leds. The numbers are really magic: the less possible to avoid oscillations
    if ((time - tsLastBiasCheck) > (CURRENT_ADJUSTMENT_PERIOD_MS))
    {
        bool changed = false;
        if (led_ir_ac_dcw - led_red_ac_dcw > 70000 && redLedPower < MAX30100_LED_CURR_50MA)
        {
            ++redLedPower;
            changed = true;
        }
        else if (led_red_ac_dcw - led_ir_ac_dcw > 70000 && redLedPower > 0)
        {
            --redLedPower;
            changed = true;
        }

        if (changed)
        {
            printk("MAX30100: Adjusting Red LED current to %d\n", redLedPower);
            max30100_set_leds_current(i2c_dev, IR_LED_CURRENT, redLedPower);
            tsLastCurrentAdjustment = time;
        }

        tsLastBiasCheck = time;
    }

    if ((time - tsLastTemperaturePoll) > (TEMPERATURE_SAMPLING_PERIOD_MS))
    {
        max30100_start_temperature(i2c_dev);
        while(!max30100_is_temperature_ready(i2c_dev)){
            max30100_beat_detector_reset();
        }       

        tsLastTemperaturePoll = time;
    }
}
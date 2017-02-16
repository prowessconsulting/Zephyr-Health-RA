#include <max30100_spo2_calculator.h>

static const uint8_t spO2LUT[43] = {100, 100, 100, 100, 99, 99, 99, 99, 99, 99, 98, 98, 98, 98,
                                    98, 97, 97, 97, 97, 97, 97, 96, 96, 96, 96, 96, 96, 95, 95,
                                    95, 95, 95, 95, 94, 94, 94, 94, 94, 93, 93, 93, 93, 93};

double irACValueSqSum;
double redACValueSqSum;
uint8_t beatsDetectedNum;
uint32_t samplesRecorded;
uint8_t spO2;

// Simple log
double log(double x)
{
    int i, j;
    double sum = 0.0f;
    double power;

    for (i = 1; i <= 16; i++)
    { 
        power = 1.0f;
        for (j = 0; j < i; j++)
        {
            power = power * ((x - 1.0f) / x); 
        }
        sum += (1.0f / i) * power; 
    }
    return sum;
}

void max30100_spo2_calculator_update(float led_ir_ac_value, float led_red_ac_value, bool beat_detected)
{
    irACValueSqSum += led_ir_ac_value * led_ir_ac_value;
    redACValueSqSum += led_red_ac_value * led_red_ac_value;
    ++samplesRecorded;

    if (beat_detected)
    {
        ++beatsDetectedNum;
        if (beatsDetectedNum == CALCULATE_EVERY_N_BEATS)
        {
            float acSqRatio = 100.0 * log(redACValueSqSum / samplesRecorded) / log(irACValueSqSum / samplesRecorded);
            uint8_t index = 0;

            if (acSqRatio > 66)
            {
                index = (uint8_t)acSqRatio - 66;
            }
            else if (acSqRatio > 50)
            {
                index = (uint8_t)acSqRatio - 50;
            }
            max30100_spo2_calculator_reset();

            spO2 = spO2LUT[index];
        }
    }
}

void max30100_spo2_calculator_reset()
{
    samplesRecorded = 0;
    redACValueSqSum = 0;
    irACValueSqSum = 0;
    beatsDetectedNum = 0;
    spO2 = 0;
}

uint8_t max30100_spo2_calculator_get_spo2()
{
    return spO2;
}

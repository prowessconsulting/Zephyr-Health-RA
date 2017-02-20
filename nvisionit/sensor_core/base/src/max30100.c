// This source code has been adapted from a reference implementation originally fo the Arduino platform.
// The orriginal source code is available here: https://github.com/oxullo/Arduino-MAX30100
// The source code is covered unter the GPL licence, which can be found here:

// This file handles the raw data comms from the device.
#include <max30100.h>

int max30100_init(struct device *i2c_dev)
{
    printk("MAX30100: Location: 0x%x\n", i2c_dev);

    printk("MAX30100: Setting mode\n");
    max30100_set_mode(i2c_dev, DEFAULT_MODE);
    printk("MAX30100: Setting LED pulse width\n");
    max30100_set_leds_pulse_width(i2c_dev, DEFAULT_PULSE_WIDTH);
    printk("MAX30100: Setting sampling rate\n");
    max30100_set_sampling_rate(i2c_dev, DEFAULT_SAMPLING_RATE);
    printk("MAX30100: Setting LED current\n");
    max30100_set_leds_current(i2c_dev, DEFAULT_IR_LED_CURRENT, DEFAULT_RED_LED_CURRENT);
    printk("MAX30100: Setting High res mode\n");
    max30100_set_highres_mode_enabled(i2c_dev, true);

    uint8_t whoami = 0;
    int ret = i2c_reg_read_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_REVISION_ID, &whoami);

    printk("MAX30100: Revision: %d\n", whoami);

    return 0;
}

void max30100_set_mode(struct device *i2c_dev, uint8_t modeConfig)
{
    i2c_reg_write_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_MODE_CONFIGURATION, modeConfig);
}

void max30100_start_temperature(struct device *i2c_dev)
{
    // Set up the read register
    uint8_t modeConfig;
    i2c_reg_read_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_MODE_CONFIGURATION, &modeConfig);
    modeConfig |= MAX30100_MC_TEMP_EN;

    i2c_reg_write_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_MODE_CONFIGURATION, modeConfig);
}

bool max30100_is_temperature_ready(struct device *i2c_dev)
{
    uint8_t tempReady;
    i2c_reg_read_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_MODE_CONFIGURATION, &tempReady);

    return !(tempReady & MAX30100_MC_TEMP_EN);
}

uint8_t temp_stor[2];

float max30100_get_temperature(struct device *i2c_dev)
{
    i2c_burst_read(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_TEMPERATURE_DATA_INT, temp_stor, 2);

    float temp = temp_stor[0];
    temp += temp_stor[1] * 0.0625f;

    return temp;
}

void max30100_set_leds_pulse_width(struct device *i2c_dev, uint8_t ledPulseWidth)
{
    uint8_t previous;
    i2c_reg_read_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_SPO2_CONFIGURATION, &previous);
    i2c_reg_write_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_SPO2_CONFIGURATION, (previous & 0xfc) | ledPulseWidth);
}

void max30100_set_sampling_rate(struct device *i2c_dev, uint8_t samplingRate)
{
    uint8_t previous;
    i2c_reg_read_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_SPO2_CONFIGURATION, &previous);
    i2c_reg_write_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_SPO2_CONFIGURATION, (previous & 0xe3) | (samplingRate << 2));
}

void max30100_set_leds_current(struct device *i2c_dev, uint8_t irLedCurrent, uint8_t redLedCurrent)
{
    i2c_reg_write_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_LED_CONFIGURATION, redLedCurrent << 4 | irLedCurrent);
}

void max30100_set_highres_mode_enabled(struct device *i2c_dev, bool enabled)
{
    uint8_t previous;
    i2c_reg_read_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_SPO2_CONFIGURATION, &previous);
    if (enabled)
    {
        i2c_reg_write_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_SPO2_CONFIGURATION, previous | MAX30100_SPC_SPO2_HI_RES_EN);
    }
    else
    {
        i2c_reg_write_byte(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_SPO2_CONFIGURATION, previous & ~MAX30100_SPC_SPO2_HI_RES_EN);
    }
}

void max30100_update(struct device *i2c_dev, uint8_t *buffer)
{
    i2c_burst_read(i2c_dev, MAX30100_I2C_ADDRESS, MAX30100_REG_FIFO_DATA, buffer, 4);
}

#include <sensor.h>
#include <string.h>

static const std::string tagMax{"Sensor"};

void Max30102::init()
{
    ESP_LOGI(tagMax.c_str(), "%s", "Start init max driver");

    sensorHandler = _i2cHelper.get_handler(kI2cAddress);
    if (sensorHandler != 0)
    {
        uint8_t pid = 0;
        uint8_t rev = 0;

        _i2cHelper.i2c_read_register(sensorHandler, Regs::Regs::PART_ID, &pid);
        _i2cHelper.i2c_read_register(sensorHandler, Regs::Regs::REV_ID, &rev);

        if (pid != kPartID || rev != kRevisionID)
        {
            PrintValue(tagMax, "pid", pid);
            PrintValue(tagMax, "rev", rev);

            ESP_LOGW(tagMax.c_str(), "Wrong product or revison id");
        }

        ESP_LOGI(tagMax.c_str(), "%s", "Init max driver done");

        memset((void *)led1Data, 0, kLedBufferSize * sizeof(uint32_t));
        memset((void *)led2Data, 0, kLedBufferSize * sizeof(uint32_t));
    }
    else
    {
        ESP_LOGI(tagMax.c_str(), "%s", "Init max driver failed");
        sensorHandler = 0;
    }
};

void Max30102::deinit()
{
    ESP_LOGI(tagMax.c_str(), "%s", "Deinit max driver");
    sensorHandler = 0;
};

void Max30102::start()
{
    SensorReset();
    vTaskDelay(kAfterResetTimeoutMs / portTICK_PERIOD_MS);
    SensorConfig();
    SensorStart();
};

void Max30102::stop()
{
    SensorStop();
};

size_t Max30102::readData(uint32_t *data)
{
    // read new data from fifo
    size_t amountOfSamples = readFromFifo();
    offsetInBuffer += amountOfSamples;

    if (offsetInBuffer != 0)
    {
        if ((offsetInBuffer > (kLedBufferSize - 64U)) ||
            ((xTaskGetTickCount() - LastSentResultTickCount) > pdMS_TO_TICKS(10000U)))
        {
            // calculate values from buffered value
            if (!(ambientLightOverflow() || IsFifoOverFlow()))
            {
                SensorResult result = calculate(led1Data, led2Data, offsetInBuffer);
                memcpy(data, (uint32_t *)&result, sizeof(SensorResult));
                LastSentResultTickCount = xTaskGetTickCount();
                return sizeof(SensorResult);
            }
            else
            {
                ESP_LOGE(tagMax.c_str(), "Ambient light or fifo overflow detected");
                return 0;
            }

            offsetInBuffer = 0;
            memset((void *)led1Data, 0, kLedBufferSize * sizeof(uint32_t));
            memset((void *)led2Data, 0, kLedBufferSize * sizeof(uint32_t));
        }

        if (offsetInBuffer > kLedBufferSize)
        {
            offsetInBuffer = 0;
        }
    }

    return 0;
}

SensorResult Max30102::calculate(uint32_t *led1Data, uint32_t *led2Data, size_t numSamplesRead)
{
    int32_t heartRate = 0;
    int8_t heartRateValid = 0;
    int32_t spo2 = 0;
    int8_t spo2Valid = 0;

    maxim_heart_rate_and_oxygen_saturation(led2Data, numSamplesRead, led1Data, &spo2, &spo2Valid, &heartRate, &heartRateValid);

    ESP_LOGI(tagMax.c_str(), "Calculated: heart=%ld/%d, spo2=%ld/%d, count=%u",
             heartRate, heartRateValid, spo2, spo2Valid, numSamplesRead);

    return (spo2Valid && heartRateValid) ? SensorResult{heartRate, spo2} : SensorResult{-1, -1};
}

void Max30102::SensorConfig(const SensorConfigStruct &config)
{
    // LED Pulse Amplitude Configuration
    // powerLevel = 0x02:0.4mA, 0x1F:6.4mA, 0x7F:25.4mA, 0xFF:50.0mA
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::LED1_PA,
                                                  config.powerLevel));
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::LED2_PA,
                                                  config.powerLevel));

    // set sample average, enable rollover, set (32 - 2) as an fifo almost full threshold
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::FIFO_CONFIG,
                                                  (static_cast<uint8_t>(config.sampleAverage) << 5) |
                                                      (1 << 4) | 0x09));
    // spo2 configuration register
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::SPO2_CONFIG,
                                                  (static_cast<uint8_t>(config.scaleWidth) << 5) |
                                                      (static_cast<uint8_t>(config.sampleRate) << 2) |
                                                      static_cast<uint8_t>(config.pulseWidth)));
}

void Max30102::SensorStart() const
{
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::FIFO_WR_PTR, 0));
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::FIFO_OVFLW, 0));
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::FIFO_RD_PTR, 0));

    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::MULTILED_CONFIG1,
                                                  (static_cast<uint8_t>(SensRegs::SlotConfig::SLOT_LED1_IR) << 4) |
                                                      static_cast<uint8_t>(SensRegs::SlotConfig::SLOT_LED1_RED)));
    // multi led mode and enable
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::MODE_CONFIG, 7));
}

void Max30102::SensorStop() const
{
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::MULTILED_CONFIG1, 0));
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, SensRegs::Regs::MODE_CONFIG, 1 << 7));
}

size_t Max30102::readFromFifo()
{
    uint8_t fifoWrite = 0;
    ESP_ERROR_CHECK(_i2cHelper.i2c_read_register(sensorHandler, SensRegs::Regs::FIFO_WR_PTR, &fifoWrite));
    PrintValue(tagMax, "Fifo values count", fifoWrite);
    size_t numSamples = 0;

    if (fifoWrite > 24)
    {
        for (size_t i = 0; i < fifoWrite; i++)
        {
            uint8_t data[6];

            ESP_ERROR_CHECK(_i2cHelper.i2c_read_mult_register(sensorHandler,
                                                              SensRegs::Regs::FIFO_DATA,
                                                              data, 6));

            led1Data[offsetInBuffer + i] = (data[0] << 16) | (data[1] << 8) | data[2];
            led2Data[offsetInBuffer + i] = (data[3] << 16) | (data[4] << 8) | data[5];

            // PrintValue(tagMax, "led1", led1Data[offsetInBuffer + i]);
            // PrintValue(tagMax, "led2", led2Data[offsetInBuffer + i]);
        }
        numSamples = fifoWrite;
    }

    return numSamples;
};

bool Max30102::IsFifoOverFlow() const
{
    uint8_t overFlow = 0;
    ESP_ERROR_CHECK(_i2cHelper.i2c_read_register(sensorHandler, SensRegs::Regs::FIFO_OVFLW, &overFlow));
    return (overFlow != 0);
}

bool Max30102::ambientLightOverflow() const
{
    uint8_t config = 0;
    ESP_ERROR_CHECK(_i2cHelper.i2c_read_register(sensorHandler, SensRegs::Regs::ISR_STAT1, &config));
    return ((1 << 5) & config);
}

void Max30102::SensorWakeUp() const
{
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, Regs::Regs::MODE_CONFIG, 0));
};

void Max30102::SensorReset() const
{
    ESP_ERROR_CHECK(_i2cHelper.i2c_write_register(sensorHandler, Regs::Regs::MODE_CONFIG, 1 << 6));
};
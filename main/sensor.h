#ifndef DRIVER_MAX30102_H
#define DRIVER_MAX30102_H

#include <stdint.h>

#include "i2c_helper.h"
#include "sensor_abstract.h"

#include "sensor_spo2_algorithm.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "xtensa/core-macros.h"

namespace Regs = SensRegs;

struct SensorResult
{
    int32_t pulse;
    int32_t saturation;
};

struct SensorConfigStruct
{
    uint8_t powerLevel;
    Regs::SampleAveraging sampleAverage;
    Regs::Spo2SampleRate sampleRate;
    Regs::PulseWidth pulseWidth;
    Regs::AdcFullScaleWidth scaleWidth;

    SensorConfigStruct() : powerLevel(0x1F),
                           sampleAverage(Regs::SampleAveraging::MAX30102_SAMPLE_AVERAGING_2),
                           sampleRate(Regs::Spo2SampleRate::MAX30102_SPO2_SAMPLE_RATE_100_HZ),
                           pulseWidth(Regs::PulseWidth::LED_PW_411US),
                           scaleWidth(Regs::AdcFullScaleWidth::SPO2_ADC_RGE_LSB_15PA63_FULLSCALE_4096NA)
    {
    }
};

class Max30102 : Sensor
{

public:
    static SensorResult calculate(uint32_t *led1Data, uint32_t *led2Data, size_t numSamplesRead);

    Max30102(I2CHelper &i2cHelper) : _i2cHelper(i2cHelper),
                                     sensorHandler(0),
                                     LastSentResultTickCount(0),
                                     offsetInBuffer(0) {};
    virtual ~Max30102() {}

    void init() override;
    void deinit() override;
    void start() override;
    void stop() override;
    size_t readData(uint32_t *data) override;

    bool isInitDone() const
    {
        return sensorHandler != 0;
    }

private:
    static constexpr uint8_t kI2cAddress = 0x57U;
    static constexpr auto kRevisionID = 3U;
    static constexpr auto kPartID = 21U;
    static constexpr auto kAfterResetTimeoutMs = 5000U;
    static constexpr auto kLedBufferSize = 255U;

    I2CHelper &_i2cHelper;
    i2c_master_dev_handle_t sensorHandler;
    TickType_t LastSentResultTickCount = 0;
    size_t offsetInBuffer = 0;

    uint32_t led1Data[kLedBufferSize];
    uint32_t led2Data[kLedBufferSize];

    void SensorConfig(const SensorConfigStruct &config = SensorConfigStruct());
    void SensorStart() const;
    void SensorStop() const;
    void SensorWakeUp() const;
    void SensorReset() const;

    bool IsFifoOverFlow() const;
    bool ambientLightOverflow() const;
    size_t readFromFifo();
};

#endif
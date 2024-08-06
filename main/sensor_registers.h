#ifndef DRIVER_REGISTERS_H
#define DRIVER_REGISTERS_H

#include <stdint.h>

namespace SensRegs
{
    enum class SampleAveraging
    {
        MAX30102_SAMPLE_AVERAGING_1 = 0x00,  /**< no averaging */
        MAX30102_SAMPLE_AVERAGING_2 = 0x01,  /**< averaging 2 */
        MAX30102_SAMPLE_AVERAGING_4 = 0x02,  /**< averaging 4 */
        MAX30102_SAMPLE_AVERAGING_8 = 0x03,  /**< averaging 8 */
        MAX30102_SAMPLE_AVERAGING_16 = 0x04, /**< averaging 16 */
        MAX30102_SAMPLE_AVERAGING_32 = 0x05, /**< averaging 32 */
    };

    enum class Mode
    {
        MAX30102_MODE_HEART_RATE = 0x02, /**< heart rate mode */
        MAX30102_MODE_SPO2 = 0x03,       /**< spo2 mode */
        MAX30102_MODE_MULTI_LED = 0x07,  /**< multi-led mode */
    };

    enum class InterruptStatus
    {
        MAX30102_INTERRUPT_STATUS_FIFO_FULL = 7,    /**< fifo almost full flag */
        MAX30102_INTERRUPT_STATUS_PPG_RDY = 6,      /**< new fifo data ready */
        MAX30102_INTERRUPT_STATUS_ALC_OVF = 5,      /**< ambient light cancellation overflow */
        MAX30102_INTERRUPT_STATUS_PWR_RDY = 0,      /**< power ready flag */
        MAX30102_INTERRUPT_STATUS_DIE_TEMP_RDY = 1, /**< internal temperature ready flag */
    };

    enum class max30102_interrupt_t
    {
        MAX30102_INTERRUPT_FIFO_FULL_EN = 7,    /**< fifo almost full enable */
        MAX30102_INTERRUPT_PPG_RDY_EN = 6,      /**< new fifo data ready enable */
        MAX30102_INTERRUPT_ALC_OVF_EN = 5,      /**< ambient light cancellation overflow enable */
        MAX30102_INTERRUPT_DIE_TEMP_RDY_EN = 1, /**< internal temperature enable */
    };

    enum class Spo2AdcRange
    {
        MAX30102_SPO2_ADC_RANGE_2048 = 0,  /**< range 2048 */
        MAX30102_SPO2_ADC_RANGE_4096 = 1,  /**< range 4096 */
        MAX30102_SPO2_ADC_RANGE_8192 = 2,  /**< range 8192 */
        MAX30102_SPO2_ADC_RANGE_16384 = 3, /**< range 16384 */
    };

    enum class Spo2SampleRate
    {
        MAX30102_SPO2_SAMPLE_RATE_50_HZ = 0,   /**< 50Hz */
        MAX30102_SPO2_SAMPLE_RATE_100_HZ = 1,  /**< 100Hz */
        MAX30102_SPO2_SAMPLE_RATE_200_HZ = 2,  /**< 200Hz */
        MAX30102_SPO2_SAMPLE_RATE_400_HZ = 3,  /**< 400Hz */
        MAX30102_SPO2_SAMPLE_RATE_800_HZ = 4,  /**< 800Hz */
        MAX30102_SPO2_SAMPLE_RATE_1000_HZ = 5, /**< 1000Hz */
        MAX30102_SPO2_SAMPLE_RATE_1600_HZ = 6, /**< 1600Hz */
        MAX30102_SPO2_SAMPLE_RATE_3200_HZ = 7, /**< 3200Hz */
    };

    enum class AdcResolution
    {
        MAX30102_ADC_RESOLUTION_15_BIT = 0, /**< 15 bits */
        MAX30102_ADC_RESOLUTION_16_BIT = 1, /**< 16 bits */
        MAX30102_ADC_RESOLUTION_17_BIT = 2, /**< 17 bits */
        MAX30102_ADC_RESOLUTION_18_BIT = 3, /**< 18 bits */
    };

    enum class Leds
    {
        MAX30102_LED_NONE = 0, /**< time slot is disabled */
        MAX30102_LED_RED = 1,  /**< enable red */
        MAX30102_LED_IR = 2,   /**< enable ir */
    };

    enum class Slots
    {
        MAX30102_SLOT_1 = 0, /**< slot 1 */
        MAX30102_SLOT_2 = 1, /**< slot 2 */
        MAX30102_SLOT_3 = 2, /**< slot 3 */
        MAX30102_SLOT_4 = 3, /**< slot 4 */
    };

    enum class PulseWidth
    {
        LED_PW_69US = 0, /**< 15 bits adc resolution */
        LED_PW_118US,    /**< 16 bits adc resolution */
        LED_PW_215US,    /**< 17 bits adc resolution */
        LED_PW_411US,    /**< 18 bits adc resolution */
    };

    enum class AdcFullScaleWidth
    {
        SPO2_ADC_RGE_LSB_7PA81_FULLSCALE_2048NA = 0,
        SPO2_ADC_RGE_LSB_15PA63_FULLSCALE_4096NA,
        SPO2_ADC_RGE_LSB_32PA25_FULLSCALE_8192NA,
        SPO2_ADC_RGE_LSB_62PA5_FULLSCALE_16384NA,
    };

    enum class SlotConfig
    {
        SLOT_DISABLE = 0,
        SLOT_LED1_RED = 1,
        SLOT_LED1_IR = 1,
    };

    enum class Regs
    {
        ISR_STAT1 = 0,
        ISR_STAT2 = 1,
        FIFO_WR_PTR = 0x04,
        FIFO_OVFLW = 0x05,
        FIFO_RD_PTR = 0x06,
        FIFO_DATA = 0x07,
        FIFO_CONFIG = 0x08,
        MODE_CONFIG = 0x09,
        SPO2_CONFIG = 0x0A,
        LED1_PA = 0x0C,
        LED2_PA = 0x0D,
        MULTILED_CONFIG1 = 0x11,
        MULTILED_CONFIG2 = 0x12,
        REV_ID = 0xFE,
        PART_ID = 0xFF,
    };
};
#endif
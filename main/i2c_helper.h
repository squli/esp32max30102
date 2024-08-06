#ifndef I2C_HELPER_H
#define I2C_HELPER_H

#include "driver/i2c_master.h"
#include "sensor_registers.h"

#include <string>
#include <mutex>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

class I2CHelper
{
public:
    I2CHelper() : i2c_mst_config{0, GPIO_NUM_21, GPIO_NUM_22, I2C_CLK_SRC_DEFAULT, 7, 0, 0, true},
                  bus_handle(NULL)
    {
        if (bus_handle == 0)
        {
            ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
        }
    };

    ~I2CHelper()
    {
        ESP_ERROR_CHECK(i2c_del_master_bus(bus_handle));
    };

    i2c_master_dev_handle_t get_handler(uint8_t i2cAdress);
    esp_err_t i2c_write_register(i2c_master_dev_handle_t dev_handle,
                                 SensRegs::Regs registerNum,
                                 uint8_t registerValue);
    esp_err_t i2c_read_register(i2c_master_dev_handle_t dev_handle,
                                SensRegs::Regs registerNum,
                                uint8_t *registerValue);
    esp_err_t i2c_read_mult_register(i2c_master_dev_handle_t dev_handle,
                                     SensRegs::Regs registerNum,
                                     uint8_t *registerValue,
                                     size_t dataAmount);

private:
    static constexpr auto kI2cInstanceSpeed = 400000U;
    static constexpr auto i2cTimeoutMs = 16U;

    const i2c_master_bus_config_t i2c_mst_config;
    i2c_master_bus_handle_t bus_handle;
    std::mutex i2cMutex;
};

#endif
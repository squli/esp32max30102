#include "i2c_helper.h"

const std::string tag{"I2Chelper"};

i2c_master_dev_handle_t I2CHelper::get_handler(uint8_t i2cAdress)
{
    i2c_master_dev_handle_t dev_handle;
    i2c_device_config_t dev_cfg;

    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = i2cAdress;
    dev_cfg.scl_speed_hz = kI2cInstanceSpeed;

    i2cMutex.lock();
    ESP_ERROR_CHECK(i2c_master_probe(bus_handle, i2cAdress, i2cTimeoutMs));
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
    i2cMutex.unlock();

    return dev_handle;
}

esp_err_t I2CHelper::i2c_write_register(i2c_master_dev_handle_t dev_handle,
                                        SensRegs::Regs registerNum,
                                        uint8_t registerValue)
{
    uint8_t data_wr[2] = {static_cast<uint8_t>(registerNum), registerValue};

    // ESP_LOGI(tag.c_str(), "write reg=%d dev_handle=%d", int(registerNum), int(dev_handle));
    i2cMutex.lock();
    esp_err_t err = i2c_master_transmit(dev_handle,
                                        data_wr,
                                        2,
                                        i2cTimeoutMs);
    i2cMutex.unlock();
    return err;
};

esp_err_t I2CHelper::i2c_read_register(i2c_master_dev_handle_t dev_handle,
                                       SensRegs::Regs registerNum,
                                       uint8_t *registerValue)
{
    uint8_t buf = static_cast<uint8_t>(registerNum);

    // ESP_LOGI(tag.c_str(), "read reg=%d dev_handle=%d", int(registerNum), int(dev_handle));

    i2cMutex.lock();
    esp_err_t err = i2c_master_transmit_receive(dev_handle,
                                                &buf,
                                                1,
                                                registerValue,
                                                1,
                                                i2cTimeoutMs);
    i2cMutex.unlock();

    return err;
};

esp_err_t I2CHelper::i2c_read_mult_register(i2c_master_dev_handle_t dev_handle,
                                            SensRegs::Regs registerNum,
                                            uint8_t *registerValue,
                                            size_t dataAmount)
{
    uint8_t buf = static_cast<uint8_t>(registerNum);

    // ESP_LOGI(tag.c_str(), "readm reg=%d dev_handle=%d", int(registerNum), int(dev_handle));

    i2cMutex.lock();
    esp_err_t err = i2c_master_transmit_receive(dev_handle,
                                                &buf,
                                                1,
                                                registerValue,
                                                dataAmount,
                                                i2cTimeoutMs);
    i2cMutex.unlock();

    return err;
};
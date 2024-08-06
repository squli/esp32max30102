#ifndef ABSTRACT_SENSOR_H
#define ABSTRACT_SENSOR_H

#include <string>
#include <stdint.h>
#include "esp_log.h"

class Sensor
{
public:
    virtual void init() = 0;
    virtual void deinit() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual size_t readData(uint32_t *data) = 0;

    static void PrintValue(const std::string tag, const std::string name, int32_t value)
    {
        if (value < 0xFFFFFFFF)
        {
            ESP_LOGI(tag.c_str(), "%s=%s", name.c_str(), std::to_string(value).c_str());
        }
    }
};

#endif
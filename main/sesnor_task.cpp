#include <string.h>

#include "sensor_task.h"
#include "sensor.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <freertos/semphr.h>

extern QueueHandle_t SensorCommandsQueueHandle;
extern QueueHandle_t SensorResultsQueueHandle;

static I2CHelper i2cHelper;
static Max30102 max30102(i2cHelper);

extern "C" void SensorTask(void *parameters)
{
    bool isEnabled = false;

    for (;;)
    {
        SensorCommands command = SensorCommands::SENSOR_IDLE;
        if (xQueueReceive(SensorCommandsQueueHandle, &command, 0))
        {
            if (command == SensorCommands::SENSOR_RUN && !isEnabled)
            {
                max30102.init();
                max30102.start();
                isEnabled = true;
            }
            else if (command == SensorCommands::SENDOR_STOP && isEnabled)
            {
                max30102.stop();
                max30102.deinit();
                isEnabled = false;
            }
            else
            {
                esp_log_write(ESP_LOG_ERROR, "Sensor", "Wrong command");
            }
        }

        if (isEnabled && max30102.isInitDone())
        {
            SensorResult result{0, 0};
            size_t amount = max30102.readData((uint32_t *)&result);
            if (amount != 0)
            {
                xQueueSend(SensorResultsQueueHandle, &result, pdMS_TO_TICKS(0));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10U));
    }
}
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <freertos/semphr.h>

#include "ble_task.h"
#include "sensor_task.h"

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include "esp_log.h"

#define QUEUE_SIZE (16U)

static const char *TAG = "main";

static constexpr auto kQueueTimeoutMs = 2U;

QueueHandle_t SensorCommandsQueueHandle;
QueueHandle_t SensorResultsQueueHandle;
TaskHandle_t xTaskBuffer;

/**
 * @brief This function should handle new data written into ctrl characteristic
 * 
 * @param new_data 
 */
void ble_ctrl_char_write_callback(uint8_t new_data) {
     SensorCommands command;

    if (new_data == 1) {
       command = SensorCommands::SENSOR_RUN;
    } else if (new_data == 0) {
        command = SensorCommands::SENDOR_STOP;        
    } else {
        ESP_LOGW(TAG, "Invalid command received: %u", new_data);
        return;
    }

    xQueueSend(SensorCommandsQueueHandle, &command, pdMS_TO_TICKS(kQueueTimeoutMs));
    ESP_LOGI(TAG, "new command=%u handled", new_data);
    return;
}

extern "C" void app_main()
{
    // Queue for commands from BLE-task to sensor task
    SensorCommandsQueueHandle = xQueueCreate(QUEUE_SIZE, sizeof(uint32_t));

    // Queue for sensor results from sensor task to BLE-task
    SensorResultsQueueHandle = xQueueCreate(QUEUE_SIZE, sizeof(SensorResult));
        
    gatt_svr_ctrl_char_handler_ptr ptr = &ble_ctrl_char_write_callback;

    init_ble(ptr);
    xTaskCreate(SensorTask, "SnsTask", 4096U, nullptr, tskIDLE_PRIORITY, &xTaskBuffer);

    for (;;)
    {
        SensorResult result;
        if (xQueueReceive(SensorResultsQueueHandle, &result, pdMS_TO_TICKS(kQueueTimeoutMs)) == true)
        {
            ESP_LOGI(TAG, "heartRate=%ld, spo2=%ld", result.pulse, result.saturation);
            gatt_svr_update_data(result.pulse, result.saturation);
        }
        vTaskDelay(pdMS_TO_TICKS(1U));
    }
}
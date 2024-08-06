#ifndef SENSOR_TASK_H
#define SENSOR_TASK_H

#include "sensor.h"

/// @brief
enum class SensorCommands
{
    SENSOR_IDLE = 0,
    SENSOR_RUN,
    SENDOR_STOP,
};

#ifdef __cplusplus
extern "C" {
#endif

void SensorTask(void *parameters);

#ifdef __cplusplus
}
#endif

#endif
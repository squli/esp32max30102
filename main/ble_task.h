#ifndef BLE_TASK_H
#define BLE_TASK_H

#include "ble_service.h"

#ifdef __cplusplus
extern "C" {
#endif

void bleprph_host_task(void *param);
void init_ble(gatt_svr_ctrl_char_handler_ptr f);

#ifdef __cplusplus
}
#endif

#endif
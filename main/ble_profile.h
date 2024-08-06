#ifndef BLE_PROFILE_H
#define BLE_PROFILE_H

#include "host/ble_uuid.h"
#include "host/ble_gatt.h"

// BLE service with 3 characteristics - heartrate, spo2, control
static const ble_uuid128_t gatt_svr_svc_uuid =
    BLE_UUID128_INIT(0x2d, 0x71, 0xa2, 0x59, 0xb4, 0x58, 0xc8, 0x12,
                     0x99, 0x99, 0x43, 0x95, 0x12, 0x2f, 0x46, 0x59);

/********************************************************************
    A characteristic that for heartrate: read, notify, indicate
********************************************************************/
uint8_t gatt_svr_chr_heartrate_val;
uint16_t gatt_svr_chr_heartrate_val_handle;
static const ble_uuid128_t gatt_svr_chr_heartrate_uuid =
    BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x11, 0x11, 0x11, 0x11,
                     0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x33);

uint8_t gatt_svr_dsc_heartrate_val;
static const ble_uuid128_t gatt_svr_dsc_heartrate_uuid =
    BLE_UUID128_INIT(0x01, 0x01, 0x01, 0x01, 0x12, 0x12, 0x12, 0x12,
                     0x23, 0x23, 0x23, 0x23, 0x34, 0x34, 0x34, 0x34);

/********************************************************************
    A characteristic that for spo2: read, notify, indicate
********************************************************************/
uint8_t gatt_svr_chr_spo2_val;
uint16_t gatt_svr_chr_spo2_val_handle;
static const ble_uuid128_t gatt_svr_chr_spo2_uuid =
    BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x12, 0x12, 0x12, 0x12,
                     0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x33);

uint8_t gatt_svr_dsc_spo2_val;
const ble_uuid128_t gatt_svr_dsc_spo2_uuid =
    BLE_UUID128_INIT(0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12,
                     0x23, 0x23, 0x23, 0x23, 0x34, 0x34, 0x34, 0x34);

/********************************************************************
    A characteristic that for control: write, read
********************************************************************/
uint8_t gatt_svr_chr_ctrl_val;
uint16_t gatt_svr_chr_ctrl_val_handle;
static const ble_uuid128_t gatt_svr_chr_ctrl_uuid =
    BLE_UUID128_INIT(0x00, 0x00, 0x00, 0x00, 0x12, 0x12, 0x12, 0x12,
                     0x22, 0x22, 0x22, 0x22, 0x33, 0x33, 0x33, 0x35);

uint8_t gatt_svr_dsc_ctrl_val;
const ble_uuid128_t gatt_svr_dsc_ctrl_uuid =
    BLE_UUID128_INIT(0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x12,
                     0x23, 0x23, 0x23, 0x23, 0x34, 0x34, 0x34, 0x36);

/********************************************************************/

static int gatt_svc_access(uint16_t conn_handle, 
                           uint16_t attr_handle, 
                           struct ble_gatt_access_ctxt *ctxt, void *arg);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        /*** Service ***/
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_uuid.u,

        .characteristics = (struct ble_gatt_chr_def[])
        { {
                .uuid = &gatt_svr_chr_heartrate_uuid.u,
                .access_cb = gatt_svc_access,
                .descriptors = (struct ble_gatt_dsc_def[])
                { {
                      .uuid = &gatt_svr_dsc_heartrate_uuid.u,
                      .att_flags = BLE_ATT_F_READ,
                      .access_cb = gatt_svc_access,
                    }, {
                      0,
                    }
                },
                .flags = BLE_GATT_CHR_F_READ | 
                         BLE_GATT_CHR_F_NOTIFY | 
                         BLE_GATT_CHR_F_INDICATE,
                .val_handle = &gatt_svr_chr_heartrate_val_handle,
            }, {
                .uuid = &gatt_svr_chr_spo2_uuid.u,
                .access_cb = gatt_svc_access,
                .descriptors = (struct ble_gatt_dsc_def[])
                { {
                      .uuid = &gatt_svr_dsc_spo2_uuid.u,
                      .att_flags = BLE_ATT_F_READ,
                      .access_cb = gatt_svc_access,
                    }, {
                      0,
                    }
                },
                .flags = BLE_GATT_CHR_F_READ | 
                         BLE_GATT_CHR_F_NOTIFY | 
                         BLE_GATT_CHR_F_INDICATE,
                .val_handle = &gatt_svr_chr_spo2_val_handle,
            }, {
                .uuid = &gatt_svr_chr_ctrl_uuid.u,
                .access_cb = gatt_svc_access,
                .descriptors = (struct ble_gatt_dsc_def[])
                { {
                      .uuid = &gatt_svr_dsc_ctrl_uuid.u,
                      .att_flags = BLE_ATT_F_READ,
                      .access_cb = gatt_svc_access,
                    }, {
                      0,
                    }
                },
                .flags = BLE_GATT_CHR_F_READ | 
                         BLE_GATT_CHR_F_WRITE | 
                         BLE_GATT_CHR_F_NOTIFY | 
                         BLE_GATT_CHR_F_INDICATE,
                .val_handle = &gatt_svr_chr_ctrl_val_handle,                 
            }, {
                0,
            }
        },
    },

    {
        0,
    },
};

#endif //BLE_PROFILE_H
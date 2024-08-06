/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "ble_task.h"
#include "services/ans/ble_svc_ans.h"
#include "ble_profile.h"

static gatt_svr_ctrl_char_handler_ptr ctrl_func = NULL;

static int gatt_svr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
                          void *dst, uint16_t *len);
static int gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                           struct ble_gatt_access_ctxt *ctxt, void *arg);
static int gatt_svr_read_chr_callback(struct ble_gatt_access_ctxt *ctxt,
                                      uint16_t attr_handle);
static int gatt_svr_write_chr_callback(struct ble_gatt_access_ctxt *ctxt,
                                       uint16_t attr_handle,
                                       uint8_t *value);
static int gatt_svr_read_chr_descr_callback(struct ble_gatt_access_ctxt *ctxt,
                                            const ble_uuid_t *uuid);

static int gatt_svr_write(struct os_mbuf *om, uint16_t min_len, uint16_t max_len,
                          void *dst, uint16_t *len)
{
    uint16_t om_len = OS_MBUF_PKTLEN(om);
    if (om_len < min_len || om_len > max_len)
    {
        return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    int rc = ble_hs_mbuf_to_flat(om, dst, max_len, len);
    if (rc != 0)
    {
        return BLE_ATT_ERR_UNLIKELY;
    }

    return 0;
}

static int gatt_svr_read_chr_callback(struct ble_gatt_access_ctxt *ctxt,
                                      uint16_t attr_handle)
{
    int rc = 0;

    if (attr_handle == gatt_svr_chr_heartrate_val_handle)
    {
        rc = os_mbuf_append(ctxt->om,
                            &gatt_svr_chr_heartrate_val,
                            sizeof(gatt_svr_chr_heartrate_val));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    else if (attr_handle == gatt_svr_chr_spo2_val_handle)
    {
        rc = os_mbuf_append(ctxt->om,
                            &gatt_svr_chr_spo2_val,
                            sizeof(gatt_svr_chr_spo2_val));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    else if (attr_handle == gatt_svr_chr_ctrl_val_handle)
    {
        rc = os_mbuf_append(ctxt->om,
                            &gatt_svr_chr_ctrl_val,
                            sizeof(gatt_svr_chr_ctrl_val));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

static int gatt_svr_write_chr_callback(struct ble_gatt_access_ctxt *ctxt,
                                       uint16_t attr_handle,
                                       uint8_t *value)
{
    int rc = 0;

    if (attr_handle == gatt_svr_chr_ctrl_val_handle)
    {
        rc = gatt_svr_write(ctxt->om,
                            sizeof(*value),
                            sizeof(*value),
                            value, NULL);
        ble_gatts_chr_updated(attr_handle);
        MODLOG_DFLT(INFO, "Notification/Indication scheduled for all subscribed peers.\n");

        // make a call of the handler
        ctrl_func(*value);

        return rc;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

static int gatt_svr_read_chr_descr_callback(struct ble_gatt_access_ctxt *ctxt,
                                            const ble_uuid_t *uuid)
{
    int rc = 0;

    if (ble_uuid_cmp(uuid, &gatt_svr_dsc_heartrate_uuid.u) == 0)
    {
        rc = os_mbuf_append(ctxt->om,
                            &gatt_svr_dsc_heartrate_val,
                            sizeof(gatt_svr_chr_heartrate_val));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    else if (ble_uuid_cmp(uuid, &gatt_svr_dsc_spo2_uuid.u) == 0)
    {
        rc = os_mbuf_append(ctxt->om,
                            &gatt_svr_dsc_spo2_val,
                            sizeof(gatt_svr_chr_spo2_val));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    else if (ble_uuid_cmp(uuid, &gatt_svr_dsc_ctrl_uuid.u) == 0)
    {
        rc = os_mbuf_append(ctxt->om,
                            &gatt_svr_dsc_ctrl_val,
                            sizeof(gatt_svr_chr_ctrl_val));
        return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

/**
 * Access callback whenever a characteristic/descriptor is read or written to.
 * Here reads and writes need to be handled.
 * ctxt->op tells weather the operation is read or write and
 * weather it is on a characteristic or descriptor,
 * ctxt->dsc->uuid tells which characteristic/descriptor is accessed.
 * attr_handle give the value handle of the attribute being accessed.
 * Accordingly do:
 *     Append the value to ctxt->om if the operation is READ
 *     Write ctxt->om to the value if the operation is WRITE
 **/
static int
gatt_svc_access(uint16_t conn_handle, uint16_t attr_handle,
                struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    const ble_uuid_t *uuid = NULL;

    switch (ctxt->op)
    {
    case BLE_GATT_ACCESS_OP_READ_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE)
        {
            MODLOG_DFLT(INFO, "Characteristic read; conn_handle=%d attr_handle=%d\n",
                        conn_handle, attr_handle);
        }
        else
        {
            MODLOG_DFLT(INFO, "Characteristic read by NimBLE stack; attr_handle=%d\n",
                        attr_handle);
        }
        uuid = ctxt->chr->uuid;

        if (0 != gatt_svr_read_chr_callback(ctxt, attr_handle))
        {
            goto unknown;
        }
        return 0;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE)
        {
            MODLOG_DFLT(INFO, "Characteristic write; conn_handle=%d attr_handle=%d",
                        conn_handle, attr_handle);
        }
        else
        {
            MODLOG_DFLT(INFO, "Characteristic write by NimBLE stack; attr_handle=%d",
                        attr_handle);
        }
        uuid = ctxt->chr->uuid;

        if (0 != gatt_svr_write_chr_callback(ctxt, attr_handle, &gatt_svr_chr_ctrl_val))
        {
            goto unknown;
        }
        return 0;

    case BLE_GATT_ACCESS_OP_READ_DSC:
        if (conn_handle != BLE_HS_CONN_HANDLE_NONE)
        {
            MODLOG_DFLT(INFO, "Descriptor read; conn_handle=%d attr_handle=%d\n",
                        conn_handle, attr_handle);
        }
        else
        {
            MODLOG_DFLT(INFO, "Descriptor read by NimBLE stack; attr_handle=%d\n",
                        attr_handle);
        }
        uuid = ctxt->dsc->uuid;
        if (0 != gatt_svr_read_chr_descr_callback(ctxt, uuid))
        {
            goto unknown;
        }
        return 0;

    case BLE_GATT_ACCESS_OP_WRITE_DSC:
        MODLOG_DFLT(INFO, "Descriptor write not supported\n");
        goto unknown;

    default:
        goto unknown;
    }

unknown:
    /* Unknown characteristic/descriptor;
     * The NimBLE host should not have called this function;
     */
    assert(0);
    return BLE_ATT_ERR_UNLIKELY;
}

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg)
{
    char buf[BLE_UUID_STR_LEN];

    switch (ctxt->op)
    {
    case BLE_GATT_REGISTER_OP_SVC:
        MODLOG_DFLT(DEBUG, "registered service %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->svc.svc_def->uuid, buf),
                    ctxt->svc.handle);
        break;

    case BLE_GATT_REGISTER_OP_CHR:
        MODLOG_DFLT(DEBUG, "registering characteristic %s with "
                           "def_handle=%d val_handle=%d\n",
                    ble_uuid_to_str(ctxt->chr.chr_def->uuid, buf),
                    ctxt->chr.def_handle,
                    ctxt->chr.val_handle);
        break;

    case BLE_GATT_REGISTER_OP_DSC:
        MODLOG_DFLT(DEBUG, "registering descriptor %s with handle=%d\n",
                    ble_uuid_to_str(ctxt->dsc.dsc_def->uuid, buf),
                    ctxt->dsc.handle);
        break;

    default:
        assert(0);
        break;
    }
}

int gatt_svr_init(void)
{
    int rc = 0;

    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_svc_ans_init();

    rc = ble_gatts_count_cfg(gatt_svr_svcs);
    if (rc != 0)
    {
        return rc;
    }

    rc = ble_gatts_add_svcs(gatt_svr_svcs);
    if (rc != 0)
    {
        return rc;
    }

    gatt_svr_chr_heartrate_val = 0x99;
    gatt_svr_chr_spo2_val = 0x99;
    gatt_svr_chr_ctrl_val = 0;

    return 0;
}

void gatt_svr_update_data(uint8_t heartrate, uint8_t spo2) {    
    gatt_svr_chr_heartrate_val = heartrate;
    gatt_svr_chr_spo2_val = spo2;
}

void gatt_svr_set_ctrl_char_handler(gatt_svr_ctrl_char_handler_ptr f) {
    ctrl_func = f;
}
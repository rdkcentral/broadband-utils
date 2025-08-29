/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @file gw_lan_refresh.c
 * @brief Main program entry for the Gateway Lan Refresh daemon
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <rbus.h>
#include <pthread.h>

#include "ccsp/ccsp_hal_ethsw.h"

#define CONSOLE_LOG_FILE "/rdklogs/logs/Consolelog.txt.0"

#define DBG_PRINT(fmt ...)     {\
    FILE     *fp        = NULL;\
    fp = fopen ( CONSOLE_LOG_FILE, "a+");\
    if (fp)\
    {\
        fprintf(fp,fmt);\
        fclose(fp);\
    }\
}\


#define SLEEP_TIME 4

pthread_mutex_t gw_lan_mutex = PTHREAD_MUTEX_INITIALIZER;

void refresh_external_switch()
{
    CCSP_HAL_ETHSW_PORT port;
    INT max_phy_eth_ports = 0;

    /* Total 3 LAN ports*/
    max_phy_eth_ports = CCSP_HAL_ETHSW_EthPort3;

    for (port = CCSP_HAL_ETHSW_EthPort1; port <= max_phy_eth_ports; port++)
    {
        // Disable the port
        DBG_PRINT("%s(): setting admin status down for port %d\n", __FUNCTION__, port);
        CcspHalEthSwSetPortAdminStatus(port, CCSP_HAL_ETHSW_AdminDown);
    }

    sleep(SLEEP_TIME);

    for (port = CCSP_HAL_ETHSW_EthPort1; port <= max_phy_eth_ports; port++)
    {
        // Enable the port
        DBG_PRINT("%s(): setting admin status up for port %d\n", __FUNCTION__, port);
        CcspHalEthSwSetPortAdminStatus(port, CCSP_HAL_ETHSW_AdminUp);
    }
}

void set_wifi_refresh_dm(rbusHandle_t handle, const char* param) {
    pthread_mutex_lock(&gw_lan_mutex);

    rbusError_t err = rbus_setBoolean(handle, param, true);
    if (err != RBUS_ERROR_SUCCESS) {
        DBG_PRINT("Failed to set %s: %d\n", param, err);
    } else {
        DBG_PRINT("Successfully set %s\n", param);
    }

    pthread_mutex_unlock(&gw_lan_mutex);
}

void refresh_wifi()
{
    rbusHandle_t handle;
    rbusError_t err;
    err = rbus_open(&handle, "gw_lan_refresh");
    if (err != RBUS_ERROR_SUCCESS) {
        DBG_PRINT("Failed to initialize RBus: %d\n", err);
    }
    /* dis-associate connected wifi clients */
    set_wifi_refresh_dm(handle,"Device.WiFi.AccessPoint.1.X_CISCO_COM_KickAssocDevices");
    set_wifi_refresh_dm(handle,"Device.WiFi.AccessPoint.2.X_CISCO_COM_KickAssocDevices");

    rbus_close(handle);
}

int main(int argc, char **argv)
{
    if (argc == 2) {
        if (strncmp (argv[1], "ethsw", strlen ("ethsw")) == 0) {
            DBG_PRINT ("[%s] calling to update ethsw setting \n",argv[0]);
            refresh_external_switch();
        }else if (strncmp (argv[1], "wifi", strlen ("wifi")) == 0) {
            DBG_PRINT ("[%s] calling to update wifi setting \n",argv[0]);
            refresh_wifi();
        }
        else {
            
        }
    }else {
        DBG_PRINT("gw_lan_refresh \n");
        refresh_external_switch();
        refresh_wifi();
    }
    return 0;
}

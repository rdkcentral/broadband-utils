/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2026 RDK Management
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

#include "mtlsUtils.h"
#include "rfc_common.h"

#include "ccsp/platform_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BUF_SIZE 64

#if defined(RDKB_SUPPORT)
std::string getErouterMac()
{
    char macBuf[BUF_SIZE] = {0};
    std::string erouterMac;

    if (platform_hal_GetBaseMacAddress(macBuf) == RETURN_OK) {
        erouterMac = macBuf;
        while (!erouterMac.empty() && (erouterMac.back() == '\n' || erouterMac.back() == ' ')) {
            erouterMac.pop_back();
        }
    }

    RDK_LOG(RDK_LOG_INFO, LOG_RFCMGR, "[%s] Recevied eRouterMac(eSTBMac): %s\n", __FUNCTION__,erouterMac.c_str());
    return erouterMac;
}

std::string geteCMMac()
{
    std::string macAddress;
    char macBuf[BUF_SIZE] = {0};

    if (platform_hal_GetBaseMacAddress(macBuf) == RETURN_OK) {
        macAddress = macBuf;
        while (!macAddress.empty() && (macAddress.back() == '\n' || macAddress.back() == ' ')) {
            macAddress.pop_back();
        }
    }

    RDK_LOG(RDK_LOG_INFO, LOG_RFCMGR, "[%s] Received eCM MAC: %s\n", __FUNCTION__, macAddress.c_str());
    return macAddress;
}
#endif
  
#ifdef __cplusplus
}
#endif

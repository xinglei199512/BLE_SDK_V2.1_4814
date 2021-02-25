/**
 ****************************************************************************************
 *
 * @file   mesh_node_config.c
 *
 * @brief  .
 *
 * @author  jiachuang
 * @date    2018-10-08 11:12
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"
#include "mesh_node_config.h"
#include "sdk_mesh_config.h"
#include "mesh_core_api.h"
#include "bx_config.h"
#include <string.h>
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */




/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief   config mesh features
 * @return  void
 ****************************************************************************************
 */
void mesh_node_feature_config_init(void)
{
    mesh_core_params_t param;
    //element num
    param.element_num = ELEMENT_NUM;
    mesh_core_params_set(MESH_CORE_PARAM_ELEMENT_NUM , &param);
    //relay
    param.relay = MESH_SUPPORT_RELAY;
    mesh_core_params_set(MESH_CORE_PARAM_FEATURE_RELAY , &param);
    //proxy
    param.proxy = MESH_SUPPORT_PROXY;
    mesh_core_params_set(MESH_CORE_PARAM_FEATURE_PROXY , &param);
    //friend
    param.friend = MESH_SUPPORT_FRIEND;
    mesh_core_params_set(MESH_CORE_PARAM_FEATURE_FRIENT , &param);
    //low_power
    param.low_power = MESH_SUPPORT_LOW_POWER;
    mesh_core_params_set(MESH_CORE_PARAM_FEATURE_LOW_POWER , &param);
    //network_transmit
    param.network_transmit.count            = TRANSMIT_DEFAULT_COUNT;
    param.network_transmit.interval_steps   = TRANSMIT_DEFAULT_INTERVAL;
    mesh_core_params_set(MESH_CORE_PARAM_PROPERTY_NETWORK_TRANSMIT , &param);
    //relay transmit
    param.relay_transmit.count              = TRANSMIT_DEFAULT_RELAY_COUNT;
    param.relay_transmit.interval_steps     = TRANSMIT_DEFAULT_RELAY_INTERVAL;
    mesh_core_params_set(MESH_CORE_PARAM_PROPERTY_RELAY_TRANSMIT , &param);
    //proxy service adv transmit
    param.proxy_services_transmit.count              = TRANSMIT_PROXY_SERVICE_COUNT;
    param.proxy_services_transmit.interval_steps     = TRANSMIT_PROXY_SERVICE_INTERVALT;
    param.proxy_services_transmit.period             = TRANSMIT_PROXY_SERVICE_PERIOD;
    mesh_core_params_set(MESH_CORE_PARAM_PROPERTY_PROXY_SERVICES_TRANSMIT , &param);

    param.unprov_beacon_transmit.count              = UPROV_BEACON_DEFAULT_COUNT;
    param.unprov_beacon_transmit.interval_steps     = UPROV_BEACON_DEFAULT_INTERVAL;
    param.unprov_beacon_transmit.period             = UPROV_BEACON_DEFAULT_PERIOD;
    mesh_core_params_set(MESH_CORE_PARAM_PROPERTY_UNPROV_BEACON_TRANSMIT , &param);

    //set mesh node name
    {
        nvds_tag_len_t device_name_length = NVDS_LEN_DEVICE_NAME;
        uint8_t *p_name = (uint8_t *)pvPortMalloc(NVDS_LEN_DEVICE_NAME*sizeof(uint8_t));
        mesh_core_params_t mac_param;
        uint8_t mac_addr[BD_ADDR_LEN] = {0};
        uint8_t max_name_len = MESH_NAME_MAX_LEN -4;//name+4 mac

        mac_param.mac_address = mac_addr;
        mesh_core_params_get(MESH_CORE_PARAM_MAC_ADDRESS,&mac_param);

        if(p_name != NULL)
        {
            nvds_get(NVDS_TAG_DEVICE_NAME, &device_name_length, p_name);
            device_name_length = (nvds_tag_len_t)strlen((const char *)p_name);
            if(max_name_len < device_name_length) device_name_length = max_name_len;

            p_name[device_name_length+0] = '-';
            p_name[device_name_length+1] = (mac_addr[0] / 100 ) % 10 + '0';
            p_name[device_name_length+2] = (mac_addr[0] / 10  ) % 10 + '0';
            p_name[device_name_length+3] = (mac_addr[0] / 1   ) % 10 + '0';

            param.name.len = device_name_length + 4;

            memcpy(param.name.name,p_name,param.name.len);
            vPortFree(p_name);
        }
        else
        {
            uint8_t s_name[] = BX_DEV_NAME;
            param.name.len = sizeof(s_name);
            if(MESH_NAME_MAX_LEN < param.name.len) param.name.len = MESH_NAME_MAX_LEN;
            memcpy(param.name.name,s_name,param.name.len);
        }



        mesh_core_params_set(MESH_CORE_PARAM_NAME , &param);

    }
}






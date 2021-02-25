/****************************************************************************************
 *
 * @file mesh_core_api.c
 *
 * @brief mesh core api for user.
 *
 * Copyright (C) Apollo 2018-2023
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "mesh_core_api.h"
#include "arch.h"
#include "node_save.h"
#include "nvds_in_ram.h"
#include "osapp_utils.h"
#include "provisioning_s.h"
#include "mesh_node_base.h"
#include "mesh_sched.h"
/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/**
 * @brief mesh node properties
 */
typedef struct
{
    /// normal network layer transmit
    adv_transmit_state_t network_transmit;
    /// network layer retransmit
    adv_transmit_state_t relay_transmit;
    /// mesh property proxy services adv transmit
    adv_transmit_state_t proxy_services_transmit;
    /// unprov beacon adv transmit
    adv_transmit_state_t unprov_beacon_transmit;
} mesh_properties_t;

/**
 * @brief mesh node features
 */
typedef struct
{
    /// relay feature status
    mesh_feature_stat_t relay;
    /// proxy feature status
    mesh_feature_stat_t proxy;
    /// friend feature status
    mesh_feature_stat_t friend;
    /// low_power feature status
    mesh_feature_stat_t low_power;
}mesh_features_t;

/**
 * @brief mesh node structure
 */
typedef struct
{
    /// the element buff of the node
    mesh_elmt_t elmt[ELEMENT_NUM];
    /// the number of the element
    uint8_t element_num;
    /// mesh node features
    mesh_features_t feature;
    /// mesh node properties
    mesh_properties_t properties;
    /// mesh name
    mesh_name_t name;
    /// mesh role
    mesh_role_t role;
    /// device is provisioned
    bool is_provisioned;
    /// node is provisioner
    bool is_provisioner;
}mesh_node_t;
/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * MACROS
 ****************************************************************************************
 */

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */


/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static mesh_node_t mesh_node;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief  Fast get the element of the node.
 * @return The element of the node.
 ****************************************************************************************
 */
uint8_t get_element_num(void)
{
    return mesh_node.element_num;
}

/**
 ****************************************************************************************
 * @brief  Fast get the first element pointer of the node.
 * @return The first element pointer of the node.
 ****************************************************************************************
 */
mesh_elmt_t * get_mesh_elmt(void)
{
    return mesh_node.elmt;
}

/**
 ****************************************************************************************
 * @brief The mesh core params get.
 *
 * @param[in] type       The mesh core params type which to be get.
 * @param[out] p_param   Pointer to the param data.
 *
 * @return err  the set status  @ref MESH_CORE_ERRORS.
 *
 ****************************************************************************************
 */
err_t mesh_core_params_get(mesh_core_param_types_t type,mesh_core_params_t * p_param)
{
    err_t err = MESH_CORE_SUCCESS;

    if(p_param)
    {
        switch(type)
        {
            case MESH_CORE_PARAM_MESH_ROLES:
                p_param->role = mesh_node.role;
                break;
            case MESH_CORE_PARAM_IS_PROVISIONED :
                p_param->is_provisioned = mesh_node.is_provisioned;
                break;
            case MESH_CORE_PARAM_IS_PROVISIONER:
                p_param->is_provisioner = mesh_node.is_provisioner;
                break;
            case MESH_CORE_PARAM_MAC_ADDRESS:
                tools_gap_public_bd_addr_get(p_param->mac_address);
                break;
            case MESH_CORE_PARAM_NAME:
                p_param->name = mesh_node.name;
                break;
            case MESH_CORE_PARAM_ELEMENT_POINTER:
                p_param->mesh_element = mesh_node.elmt;
                break;
            case MESH_CORE_PARAM_FEATURE_RELAY:
                p_param->relay = mesh_node.feature.relay;
                break;
            case MESH_CORE_PARAM_FEATURE_PROXY:
                p_param->proxy = mesh_node.feature.proxy;
                break;
            case MESH_CORE_PARAM_FEATURE_FRIENT:
                p_param->friend = mesh_node.feature.friend;
                break;
            case MESH_CORE_PARAM_FEATURE_LOW_POWER:
                p_param->low_power = mesh_node.feature.low_power;
                break;
            case MESH_CORE_PARAM_PROPERTY_NETWORK_TRANSMIT:
                p_param->network_transmit = mesh_node.properties.network_transmit;
                break;
            case MESH_CORE_PARAM_PROPERTY_RELAY_TRANSMIT:
                p_param->relay_transmit = mesh_node.properties.relay_transmit;
                break;
            case MESH_CORE_PARAM_PROPERTY_PROXY_SERVICES_TRANSMIT:
                p_param->proxy_services_transmit = mesh_node.properties.proxy_services_transmit;
                break;
            case MESH_CORE_PARAM_ELEMENT_NUM:
                p_param->element_num = mesh_node.element_num;
                break;
            case MESH_CORE_PARAM_PROPERTY_UNPROV_BEACON_TRANSMIT:
                p_param->unprov_beacon_transmit = mesh_node.properties.unprov_beacon_transmit;
                break;

            default:
                err = MESH_CORE_ERROR_NOT_FOUND;
                break;
        }
    }
    else
    {
        err = MESH_CORE_ERROR_NULL;
    }

    return err;
}
/**
 ****************************************************************************************
 * @brief The mesh core params set.
 *
 * @param[in] type      The mesh core params type which to be set.
 * @param[in] p_param   Pointer to the param data.
 *
 * @return err  the set status  @ref MESH_CORE_ERRORS.
 *
 ****************************************************************************************
 */
err_t mesh_core_params_set(mesh_core_param_types_t type,const mesh_core_params_t * p_param)
{
    err_t err = MESH_CORE_SUCCESS;
	LOG(3,"p_param:%x    type:%x\n",p_param,type);
    if(p_param)
    {
        switch(type)
        {
            case MESH_CORE_PARAM_MESH_ROLES:
                mesh_node.role = p_param->role;
                if(p_param->role == MESH_ROLE_CONFIG_SERVER)
                {
                    mesh_app_system_init_server();
                }
                else
                {
                    mesh_app_system_init_client();
                }
                break;
            case MESH_CORE_PARAM_IS_PROVISIONED :
                mesh_node.is_provisioned = p_param->is_provisioned;
                node_save_misc_is_provisioned();
                break;
            case MESH_CORE_PARAM_IS_PROVISIONER:
                mesh_node.is_provisioner = p_param->is_provisioner;
                node_save_misc_is_provisioner();
                break;
            case MESH_CORE_PARAM_MAC_ADDRESS:
                //BLOCKED
                //nvds_put(NVDS_TAG_BD_ADDRESS, NVDS_LEN_BD_ADDRESS, p_param->mac_address);
                break;
            case MESH_CORE_PARAM_NAME:
            {
                //BLOCKED
                static uint8_t first_set_name=1;
                mesh_node.name = p_param->name;
                if(first_set_name ==1)  first_set_name=0;
                else    nvds_put(NVDS_TAG_DEVICE_NAME, p_param->name.len, (uint8_t *)p_param->name.name);
            }
                break;
            case MESH_CORE_PARAM_ELEMENT_POINTER:
                //BLOCKED
                break;
            case MESH_CORE_PARAM_FEATURE_RELAY:
                mesh_node.feature.relay = p_param->relay;
                break;
            case MESH_CORE_PARAM_FEATURE_PROXY:
                mesh_node.feature.proxy = p_param->proxy;
                //action
                if(mesh_node.feature.proxy != MESH_FEATURE_ENABLED)
                {
                    provision_service_beacon_stop();
                }
                break;
            case MESH_CORE_PARAM_FEATURE_FRIENT:
                mesh_node.feature.friend = p_param->friend;
                break;
            case MESH_CORE_PARAM_FEATURE_LOW_POWER:
                mesh_node.feature.low_power = p_param->low_power;
                break;
            case MESH_CORE_PARAM_PROPERTY_NETWORK_TRANSMIT:
                mesh_node.properties.network_transmit = p_param->network_transmit;
                break;
            case MESH_CORE_PARAM_PROPERTY_RELAY_TRANSMIT:
                mesh_node.properties.relay_transmit = p_param->relay_transmit;
                break;
            case MESH_CORE_PARAM_PROPERTY_PROXY_SERVICES_TRANSMIT:
                mesh_node.properties.proxy_services_transmit = p_param->proxy_services_transmit;
                break;
            case MESH_CORE_PARAM_ELEMENT_NUM:
                mesh_node.element_num = p_param->element_num;
                break;
            case MESH_CORE_PARAM_PROPERTY_UNPROV_BEACON_TRANSMIT:
                mesh_node.properties.unprov_beacon_transmit = p_param->unprov_beacon_transmit;
                break;
            default:
                err = MESH_CORE_ERROR_NOT_FOUND;
                break;
        }
    }
    else
    {
        err = MESH_CORE_ERROR_NULL;
    }

    return err;
}
/**
 ****************************************************************************************
 * @brief The mesh core system set.
 *
 * @param[in] cmd      The mesh core system cmd which to be set.
 *
 * @return err  the set status  @ref MESH_CORE_ERRORS.
 *
 ****************************************************************************************
 */
err_t mesh_core_system_set(mesh_core_system_cmds_t cmd)
{
    err_t err = MESH_CORE_SUCCESS;

    switch(cmd)
    {
        case MESH_CORE_SYSTEM_HARD_RESTART :
            platform_reset(0);
            break;
        case MESH_CORE_SYSTEM_ALL_RESET :
            //1. stop scan
            mesh_sched_stop_scan(NULL);

            //2. clear mesh data in flash
            node_save_bxfs_hardware_erase_all();
            //node_save_mesh_reset();
            //3. restart
            platform_reset(0); 
            break;
        default:
            err = MESH_CORE_ERROR_NOT_FOUND;
            break;
    }

    return err;
}




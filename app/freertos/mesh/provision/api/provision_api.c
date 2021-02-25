/**
 ****************************************************************************************
 *
 * @file   provision_api.c
 *
 * @brief  .
 *
 * @author  jiachuang
 * @date    2018-09-14 17:15
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

#include "provision_api.h"

//#include "provisioner.h"
#include "unprov_device_intf.h"
#include "provisioner_intf.h"
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
/**/

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */



/**
 ****************************************************************************************
 * @brief   Init provision role and callback handle register.
 *
 * @param[in] role     provision role.
 * @param[in] handle   callback handle pointer.
 *
 ****************************************************************************************
 */
void provision_init(mesh_provsion_role_t role , mesh_prov_evt_cb_t handle)
{
    #if MESH_PROVISION_CLIENT_SUPPORT
    if (MESH_ROLE_PROVISIONER == role )
    {
        provisioner_role_init(handle);
    }
    #endif

    if(MESH_ROLE_UNPROV_DEVICE == role)
    {
        unprov_device_init(handle);
    }

}    


/**
 ****************************************************************************************
 * @brief   Send action to mesh stack.
 *
 * @param[in] type     The type of the action message.
 * @param[in] param    The parameter of the action message.
 *
 ****************************************************************************************
 */
void provision_action_send (mesh_prov_action_type_t type , mesh_prov_evt_param_t param)
{
    switch(type)
    {
        /*******PROVISIONER*******/
        //PROV_EVT_AUTH_INPUT_NUMBER
        case  PROV_ACTION_AUTH_INPUT_NUMBER_DONE : //input random number done
        //PROV_EVT_READ_PEER_PUBLIC_KEY_OOB
        case  PROV_ACTION_READ_PEER_PUBLIC_KEY_OOB_DONE :
        //PROV_EVT_BEACON
        case  PROV_ACTION_SET_LINK_OPEN :
        //PROV_EVT_CAPABILITIES
        case  PROV_ACTION_SEND_START_PDU :
        {            
            #if MESH_PROVISION_CLIENT_SUPPORT
            provisioner_action_send(type,param);
            #endif
        }
        break;
        /*******UNPROV DEVICE*******/
        //UNPROV_EVT_AUTH_INPUT_NUMBER
        case  UNPROV_ACTION_AUTH_INPUT_NUMBER_DONE : //input random number done
        {
            #if MESH_PROVISION_SERVER_SUPPORT
            unprov_device_action_send(type,param);
            #endif
        }
        break;
        default:break;
    }
}

/**
 ****************************************************************************************
 * @brief   Configure parameter to mesh stack.
 *
 * @param[in] type     The opcode of the configure parameter message.
 * @param[in] param    The parameter of the configuration message.
 *
 ****************************************************************************************
 */
void provision_config (mesh_prov_config_type_t opcode , mesh_prov_evt_param_t param)
{
    switch(opcode)
    {
        /*******PROVISIONER*******/
        case  PROV_SET_PROVISION_METHOD :
        case  PROV_SET_PRIVATE_KEY :
        case  PROV_SET_AUTH_STATIC :
        case  PROV_SET_DISTRIBUTION_DATA :
        case  PROV_SET_INVITE_DURATION :
        case  PROV_RESET :
        case  PROV_CLEAR_CACHE :
        {
            #if MESH_PROVISION_CLIENT_SUPPORT
            provisioner_config(opcode,param);
            #endif
        }
        break;

        /*******UNPROV DEVICE*******/
        case  UNPROV_SET_PROVISION_METHOD :
        case  UNPROV_SET_BEACON :
        case  UNPROV_SET_PRIVATE_KEY :
        case  UNPROV_SET_OOB_CAPS :
        case  UNPROV_SET_AUTH_STATIC :
        case  UNPROV_RESET :
        {

           unprov_device_config(opcode,param);

        }
        break;
        default:break;
    }
}


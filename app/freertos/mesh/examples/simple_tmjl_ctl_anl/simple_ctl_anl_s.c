/**
 ****************************************************************************************
 *
 * @file   simple_ctl_anl_s.c
 *
 * @brief  .
 *
 * @author  liuzy
 * @date    2018-09-25 16:18
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

#include "simple_ctl_anl_s.h"
#include "mesh_app_hal.h"
#include "provision_api.h"
#include "mesh_core_api.h"
#include "gap.h"
#include "node_setup.h"
#include "osapp_utils.h"
#include "flash.h"
#include "mesh_app_hal.h"
#include "node_setup.h"
#include "tmall_model_server.h"
#include "beacon.h"
#include "mesh_sched.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
// user private key
// user private key
#define  USER_UNPROV_PRIVATE_KEY        0x52,0x9a,0xa0,0x67,0x0d,0x72,0xcd,0x64,0x97,0x50,0x2e,0xd4,0x73,0x50,0x2b,0x03,\
                                        0x7e,0x88,0x03,0xb5,0xc6,0x08,0x29,0xa5,0xa3,0xca,0xa2,0x19,0x50,0x55,0x30,0xba


//#define USER_UNPROV_STATIC_AUTH_VAL       0x43,0x23,0xca,0xb4,0x51,0xd0,0x11,0xb0,0xb6,0x47,0xe4,0x4a,0xfd,0x0a,0x2c,0x79
#define USER_UNPROV_STATIC_AUTH_VAL       0x75,0x13,0xdd,0x13,0xc7,0x6a,0xda,0x00,0xd9,0xf0,0xd4,0x76,0xf2,0x30,0x66,0xbc
//#define USER_UNPROV_STATIC_AUTH_VAL       0x91,0x70,0xd3,0xcf,0xfd,0xbe,0x79,0xd5,0x60,0xf8,0xd6,0x4a,0xba,0x74,0x22,0x78
//#define USER_UNPROV_STATIC_AUTH_VAL       0x70,0xc2,0xda,0x36,0x3a,0x88,0xd1,0xad,0x3a,0x4e,0xba,0x2d,0x8f,0x6d,0x16,0x4f
//#define  USER_UNPROV_STATIC_AUTH_VAL    0x1e,0x7e,0x78,0xe0,0x9a,0xee,0xdc,0xcc,0xf6,0xb8,0x39,0x4b,0xb0,0xe4,0xfc,0xa2
//#define  USER_UNPROV_STATIC_AUTH_VAL    0xd8,0x75,0xc4,0x54,0x36,0xda,0xb1,0xb1,0x64,0xb6,0x66,0x54,0x66,0x5e,0x12,0x54
//#define  USER_UNPROV_STATIC_AUTH_VAL    0x54,0x12,0x5e,0x66,0x54,0x66,0xb6,0x64,0xb1,0xb1,0xda,0x36,0x54,0xc4,0x75,0xd8

#define  USER_UNPROV_CAPABILITIES_ALGORITHMS  0x0001

//#define  USER_UNPROV_BEACON_UUID          0xa8,0x01,\
										  0x51,\
									      0x80,0x0B,0x00,0x00,\
										  0xe0,0x07,0x03,0xca,0xd2,0x38,\
										  0x00,0x00,0x00  
#define  USER_UNPROV_BEACON_UUID          0xa8,0x01,\
										  0x51,\
									      0x80,0x0B,0x00,0x00,\
										  0xdf,0x07,0x03,0xca,0xd2,0x38,\
										  0x00,0x00,0x00  

//#define  USER_UNPROV_BEACON_UUID          0xa8,0x01,\
										  0x51,\
									      0x80,0x0B,0x00,0x00,\
										  0xde,0x07,0x03,0xca,0xd2,0x38,\
										  0x00,0x00,0x00  
//#define  USER_UNPROV_BEACON_UUID          0xa8,0x01,\
										  0x51,\
									      0x80,0x0B,0x00,0x00,\
										  0xdd,0x07,0x03,0xca,0xd2,0x38,\
										  0x00,0x00,0x00  
//#define  USER_UNPROV_BEACON_UUID          0xa8,0x01,\
										  0x51,\
									      0x80,0x0B,0x00,0x00,\
										  0xdc,0x07,0x03,0xca,0xd2,0x38,\
										  0x00,0x00,0x00  

//#define  USER_UNPROV_BEACON_UUID        0x00, 0x1B, \
                                        0xDC,\
                                        0x08,0x10,0x21,0x0B,\
                                        0x0E,0x0A,0x0C,0x00,0x0B,0x0E,\
                                        0x0A,0x0C,0x00 

#define  USER_UNPROV_BEACON_OOB         0x0000//0x4020
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
static unprov_user_data_t m_unprov_user=
{
        .unprov_private_key = {USER_UNPROV_PRIVATE_KEY},
        .static_value = {USER_UNPROV_STATIC_AUTH_VAL},
        .dev_capabilities =
        {
					.algorithms = USER_UNPROV_CAPABILITIES_ALGORITHMS,       
            .output_oob_action = 0,
            .input_oob_action = 0,
            .elements_num = ELEMENT_NUM,
            .public_key_type = 0,
						.static_oob_type = 1,     
            .output_oob_size = 0,
            .input_oob_size = 0,
        },
        .beacon =
        {
            .dev_uuid = {USER_UNPROV_BEACON_UUID},
//            .uri_hash = USER_UNPROV_BEACON_URL,   //è¿ä¸ªæä»ä¹ç¨
            .oob_info = USER_UNPROV_BEACON_OOB,
        }
};
/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static void mesh_unprov_evt_cb(mesh_prov_evt_type_t type , mesh_prov_evt_param_t param);
static void user_role_init(void);
static void unprov_data_init(void);
TimerHandle_t xAutoReloadTimer1 ;
extern bool success_respond;
extern bool indication_success;
/* user init*/
uint8_t present_onoff_0=0;
uint8_t target_onoff_0=0;
int nums=0;

void tmall_test()
{
  LOG(3,"tmall_test\n");
	/*
	if(nums==0){
	beacon_start();
  LOG(3,"beacon_start\n");
  mesh_sched_start_scan();
  LOG(3,"mesh_sched_start_scan\n");

  nums++;
	}else{
	 beacon_stop();
  LOG(3,"beacon_stop\n");
   mesh_sched_stop_scan(user_scan_stop_callback);
  LOG(3,"mesh_sched_stop_scan\n");
	nums=0;
	}	
		*/
	/*
	if(success_respond==0)//如果请求失败
	{
    tmall_test_time();	
	}
	if(present_onoff_0==0)
	{
		present_onoff_0=1;
    target_onoff_0=0;	
	}
	else
	{
	  present_onoff_0 =0;
		target_onoff_0=1;
	}
	//user_generic_onoff_status_publish(present_onoff_0,target_onoff_0);
	if(indication_success==0)//如果上报失败
	{
	tmall_test_indication_onoff(present_onoff_0);	
	}
   tmall_test_indication_key0();
	 */
}
void simple_hsl_ctl_server_init(void)
{
    //hal init
    hal_init_leds();
    hal_init_buttons();
    //role init
    user_role_init();
    hal_heartbeat_init();
	
	  xAutoReloadTimer1=xTimerCreate("xAutoReloadTimer1",pdMS_TO_TICKS(0x7530),pdTRUE,0,tmall_test);
    user_model_parameter_init();
}

static void user_role_init(void)
{
    //1.role init
    provision_init(MESH_ROLE_UNPROV_DEVICE,mesh_unprov_evt_cb);
    //2. data init
    unprov_data_init();
}
#define FLASH_TAG_SAVE_AUTH_VAL 0x2010
#define FLASH_TAG_SAVE_BEACON_UUID 0x2020

static void unprov_data_init(void)
{
    volatile mesh_prov_evt_param_t evt_param;

    uint8_t  bd_addr[GAP_BD_ADDR_LEN];
    uint8_t value[16];
    uint8_t value_len = 16;
#if 1
    if(flash_multi_read(FLASH_TAG_SAVE_AUTH_VAL, value_len, value) == NVDS_OK) {
        memcpy(m_unprov_user.static_value, value, value_len);
    }
    //show_buf("FLASH_TAG_SAVE_AUTH_VAL", value, value_len);
    if(flash_multi_read(FLASH_TAG_SAVE_BEACON_UUID, value_len, value) == NVDS_OK) {
        memcpy(m_unprov_user.beacon.dev_uuid, value, value_len);
    }
    //show_buf("FLASH_TAG_SAVE_BEACON_UUID", value, value_len);
#endif
    //get bd_addr
    mesh_core_params_t core_param;
    core_param.mac_address = bd_addr;

    mesh_core_params_get(MESH_CORE_PARAM_MAC_ADDRESS,&core_param);

    //copy mac to uuid
    memcpy(m_unprov_user.beacon.dev_uuid + 7, bd_addr, GAP_BD_ADDR_LEN);

    //1. Method of configuring network access
    evt_param.unprov.method = MESH_UNPROV_PROVISION_METHOD;
    provision_config(UNPROV_SET_PROVISION_METHOD,evt_param);
    //2. private key
    memcpy(m_unprov_user.unprov_private_key,bd_addr,GAP_BD_ADDR_LEN);
    evt_param.unprov.p_unprov_private_key = m_unprov_user.unprov_private_key;
    provision_config(UNPROV_SET_PRIVATE_KEY,evt_param);
    //3.static auth value
    evt_param.unprov.p_static_val = m_unprov_user.static_value;
    provision_config(UNPROV_SET_AUTH_STATIC,evt_param);
    //4.dev_capabilities
    evt_param.unprov.p_dev_capabilities = &m_unprov_user.dev_capabilities;
    provision_config(UNPROV_SET_OOB_CAPS,evt_param);
    //5.adv beacon
//    memcpy(m_unprov_user.beacon.dev_uuid,bd_addr,GAP_BD_ADDR_LEN);
    evt_param.unprov.p_beacon = &m_unprov_user.beacon;
    provision_config(UNPROV_SET_BEACON,evt_param);
}


void  notify_user_input_finish(uint8_t user_input)
{
    LOG(LOG_LVL_INFO,"user auth = %d\n",user_input);
    mesh_prov_evt_param_t evt_param;
    uint8_t auth_value[AUTHVALUE_LEN];
    memset(auth_value,0,AUTHVALUE_LEN);
    auth_value[AUTHVALUE_LEN -1] = user_input;
    evt_param.unprov.p_input_val = auth_value;
    provision_action_send(UNPROV_ACTION_AUTH_INPUT_NUMBER_DONE,evt_param);
}

/* unprovision device event callback function */
static void mesh_unprov_evt_cb(mesh_prov_evt_type_t type , mesh_prov_evt_param_t param)
{
    LOG(LOG_LVL_INFO,"mesh_unprov_evt_cb type : %d\n",type);

    switch(type)
    {
        case  UNPROV_EVT_INVITE_MAKE_ATTENTION : //(NO ACTION)
        {
            //m_unprov_user.attention_duration = param.unprov.attention_duration;
        }
        break;
        case  UNPROV_EVT_EXPOSE_PUBLIC_KEY :  //(NO ACTION)
        {

        }
        break;
        case  UNPROV_EVT_AUTH_INPUT_NUMBER : //alert input dialog
        {
             LOG(LOG_LVL_INFO,"input auth = \n");
        }
        break;
        case  UNPROV_EVT_AUTH_DISPLAY_NUMBER : //unprov_device expose random number //(NO ACTION)
        {
             LOG(LOG_LVL_INFO,"ooutput auth = ");
        }
        break;
        case  UNPROV_EVT_PROVISION_DONE :  //(NO ACTION)
        {
            delete_onoff_timer();
        }
        break;
        default:break;
    }
}



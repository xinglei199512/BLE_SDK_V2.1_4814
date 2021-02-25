/*
 * client_service_uart_test.h
 *
 *  Created on: 2018-5-14
 *      Author: huichen
 */

#ifndef FREERTOS_APP_MESH_MESH_TEST_CLIENT_SERVER_UART_TEST_H_
#define FREERTOS_APP_MESH_MESH_TEST_CLIENT_SERVER_UART_TEST_H_

#include "provision_base.h"
#include "beacon.h"
#include "unprov_device.h"
#include "provisioner.h"
#include "sdk_mesh_definitions.h"
#include "LOG.h"
#include "mesh_uart_config.h"
#include "mesh_env.h"
#include "mesh_model.h"
#include "mesh_uart_ctrl.h"
#include "config_client_events_api.h"
#ifdef MESH_TEST_UART_CLENT_SERVER
#include "uart_log_test.h"
#include "mesh_uart_ctrl.h"


///  ---------------   define
#define CLINET_SERVER_UART_ELEMENT_NUM 1
#define CLINET_SERVER_UART_MODEL_NUM 1
///  ---------------   typedef
enum{
    CLIENT_SERVER_MSG_CFG_CLIENT = 0X01,
    CLIENT_SERVER_MSG_CFG_SERVER = 0X02,
};
//pc and device  msg id (client server)
enum {
    // -- >device to pc
    //client
    TX_OP_CLIENT_SEQUENCE_NUM_REQ               = 0x01,//sequence_number  req
    TX_OP_CLIENT_DISTRIBUTION_REQ               = 0x02,
    TX_OP_CLIENT_APP_KEY_REQ                    = 0x03,
    TX_OP_CLIENT_ELEMENT_ADDR_REQ               = 0x04,
    TX_OP_CLIENT_MODEL_APPKEY_INDEX_REQ         = 0x05,
    TX_OP_CLIENT_CLIENT_DATA_INIT_DONE          = 0x06,
    TX_OP_CLIENT_DEVICE_KEY_VAL                 = 0x11,
    TX_OP_CLIENT_CLIENT_APPKEY_ADD_TX_REQ       = 0x21,
    TX_OP_CLIENT_MODEL_APP_BIND_TX_REQ          = 0x22,
    TX_OP_CLIENT_CLIENT_MODLE_CTRL_MSG_REQ      = 0x23,
    TX_OP_CLIENT_MODLE_CTRL_MSG_NOACK_ACK       = 0x24,
    TX_OP_CLIENT_MODLE_SUBSCRIP_UPDATA_TX_ACK   = 0x25,
    TX_OP_CLIENT_CONFIG_RELAY_Set_ACK           = 0x26,
    
    //server
    TX_OP_SERVER_SERVER_DATA_INIT_DONE          = 0x36,
    TX_OP_SERVER_DEVICE_KEY_VAL                 = 0x41,
    TX_OP_SERVER_LED_STATE_VAL                  = 0x42,

    // -- > pc to device
    RX_OP_CLIENT_CFG                            = 0x90,
    RX_OP_CLIENT_SEQUENCE_NUM_RSP               = 0x81,
    RX_OP_CLIENT_DISTRIBUTION_RSP               = 0x82,//设置分发数据
    RX_OP_CLIENT_APP_KEY_RSP                    = 0x83,//设置本地的appkey
    RX_OP_CLIENT_ELEMENT_ADDR_RSP               = 0x84,//设置本地元素地址
    RX_OP_CLIENT_MODEL_APPKEY_INDEX_RSP         = 0x85,
    RX_OP_CLIENT_CLIENT_APPKEY_ADD_TX_RSP       = 0xA1,
    RX_OP_CLIENT_MODEL_APP_BIND_TX_RSP          = 0xA2,
    RX_OP_CLIENT_CLIENT_MODLE_CTRL_MSG_RSP      = 0xA3,
    RX_OP_CLIENT_MODLE_CTRL_MSG_NOACK_RSP       = 0xA4,
    RX_OP_CLIENT_MODLE_SUBSCRIP_UPDATA_TX_RSP   = 0xA5,
    RX_OP_CLIENT_CONFIG_RELAY_Get               = 0xA6,
    RX_OP_CLIENT_CONFIG_RELAY_Set               = 0xA7,
    TX_OP_CLIENT_CONFIG_COMPDATA_Get            = 0xA8,
    RX_OP_CLIENT_CONFIG_RELAY_STATUS            = 0xB0,
    RX_OP_IV_UPDATE_TRANSACTION                 = 0xC0,
};
//client model crtl msg id (client server)
enum {
  CTRL_MSG_ID_GENERIC_ONOFF_SET = 0x0001,
  CTRL_MSG_ID_GENERIC_ONOFF_SET_DEF = 0x0002,
};
typedef struct
{
    uint16_t bound_netkey_idx;
    uint16_t global_idx;//mesh need(0-1024)
    uint8_t key[MESH_KEY_LENGTH];
}uart_log_client_server_appkey_t;
typedef struct
{
    uint16_t    id;
    uint16_t uni_addr[CLINET_SERVER_UART_ELEMENT_NUM];
}uart_log_client_server_element_t;
typedef struct
{
    uint8_t ele_idx;
    uint32_t model_id;
    uint16_t appkey_index;
}uart_log_client_server_model_t;
typedef struct
{
    provision_data_t distribution;
    uart_log_client_server_appkey_t appkey;
    uart_log_client_server_element_t element;
    uart_log_client_server_model_t model;
    uint8_t devkey[MESH_KEY_LENGTH];
    uint8_t devkey_s[MESH_KEY_LENGTH];//server dev key
}uart_log_client_server_owndata_t;
typedef struct
{
    uint16_t id;
    uint16_t addr;
    uint8_t *pdata;
}uart_log_client_ctrl_msg_t;
typedef struct
{
    uint8_t id;
    uint8_t OnOff;
}uart_log_server_led_ctrl_t;
///  ---------------  EXTERN value
/// extern


///  ---------------  EXTERN FUNCTION
//FUNCTION to system use
//==
extern void client_server_uart_msg_send(const uart_log_provision_data_t *param);

extern void config_client_appkey_add_success_cb(config_client_evt_param_t * p_param);
//==
extern void config_client_model_app_bind_success_cb(config_client_evt_param_t * p_param);
//==
extern void config_client_config_relay_status_success_cb(config_client_evt_param_t * p_param);
//==
extern void config_client_config_composition_data_status_success_cb(config_client_evt_param_t * p_param);
//==
extern void config_server_send_led_state(uart_log_server_led_ctrl_t led);

//FUNCTION to uesr use
//Common
extern void client_server_uart_init(QueueHandle_t handle);
//==
extern void client_server_uart_config_client_callback(uint8_t const *param,uint8_t len);
//==
extern void client_server_uart_system_msg_receive(const uart_log_provision_data_t *pmsg);
//==
extern void client_server_uart_config_client_msg_receive(const uart_log_provision_data_t *pmsg);
//==
extern void client_server_uart_provisioner_done_cb(uint8_t success , uint8_t reason);
//==
extern void client_server_uart_unprov_dev_done_cb(uint8_t success , uint8_t reason);

#endif /* MESH_TEST_UART_CLENT_SERVER */
#endif /* FREERTOS_APP_MESH_MESH_TEST_CLIENT_SERVER_UART_TEST_H_ */

/**
 ****************************************************************************************
 *
 * @file   provision_comm.c
 *
 * @brief  .
 *
 * @author  Administrator
 * @date    2019-03-16 18:30
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) BlueX Microelectronics 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "provision_comm.h"
#include "co_endian.h"
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define  PROV_AUTH_ALPHYNUMBIC_NUMBER 36
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
static uint8_t provisioning_pdu_size[Provisioning_PDU_Type_Max] =
{
    [Provisioning_Invite]           = INVITE_PDU_PARAMS_LEN,
    [Provisioning_Capabilities]     = CAPABILITIES_PDU_PARAMS_LEN,
    [Provisioning_Start]            = START_PDU_PARAMS_LEN,
    [Provisioning_Public_Key]       = PUBLIC_KEY_PDU_PARAMS_LEN,
    [Provisioning_Input_Complete]   = 0,
    [Provisioning_Confirmation]     = CONFIRMATION_PDU_PARAMS_LEN,
    [Provisioning_Random]           = RANDOM_PDU_PARAMS_LEN,
    [Provisioning_Data]             = DATA_PDU_PARAMS_LEN,
    [Provisioning_Complete]         = 0,
    [Provisioning_Failed]           = 1,
};

const char prov_evt_s[Invalid_Evt + 1][40] =
{
    "Timeout\n",
    "Tx_Normal_Cancel\n",
    "Tx_Timeout_Cancel\n",
    "Tx_Success\n",
    "Tx_Fail\n",
    "Ack_Finish\n",
    "Link_Close_Evt\n",
    "Link_Close_Tx_Finish\n",
    "Provisioning_Fail_Rx\n",
    "Start_Provision\n",
    "Provisioning_Capablities_Rx\n",
    "Provisioning_Invite_Rx\n",
    "Provisioning_Start_Rx\n",
    "Provisioning_Public_Key_Rx\n",  
    "Provisioning_Input_Complete_Rx\n",
    "Provisioning_Confirmation_Rx\n",
    "Start_Pdu_Check_Pass",
    "Start_Pdu_Check_Fail",
    "Public_Key_Check_Pass",
    "Public_Key_Check_Fail",
    "User_Input_Complete\n",
    "Confirmation_Calc_Done\n",
    "Provisioning_Random_Rx\n",
    "Confirmation_Check_Pass\n",
    "Confirmation_Check_Fail\n",
    "Provisioning_Data_Rx\n",
    "Session_Key_Calc_Done\n",
    "Decrypt_Success\n",
    "Decrypt_Fail\n",
    "Provision_Fail\n",
    "Encrypt_Data_done\n",
    "Devkey_Calc_done\n",
    "Provisioning_Complete_Rx\n",
    "Invalid_Evt\n"
};

const char prov_state_s[INVALID_STATUS + 1][40] =
{
    "IDLE_STATUS\n",
    "INVITE_TX_STATUS\n",
    "INVITE_TX_CANCEL_STATUS\n",
    "WAIT_INVITE_ACK_STATUS\n",
    "CAPABILITIES_TX_STATUS\n",
    "CAPABILITIES_TX_CANCEL_STATUS\n",
    "WAIT_CAPABILITIES_ACK_STATUS\n",
    "WAIT_CAPABILITIES_STATUS\n",
    "WAIT_INPUT_START_STATUS\n",
    "START_TX_STATUS\n",
    "START_TX_CANCEL_STATUS\n",
    "WAIT_START_ACK_STATUS\n",
    "WAIT_START_STATUS\n" , 
    "START_CHECK_STATUS\n",
    "CAPABILITIES_EXCHANGE_FINISH_STATUS\n",
    "WAIT_PEER_PUBLIC_KEY_STATUS\n",
    "WAIT_OOB_PUBLICKEY_STATUS\n",
    "PUBLIC_KEY_TX_STATUS\n",
    "PUBLIC_KEY_TX_CANCEL_STATUS\n",
    "PUBLIC_KEY_CHECK_STATUS\n",
    "WAIT_PUBLIC_KEY_ACK_STATUS\n",
    "WAIT_AUTH_INFO_STATUS\n",
    "INPUT_COMPELE_TX_STATUS\n",
    "INPUT_COMPELE_TX_CANCEL_STATUS\n",
    "WAIT_INPUT_COMPELE_ACK_STATUS\n",
    "WAIT_INPUT_COMPELE_STATUS\n",   
    "WAIT_PEER_CONFIRMATION_STATUS\n",
    "CONFIRM_CALC_STATUS\n",
    "CONFIRMATION_TX_STATUS\n",
    "CONFIRMATION_TX_CANCEL_STATUS\n",
    "WAIT_CONFIRMATION_ACK_STATUS\n",
    "WAIT_PEER_RANDOM_STATUS\n",
    "WAIT_RANDOM_ACK_STATUS\n",
    "CONFIRMATION_CHECK_STATUS\n",
    "RANDOM_TX_STATUS\n",
    "RANDOM_TX_CANCEL_STATUS\n",
    "CALC_SESSION_STATUS\n",
    "CALC_SESSION_CANCEL_STATUS\n",
    "WAITING_PROVISION_DATA_STATUS\n",
    "DECRY_PROVISION_DATA_STATUS\n",
    "ENCRYPT_PROVISIONING_DATA_STATUS\n",
    "ENCRYPT_PROVISIONING_CANCEL_STATUS\n",
    "PROVISIONING_DATA_TX_STATUS\n",
    "PROVISIONING_DATA_TX_CANCEL_STATUS\n",
    "WAIT_PROVISIONING_DATA_ACK_STATUS\n",
    "WAIT_PEER_COMPLETE_STATUS\n",
    "COMPLETE_TX_STATUS\n",
    "COMPLETE_TX_CANCEL_STATUS\n",
    "WAIT_COMPLETE_ACK_STATUS\n",
    "PROVISION_DONE_STATUS\n",
    "WAIT_TX_CANCEL_STATUS\n",
    "WAIT_ACK_CANCEL_STATUS\n",
    "WAIT_ASYNC_CALC_STATUS\n",
    "UNPROV_DEVKEY_CALC_STATUS",
    "LINK_CLOSE_TX_STATUS\n",
    "FAIL_PDU_TX_STATUS\n",
    "WAIT_RX_ACK_STATUS\n",
    "WAIT_CLOSE_LINK_STATUS\n",
    "INVALID_STATUS\n",
};
/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */

 Provisioning_PDU_Type provision_pkt_analysis(uint8_t * pkt, uint16_t length)
 {
       Provisioning_PDU_Type type =(Provisioning_PDU_Type) pkt[0];
       return type;
 }




 
Auth_Data_Type provision_get_auth_data_type(uint8_t auth_method, uint8_t         auth_action)
{
     if(auth_method == Output_OOB_Authentication)
     {
         if(auth_action <OUTPUT_NUMERIC )
         {
              return BINARY_AUTH;
         }
         else  if(auth_action <OUTPUT_ALPHANUMERIC )
         {
              return NUMERIC_AUTH;
         }
         else
         {
             return ALPHANUMERIC_AUTH;
         }
     }
     else 
     {
         if(auth_action <INPUT_NUMERIC )
         {
              return BINARY_AUTH;
         }
         else if(auth_action <INPUT_ALPHANUMERIC )
         {
              return NUMERIC_AUTH;
         }
         else
         {
             return ALPHANUMERIC_AUTH;
         }
     }
}

void  random_generate_count(uint8_t * data, uint8_t size)
{
    uint32_t tmp;
    uint8_t result;
    tmp = rand();
    result = tmp %(size +1);
    data[AUTHVALUE_LEN -1] = result;
}

void  random_generate_numeric(uint8_t * data, uint8_t size)
{
    static const uint32_t numeric_max[] =
    {
        0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000
    };

    uint32_t tmp;
    tmp = rand();
    tmp = co_bswap32((tmp % numeric_max[size]));
    memcpy(&data[AUTHVALUE_LEN - sizeof(tmp)], &tmp, sizeof(tmp));

}

void random_generate_alphanumeric(uint8_t * data, uint8_t size)
{
    static const char alphanumeric[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    uint32_t tmp;
    for (uint32_t i = 0; i < size; i++)
    {
        tmp = rand();
        tmp = tmp % PROV_AUTH_ALPHYNUMBIC_NUMBER ;
        data[i] = alphanumeric[tmp];
    }
}

 void provision_generate_auth(uint8_t * data, uint8_t size, Auth_Data_Type data_type)
 {
     if(BINARY_AUTH == data_type)
     {
         random_generate_count(data,size);
     }
     else  if(NUMERIC_AUTH == data_type)
     {
         random_generate_numeric(data,size);
     }
     else
     {
         random_generate_alphanumeric(data,size);
     }

 }


uint8_t *provisioning_pdu_build(uint8_t type, uint8_t *p_length)
{
    BX_ASSERT(type < Provisioning_PDU_Type_Max);
    uint8_t size = provisioning_pdu_size[type];
    *p_length = size + PARAMS_OFFSET_IN_PROVISIONING_PDU;
    uint8_t *data = pvPortMalloc(*p_length);
    data[0] = type;
    return data;
}

void provision_print_evt(prov_fsm_evt_t evt)
{
    LOG(LOG_LVL_INFO,prov_evt_s[evt]);
}

void provision_print_state(prov_fsm_state_t state)
{
    LOG(LOG_LVL_INFO,prov_state_s[state]);
}

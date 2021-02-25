#ifndef GENERIC_POWER_ONOFF_MSG_HANDLER__H
#define GENERIC_POWER_ONOFF_MSG_HANDLER__H
#include <stdint.h>
#include "generic_power_onoff_server.h"
#include "generic_power_onoff_client.h"
#include "model_msg.h"

#define GENERIC_POWER_ONOFF_OPCODE_OFFSET 0x211
#define GENERIC_POWER_ONOFF_SERVER_MODEL_ID 0x1006
#define GENERIC_POWER_ONOFF_SETUP_SERVER_MODEL_ID 0x1007
#define GENERIC_POWER_CLIENT_SERVER_MODEL_ID 0x1008
enum Generic_Power_OnOff_Opcode
{
    Generic_OnPowerUp_Get,
    Generic_OnPowerUp_Status,
    Generic_OnPowerUp_Set,
    Generic_OnPowerUp_Set_Unacknowledged,
    Generic_OnPowerUp_Opcode_Max,
};

extern msg_handler_model_t generic_power_onoff_model_msg[Generic_OnPowerUp_Opcode_Max];

#endif /* GENERIC_POWER_ONOFF_MSG_HANDLER__H */



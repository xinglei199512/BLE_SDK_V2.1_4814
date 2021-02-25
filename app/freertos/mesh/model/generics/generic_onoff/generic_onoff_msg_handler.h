#ifndef GENERIC_ONOFF_MSG_HANDLER_H_
#define GENERIC_ONOFF_MSG_HANDLER_H_

#include "generic_onoff_server.h"
#include "generic_onoff_client.h"


#define GENERIC_ONOFF_OPCODE_OFFSET 0x201
enum Generic_OnOff_Opcode
{
    Generic_OnOff_Get,
    Generic_OnOff_Set,
    Generic_OnOff_Set_Unacknowledged,
    Generic_OnOff_Status,
    Generic_OnOff_Opcode_Max
};

extern msg_handler_model_t generic_onoff_model_msg[Generic_OnOff_Opcode_Max];

#endif

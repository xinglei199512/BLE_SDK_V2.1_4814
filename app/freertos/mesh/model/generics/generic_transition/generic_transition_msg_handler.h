#ifndef GENERIC_TRANSITION_MSG_HANDLER__H
#define GENERIC_TRANSITION_MSG_HANDLER__H
#include "stdint.h"
#include "generic_transition_server.h"
#include "generic_transition_client.h"
#include "model_msg.h"

#define GENERIC_DEFAULT_TRANSITION_TIME_OPCODE_OFFSET 0x20D
#define GENERIC_DEFAULT_TRANSITION_TIME_SERVER_MODEL_ID 0x1004
#define GENERIC_DEFAULT_TRANSITION_TIME_CLIENT_MODEL_ID 0x1005
enum Generic_Default_Transition_Time_Opcode
{
    Generic_Default_Transition_Time_Get,
    Generic_Default_Transition_Time_Set,
    Generic_Default_Transition_Time_Set_Unacknowledged,
    Generic_Default_Transition_Time_Status,
    Generic_Default_Transition_Time_Opcode_Max
};

extern msg_handler_model_t generic_default_transition_time_model_msg[Generic_Default_Transition_Time_Opcode_Max];
#endif /* GENERIC_TRANSITION_MSG_HANDLER__H */



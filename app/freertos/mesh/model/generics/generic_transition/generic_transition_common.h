#ifndef GENERIC_TRANSITION_COMMON__H
#define GENERIC_TRANSITION_COMMON__H
#include "stdint.h"
#include "generic_transition_server.h"
#include "generic_transition_client.h"
#include "generic_transition_msg_handler.h"
#include "model_msg.h"


typedef struct
{
    uint8_t transition;
}__attribute((packed))generic_transition_msg_t;

#endif /* GENERIC_TRANSITION_COMMON__H */



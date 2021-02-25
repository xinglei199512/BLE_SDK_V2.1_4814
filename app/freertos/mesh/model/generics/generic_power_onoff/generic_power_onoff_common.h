#ifndef GENERIC_POWER_ONOFF_COMMON__H
#define GENERIC_POWER_ONOFF_COMMON__H
#include "stdint.h"
#include "generic_power_onoff_server.h"
#include "generic_power_onoff_client.h"
#include "generic_power_onoff_msg_handler.h"
#include "model_msg.h"


typedef struct
{
    uint8_t onpowerup;
}__attribute((packed))generic_onpowerup_set_t;

typedef generic_onpowerup_set_t generic_onpowerup_status_t;


#endif /* GENERIC_POWER_ONOFF_COMMON__H */



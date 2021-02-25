#ifndef TMALL_MODEL_COMMON__H
#define TMALL_MODEL_COMMON__H
#include "tmall_model_client.h"
#include "tmall_model_server.h"
#include "tmall_model_msg_handler.h"
#include "access_rx_process.h"
#include "model_msg.h"

typedef struct
{
    uint8_t tid;
    uint8_t value[];
}__attribute((packed))custom_tmall_msg_attr_indication_t;

typedef struct
{
    uint8_t num_of_value;
    uint16_t value_type;
    uint8_t len;
    uint8_t value;
}__attribute((packed))custom_tmall_msg_t;

typedef struct
{
    uint8_t tid;
    uint16_t attr_type;
    uint16_t value[3];
}__attribute((packed))custom_tmall_msg_set_t;

typedef struct
{
    uint8_t tid;
    uint16_t value_type;
    uint8_t value[3];
}__attribute((packed))custom_tmall_msg_status_t;

typedef struct
{
    uint8_t tid;
}__attribute((packed))custom_tmall_msg_attr_confirmation_t;
typedef struct
{
    uint8_t tid;
    uint16_t attr_type; 
}__attribute((packed))custom_tmall_time_t;
typedef struct
{
    uint8_t tid;
    uint16_t attr_type; 
	  bool  onoff;
}__attribute((packed))custom_tmall_indication_onoff_t;
typedef struct
{
    uint16_t attr_type;
	
}__attribute((packed))custom_tmall_indication_key0_t;
#endif /* TMALL_MODEL_COMMON__H */

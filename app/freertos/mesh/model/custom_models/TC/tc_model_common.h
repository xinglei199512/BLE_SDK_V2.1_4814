#ifndef TC_MODEL_COMMON__H
#define TC_MODEL_COMMON__H
#include "tc_model_client.h"
#include "tc_model_server.h"
#include "tc_model_msg_handler.h"
#include "access_rx_process.h"
#include "model_msg.h"
#define GAT_VALUE_NUM 200
#define SET_VALUE_NUM 200
typedef struct
{
    uint8_t tid;
    uint8_t value[];
}__attribute((packed))custom_tc_msg_attr_indication_t;

typedef struct
{
    uint8_t num_of_value;
    uint16_t value_type;
    uint8_t len;
    uint8_t value;
}__attribute((packed))custom_tc_msg_t;

typedef struct
{
    uint8_t value[SET_VALUE_NUM];
}__attribute((packed))custom_tc_msg_set_t;

typedef struct
{
    uint8_t value[SET_VALUE_NUM];
}__attribute((packed))custom_tc_msg_status_t;
typedef struct
{
    uint8_t value[1];
}__attribute((packed))custom_tc_msg_get_t;

typedef struct
{
    uint8_t value[GAT_VALUE_NUM];
}__attribute((packed))custom_tc_msg_get_value_t;

typedef struct
{
    uint8_t tid;
}__attribute((packed))custom_tc_msg_attr_confirmation_t;

#endif /* TC_MODEL_COMMON__H */

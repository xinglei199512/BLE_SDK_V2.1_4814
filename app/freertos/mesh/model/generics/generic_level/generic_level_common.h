#ifndef GENERIC_LEVEL_COMMON__H
#define GENERIC_LEVEL_COMMON__H
#include "generic_level_server.h"
#include "generic_level_client.h"
#include "generic_level_msg_handler.h"
#include "model_msg.h"

typedef struct
{
    uint16_t level;
    uint8_t tid;
}__attribute((packed))generic_level_msg_default_t;

typedef struct
{
    uint16_t level;
    uint8_t tid;
    uint8_t trans_time;
    uint8_t delay;
} __attribute((packed))generic_level_msg_set_t;

typedef struct
{
    int32_t delta_level;
    uint8_t tid;
}__attribute((packed))generic_delta_default_msg_set_t;

typedef struct
{
    int32_t delta_level;
    uint8_t tid;
    uint8_t trans_time;
    uint8_t delay;
}__attribute((packed))generic_delta_msg_set_t;

typedef struct
{
    uint16_t move_level;
    uint8_t tid;
}__attribute((packed))generic_move_default_msg_set_t;

typedef struct
{
    uint16_t move_level;
    uint8_t tid;
    uint8_t trans_time;
    uint8_t delay;
}__attribute((packed))generic_move_msg_set_t;

typedef struct
{
    uint16_t present_level;
    uint16_t target_level;
    uint8_t remaining_time;
}__attribute((packed))generic_level_msg_status_t;

typedef struct
{
    uint16_t present_level;
}__attribute((packed))generic_level_msg_default_status_t;

#endif /* GENERIC_LEVEL_COMMON__H */

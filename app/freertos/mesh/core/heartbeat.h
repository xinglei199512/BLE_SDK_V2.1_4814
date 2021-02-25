
/**
 ****************************************************************************************
 *
 * @file   heartbeat.h
 *
 * @brief  .
 *
 * @author  ZHAOYUNLIU
 * @date    2019-02-25 11:06
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2019
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup MESH_heartbeat_server_API Mesh heartbeat_server API
 * @ingroup MESH_API
 * @brief Mesh heartbeat_server  API
 *
 * @{
 ****************************************************************************************
 */

#ifndef APP_FREERTOS_MESH_MODEL_HEARTBEAT_HEARTBEAT_SERVER_H_
#define APP_FREERTOS_MESH_MODEL_HEARTBEAT_HEARTBEAT_SERVER_H_

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "mesh_errors.h"
#include "sdk_mesh_config.h"
#include "sdk_mesh_definitions.h"

#include "mesh_model.h"
#include "osapp_config.h"
#include "model_servers_events_api.h"
#include "access_rx_process.h"

typedef struct {
    uint16_t destination_addr;
    uint8_t  countlog;
    uint8_t  periodlog;
    uint8_t  ttl;
    uint16_t features;
    uint16_t netkeyindex;
}__attribute((packed))heartbeat_publication_set_t;

typedef struct {
    uint8_t  status;
    uint16_t destination_addr;
    uint8_t  countlog;
    uint8_t  periodlog;
    uint8_t  ttl;
    uint16_t features;
    uint16_t netkeyindex;
}__attribute((packed))heartbeat_publication_status_t;

typedef struct {
    uint16_t source_addr;
    uint16_t destination_addr;
    uint8_t  periodlog;
}__attribute((packed))heartbeat_subscription_set_t;

typedef struct {
    uint8_t  status;
    uint16_t source_addr;
    uint16_t destination_addr;
    uint8_t  periodlog;
    uint8_t  countlog;
    uint8_t  minhops;
    uint8_t  maxhops;
}__attribute((packed))heartbeat_subscription_status_t;

typedef struct{
    uint8_t status;
    uint16_t destination_addr;
    uint8_t pub_periodlog;
    uint16_t pub_count;
    uint8_t ttl;
    uint16_t features;
    uint16_t netkeyindex;
}heartbeat_publication_server_t;

typedef struct{
    uint8_t status;
    uint16_t source_addr;
    uint16_t destination_addr;
    uint8_t sub_periodlog;
    uint16_t sub_count;
    uint8_t minhops;
    uint8_t maxhops;
}heartbeat_subscription_server_t;

typedef struct {
    uint8_t initTTL:7,
            RFU:1;
    uint16_t features;
}__attribute((packed))heartbeat_msg_t;

uint16_t get_heartbeat_dst_addr(void);
void stop_heartbeat_massage(void);
void send_heartbeat_massage(uint16_t features);
void stop_heartbeat_massage(void);
void heartbeat_start_subscription_timer(void);
uint8_t heartbeat_calculate_16bit_to_8bit(uint16_t value_16);
uint32_t heartbeat_calculate_8bit_to_16bit(uint8_t value_8);
void heartbeat_rx_handler(upper_pdu_rx_t *ptr);
void heartbeat_start_publication_timer(void);
#endif /*APP_FREERTOS_MESH_MODEL_HEARTBEAT_HEARTBEAT_SERVER_H_ */

#ifndef MESH_BLE_TIME_H_
#define MESH_BLE_TIME_H_
#include <stdint.h>
typedef struct
{
    uint32_t time_cnt;
}ble_txrx_time_t;

void adv_rx_time_enqueue(uint32_t rtc_time);

ble_txrx_time_t *adv_rx_time_dequeue(void);

ble_txrx_time_t ble_current_time_get(void);

uint32_t ble_time_to_os_tick(ble_txrx_time_t ble_time);

ble_txrx_time_t mesh_time_add(ble_txrx_time_t a,ble_txrx_time_t b);

ble_txrx_time_t os_tick_to_ble_time(uint32_t ticks);
ble_txrx_time_t mesh_ms_to_ble_time(uint32_t ms);


int32_t mesh_time_diff(ble_txrx_time_t a,ble_txrx_time_t b);
#endif

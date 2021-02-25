#ifndef TIMER_WRAPPER_H_
#define TIMER_WRAPPER_H_
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
typedef void * mesh_timer_t;

mesh_timer_t mesh_timer_create(const char * const name,const uint32_t ticks,const bool auto_reload,void *const associated_data,void (*callback)(mesh_timer_t));

bool mesh_timer_active(mesh_timer_t timer);

void mesh_timer_start(mesh_timer_t timer);

void mesh_timer_stop(mesh_timer_t timer);

void mesh_timer_change_period(mesh_timer_t timer,uint32_t new_period);

void mesh_timer_delete(mesh_timer_t timer);

void mesh_timer_reset(mesh_timer_t timer);

void *mesh_timer_get_associated_data(mesh_timer_t timer);

void mesh_timer_set_associated_data(mesh_timer_t timer,void *associated_data);

const char *mesh_timer_get_name(mesh_timer_t timer);

uint32_t mesh_timer_get_period(mesh_timer_t timer);

uint32_t mesh_timer_get_expiry_time(mesh_timer_t timer);

uint32_t mesh_timer_get_remain_time(mesh_timer_t timer);

bool mesh_timer_active_then_reset(mesh_timer_t timer);
bool mesh_timer_cancel(mesh_timer_t timer);



#endif

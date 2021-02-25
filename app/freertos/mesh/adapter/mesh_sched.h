#ifndef MESH_SCHED_H_
#define MESH_SCHED_H_
#include <stdbool.h>
#include "rwip_config.h"
#include "rwip_task.h"
#include "task_id_patch.h"
#include "adv_bearer_tx.h"

void mesh_sched_stop_scan(void (*cb)());

void mesh_sched_start_scan(void);

void mesh_sched_adv_tx(mesh_adv_tx_t *ptr,bool high_priority);

void mesh_sched_init(void);

#endif

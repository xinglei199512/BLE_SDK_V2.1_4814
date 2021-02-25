#ifndef REASSEMBLY_H_
#define REASSEMBLY_H_
#include <stdint.h>
#include "timer_wrapper.h"
#include "friend.h"
#include "sdk_mesh_definitions.h"
#include "lower_rx_process.h"



reassembly_env_t *reassembly_env_alloc(bool local,lower_rx_env_t *env);

bool reassembly_env_release(reassembly_env_t *ptr);

#endif

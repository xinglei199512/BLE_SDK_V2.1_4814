#ifndef PERIPH_LOCK_H_
#define PERIPH_LOCK_H_
#include <stdint.h>
#include <stdbool.h>

typedef bool periph_lock_t;


bool periph_lock(periph_lock_t *periph_lock);

bool periph_unlock(periph_lock_t *periph_lock);

bool periph_lock_state_get(periph_lock_t *periph_lock);

#endif

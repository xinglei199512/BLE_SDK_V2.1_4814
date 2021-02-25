#include "periph_lock.h"
#include "ll.h"
#include "compiler_flag.h"

N_XIP_SECTION bool periph_lock(periph_lock_t *periph_lock)
{
    bool retval = false;
    ATOMIC_OP(
        if(*periph_lock == false)
        {
            *periph_lock = true;
            retval = true;
        }
    );
    return retval;
}

N_XIP_SECTION bool periph_unlock(periph_lock_t *periph_lock)
{
    bool retval = false;
    ATOMIC_OP(
        if(*periph_lock == true)
        {
            *periph_lock = false;
            retval = true;
        }
    );
    return retval;
}

N_XIP_SECTION bool periph_lock_state_get(periph_lock_t *periph_lock)
{
    return *periph_lock;
}

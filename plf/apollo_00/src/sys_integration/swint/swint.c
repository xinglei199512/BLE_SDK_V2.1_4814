#include "swint.h"
#include "apollo_00.h"

static void (*swint_cb)();

void SFT_IRQHandler(void)
{
    swint_cb();
}

void swint_req(void (*cb)())
{
    swint_cb = cb;
    NVIC_SetPendingIRQ(SFT_IRQn);
}


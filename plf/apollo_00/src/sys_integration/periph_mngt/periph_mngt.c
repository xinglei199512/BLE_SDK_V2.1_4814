#include <stddef.h>
#include "periph_mngt.h"
#include "uart_integration.h"
#include "log.h"

periph_inst_handle_t periph_domain_recovery_buf[PERIPH_DOMAIN_MAX];
periph_inst_handle_t cpu_domain_recovery_buf[CPU_DOMAIN_MAX];

periph_domain_stat_t periph_domain_stat;
cpu_domain_stat_t cpu_domain_stat;

static bool is_all_zero(uint8_t *src,uint32_t size)
{
    while(size--)
    {
        if(src[size]) return false;
    }
    return true;
}

bool periph_domain_busy(void)
{
    return is_all_zero((uint8_t *)&periph_domain_stat, sizeof(periph_domain_stat)) ? false : true;
}

bool cpu_domain_busy(void)
{
    return is_all_zero((uint8_t *)&cpu_domain_stat,sizeof(cpu_domain_stat)) ? false : true;
}


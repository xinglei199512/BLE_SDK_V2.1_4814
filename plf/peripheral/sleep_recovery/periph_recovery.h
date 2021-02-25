#ifndef PERIPH_RECOVERY_H_
#define PERIPH_RECOVERY_H_
#include "periph_mngt.h"

void recovery_list_add(periph_inst_handle_t *recovery_buf,uint8_t idx,periph_inst_handle_t hdl);

void recovery_list_remove(periph_inst_handle_t *recovery_buf,uint8_t idx);

void periph_recovery(periph_inst_handle_t *recovery_buf,uint8_t size);


#endif

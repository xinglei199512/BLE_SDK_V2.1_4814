/*
 * boot.h
 *
 *  Created on: 2016��4��3��
 *      Author: Administrator
 */

#ifndef BOOT_H_
#define BOOT_H_
#include <stdint.h>
/// RAM unloaded section
extern uint32_t unloaded_area_start;
#define RAM_UNLOADED_BASE   (&(unloaded_area_start))
extern uint32_t boot_params_start;
#define IMAGE_BOOT_PARAMS_BASE (&(boot_params_start))
extern uint32_t __initial_sp;
#define STACK_TOP __initial_sp
#endif /* BOOT_H_ */

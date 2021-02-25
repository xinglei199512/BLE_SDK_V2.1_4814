/*
 * boot.h
 *
 *  Created on: 2016Äê4ÔÂ3ÈÕ
 *      Author: Administrator
 */

#ifndef BOOT_H_
#define BOOT_H_
#include <stdint.h>
extern const uint32_t Load$$ISR_VECTOR$$Base;
#define LOAD_ISR_VECTOR_BASE (&Load$$ISR_VECTOR$$Base) 
extern const uint32_t Image$$ISR_VECTOR$$Base;
#define IMAGE_ISR_VECTOR_BASE (&Image$$ISR_VECTOR$$Base)
extern const uint32_t Load$$ISR_VECTOR$$Length;
#define LOAD_ISR_VECTOR_LENGTH (&Load$$ISR_VECTOR$$Length)



extern const uint32_t Image$$BOOT_PARAMS$$Base;
#define IMAGE_BOOT_PARAMS_BASE (&Image$$BOOT_PARAMS$$Base)


extern const uint32_t Image$$RAM_UNLOADED$$Base;
#define RAM_UNLOADED_BASE (&Image$$RAM_UNLOADED$$Base)
extern uint32_t __initial_sp;
#define STACK_TOP __initial_sp
#endif /* BOOT_H_ */

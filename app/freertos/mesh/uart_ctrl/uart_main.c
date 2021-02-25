/**
 ****************************************************************************************
 *
 * @file   uart_main.c
 *
 * @brief  .
 *
 * @author  Hui Chen
 * @date    2018-10-23 15:47
 * @version <0.0.0.1>
 *
 * @license 
 *              Copyright (C) Apollo 2018
 *                         ALL Right Reserved.
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "osapp_config.h"

#include "uart_main.h"
#include "mesh_uart_config.h"
#ifdef  MESH_TEST_UART_CTRL

#ifdef OSAPP_UART_LOG_TEST
#include "uart_log_test.h"
#ifdef MESH_TEST_UART_PROVISION
#include "mesh_uart_ctrl.h"
#endif/* MESH_TEST_UART_PROVISION */
#endif/* OSAPP_UART_LOG_TEST */
/*
 * MACROS
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

/*
 * ENUMERATIONS
 ****************************************************************************************
 */


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */


/*
 * LOCAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */
static const task_table_t uart_task_table[]=
{
#ifdef OSAPP_UART_LOG_TEST
    {true,"UARTLOG_TASK",OS_PRIORITY_UART_LOG_TASK,UART_LOG_TASK_STACK_SIZE,NULL,uart_log_task,&handler_uart_log_task},
#ifdef MESH_TEST_UART_PROVISION
    {true,"UARTPROV_TASK",OS_PRIORITY_UART_PROV_TASK,UART_PROV_TASK_STACK_SIZE,NULL,uart_prov_task,&handler_uart_prov_task},
#endif/* MESH_TEST_UART_PROVISION */
#endif/* OSAPP_UART_LOG_TEST */
};

static const task_table_info_t uart_task_table_info = ARRAY_INFO(uart_task_table);
/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief   Func miaoshu
 *
 * @param[in] xxx1     Id of the message received.
 * @param[in] xxx2     Pointer to the parameters of the message.
 *
 * @return If the message was consumed or not.
 ****************************************************************************************
 */
static void rtos_tasks_create(const task_table_info_t * task_table_info)
{
    uint8_t task_num = task_table_info->table_size;
    const task_table_t *task_table = task_table_info->task_table;
    uint8_t i;
    for( i = 0; i < task_num; i++ )
    {
        if( (task_table[i].is_used) && (task_table[i].entry_proc != NULL) )
        {
            xTaskCreate(task_table[i].entry_proc,task_table[i].task_name,task_table[i].stack_size,task_table[i].param,task_table[i].priority,task_table[i].task_handle);
            BX_ASSERT(task_table[i].task_handle != NULL);
        }
    }
}
void uart_main_init(void)
{
    rtos_tasks_create(&uart_task_table_info);
}
#else
void uart_main_init(void)
{

}
#endif/* MESH_TEST_UART_CTRL */

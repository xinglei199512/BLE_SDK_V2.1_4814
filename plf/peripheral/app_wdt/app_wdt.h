/** @addtogroup TIMER
 *  @ingroup PERIPHERAL
 *  @brief TIMER driver
 *  @{
 */
/*
 * app_timer.h
 *
 *  Created on: 2018Äê6ÔÂ26ÈÕ
 *      Author: CBK
 */

#ifndef PLF_PERIPHERAL_APP_WDT_APP_WDT_H_
#define PLF_PERIPHERAL_APP_WDT_APP_WDT_H_
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include "periph_common.h"
#include "wdt_integration.h"
#include "reg_wdt.h"
#include "bx_config.h"
#include "field_manipulate.h"


/*
 * DEFINES
 ****************************************************************************************
 */
#define WDT_CNT_WIDTH 16
#define RESTART_COUNT 0x76




/*
 * ENUMERATIONS
 ****************************************************************************************
 */
/// Internal system state , user don't care.
typedef enum
{
    WDT_INIT,
    WDT_UNINIT,
    WDT_START,
    WDT_STOP,
}wdt_sys_stat_config;

typedef enum
{
    wdt_Direct_Reset = 0,   //0 = Generate a system reset.
    wdt_Irq_Reset = 1       //1 = First generate an interrupt and if it is not cleared by the time a second timeout occurs then generate a system reset.
}wdt_RspMode;

typedef enum
{
    wdt_2_pclk_cycles = 0,
    wdt_4_pclk_cycles,
    wdt_8_pclk_cycles,
    wdt_16_pclk_cycles,
    wdt_32_pclk_cycles,
    wdt_64_pclk_cycles,
    wdt_128_pclk_cycles,
    wdt_256_pclk_cycles,
    wdt_Max_pclk_cycles = 0x8
}wdt_Rpl;


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/**@brief WDT parameters structure  */
typedef struct
{
    wdt_RspMode  mode;
    wdt_Rpl      plck;
    uint8_t      timeout_period;
    void        (*callback)(void*);             /**< wdt interrupt callback function */
    void        *callback_param;                /**< wdt interrupt callback parameter */
}app_wdt_comm_params_t;

/**@brief WDT instance structure */
typedef struct
{
    //public:
    periph_inst_t inst;                 /**< Peripheral common instance for TIMER.  */
    app_wdt_comm_params_t param;        /**< WDT initialization parameters.  */
    //private:
    reg_wdt_t *reg;                     /**< WDT register base address.  */
    uint8_t idx;                        /**< WDT instance index.  */
}app_wdt_inst_t;



/*
 * MACROS
 ****************************************************************************************
 */
/**@brief macro for WDT instantiation. */
#define WDT_INSTANCE(id)  \
        {\
            .inst = {.init_func = app_wdt_init,},\
            .idx = (id),\
            .reg = (reg_wdt_t *)REG_WDT_BASE,\
        }

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */


void app_wdt_isr(app_wdt_inst_t *inst);


/**
 * @brief Initialize APP_WDT module
 * @param[in] hdl the peripheral common instance handler(pointer) for the WDT instance
 */
void app_wdt_init(periph_inst_handle_t hdl);


/**
 * @brief De initialize APP_WDT module
 * @param[in] hdl the peripheral common instance handler(pointer) for the WDT instance
 */
void app_wdt_uninit(periph_inst_handle_t hdl);


/**
 * @brief Start APP_WDT module
 * @param[in] hdl the peripheral common instance handler(pointer) for the WDT instance
 */
void app_wdt_start(periph_inst_handle_t hdl);


/**
 * @brief Stop APP_WDT module
 * @param[in] hdl the peripheral common instance handler(pointer) for the WDT instance
 */
void app_wdt_stop(periph_inst_handle_t hdl);


void app_wdt_feed_dog(periph_inst_handle_t hdl);


/** @}*/
#endif /* PLF_PERIPHERAL_APP_WDT_APP_WDT_H_ */

/** @addtogroup PWM
 *  @ingroup PERIPHERAL
 *  @brief PWM driver
 *  @{
 */
/*
 * app_pwm.h
 *
 *  Created on: 2018-6-26
 *      Author: jiachuang
 */

#ifndef PLF_PERIPHERAL_APP_PWM_APP_PWM_H_
#define PLF_PERIPHERAL_APP_PWM_APP_PWM_H_
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdint.h>
#include <stdbool.h>
#include "periph_common.h"
#include "pwm_integration.h"
#include "_reg_base_addr.h"
#include "reg_pwm.h"
#include "bx_config.h"
#include "field_manipulate.h"
#include "periph_lock.h"

/*
 * DEFINES
 ****************************************************************************************
 */
/// This is all channel pwm tick clock dividor              <p>
/// PWM tick clock = 32MHz/(ALL_CHANNEL_PWM_CLK_DIV+1)      <p>
/// This value range from 0 to 0xFF
#define ALL_CHANNEL_PWM_CLK_DIV     1
#define MAX_PWM_CLK_FREQUENCY       ((32000000UL)/(ALL_CHANNEL_PWM_CLK_DIV+1)) //frequency

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/// Internal system state , user don't care.
typedef enum pwm_inter_stat
{
    PWM_INTER_INVALID,
    PWM_INTER_NORMAL_PWM,
    PWM_INTER_STILL_LOW,
    PWM_INTER_STILL_HIGH,
} pwm_inter_stat_t;

typedef enum pwm_sys_stat_config
{
    PWM_INIT,
    PWM_UNINIT,
    PWM_OUTPUT_START,
    PWM_OUTPUT_STOP,
}pwm_sys_stat_config_t;

typedef enum
{
    PWM_CHANNEL_0,
    PWM_CHANNEL_1,
    PWM_CHANNEL_2,
    PWM_CHANNEL_3,
    PWM_CHANNEL_4,
    PWM_CHANNEL_SUM,
}pwm_channel_t;


/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
/**@brief PWM parameters structure  */
typedef struct
{
    uint8_t         pin_num;            /**< select which pin output the pwm. */
}app_pwm_channel_t;


/**@brief PWM instance structure */
typedef struct
{
    //public:
    periph_inst_t     inst;                         /**< Peripheral common instance for PWM.  */
    app_pwm_channel_t channel[PWM_CHANNEL_SUM];     /**< PWM initialization parameters.  */
    //private:
    reg_pwm_t         *reg;                         /**< PWM register base address.  */
    pwm_inter_stat_t  inter_stat[PWM_CHANNEL_SUM];  /**< PWM internal status.  */
    periph_lock_t     init_lock;                    /**< lock for init/uninit operation  */
}app_pwm_inst_t;



/*
 * MACROS
 ****************************************************************************************
 */
/**@brief macro for PWM instantiation. */
#define PWM_INSTANCE()  \
        {\
            .inst = {.init_func = app_pwm_reinit,},\
            .reg = (reg_pwm_t *)REG_PWM_BASE,\
        }


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

periph_err_t app_pwm_reinit(periph_inst_handle_t hdl);

/**
 * @brief Initialize APP_PWM module
 * @param[in] hdl the peripheral common instance handler(pointer) for the PWM instance
 */
periph_err_t app_pwm_init(periph_inst_handle_t hdl);


/**
 * @brief De initialize APP_PWM module
 * @param[in] hdl the peripheral common instance handler(pointer) for the PWM instance
 */
periph_err_t app_pwm_uninit(periph_inst_handle_t hdl);


/**
 * @brief app_pwm_set_time APP_PWM module
 * @param[in] 		hdl 		the peripheral common instance handler(pointer) for the PWM instance
 * @param[in]       channel     the channel switched.
 * @param[in] 		high_time 	set pwm high time , uint:ticks
 * @param[in]       low_time    set pwm low time  , uint:ticks
 */
void app_pwm_set_time(periph_inst_handle_t hdl , pwm_channel_t channel , uint16_t high_time, uint16_t low_time);



/**
 * @brief app_pwm_set_duty APP_PWM module
 * @param[in] 		hdl 		the peripheral common instance handler(pointer) for the PWM instance
 * @param[in]       channel     the channel switched.
 * @param[in] 		frequency 	set pwm frequency unit  hz   (max 160k hz)
 * @param[in]       percent     set pwm percent (0~100)
 */
void app_pwm_set_duty(periph_inst_handle_t hdl , pwm_channel_t channel,uint32_t frequency,uint8_t percent);


/** @}*/
#endif /* PLF_PERIPHERAL_APP_PWM_APP_PWM_H_ */

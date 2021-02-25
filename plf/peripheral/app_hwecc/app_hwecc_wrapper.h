/** @addtogroup HWECC_WRAPPER
 *    @ingroup HWECC
 *  @{
 */

#ifndef APP_HWECC_WRAPPER_H_
#define APP_HWECC_WRAPPER_H_
#include "app_hwecc.h"

typedef void (*ecc_cb_t)(void *);

/**@brief ecc calculation parameters */
typedef struct{
    ecc_in_t in;    /**< input data for ecc.  */
    ecc_out_t out;  /**< ecc output buffer.  */
    ecc_cb_t cb;    /**< callback function pointer.  */
    void *dummy; /**< callback parameter.  */
}ecc_queue_t;

/**
 * @brief Initialize HWECC
 */
void app_hwecc_init_wrapper(void);

/**
 * @brief De initialize HWECC
 */
void app_hwecc_uninit_wrapper(void);

/**
 * @brief Start ECC Calculation Using HWECC
 * @param[in] param parameters for ecc calculation
 * @return error code
 */
periph_err_t app_hwecc_calculate_wrapper(ecc_queue_t *param);

#endif
/** @}*/


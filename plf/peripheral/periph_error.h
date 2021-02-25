/** @addtogroup PERIPHERAL
 *  @{
 */
#ifndef PERIPH_ERROR_H_
#define PERIPH_ERROR_H_

/**@brief peripheral error code */
typedef enum 
{
    PERIPH_NO_ERROR,
    PERIPH_INVALID_PARAM,
    PERIPH_BUSY,
    PERIPH_STATE_ERROR,
    PERIPH_INVALID_OPERATION,
    PERIPH_DMAC_NO_AVAILABLE_CHANNEL,
}periph_err_t;



#endif
/** @}*/
